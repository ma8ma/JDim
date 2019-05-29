// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_CHUNKED
//#define _DEBUG_TIME
#include "jddebug.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "loader.h"
#include "miscmsg.h"
#include "miscutil.h"
#include "jdsocket.h"

#ifdef _DEBUG_TIME
#include "misctime.h"
#include <iostream>
#endif

#include "config/globalconf.h"

#include "skeleton/loadable.h"

#include "httpcode.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <mutex>
#include <sstream>

#include <glibmm.h>


constexpr int MAX_LOADER = 10; // 最大スレッド数
constexpr int MAX_LOADER_SAMEHOST = 2; // 同一ホストに対して実行できる最大スレッド数
constexpr size_t LNG_BUF_MIN = 1 * 1024; // 読み込みバッファの最小値 (byte)
constexpr long TIMEOUT_MIN = 1; // タイムアウトの最小値 (秒)


namespace {

std::string get_error_message( int err_code )
{
    std::string result = " [Errno " + std::to_string( err_code ) + "] ";
    result.append( std::strerror( err_code ) );
    return result;
}

} // namespace


//
// トークンとスレッド起動待ちキュー
//
// 起動しているローダが MAX_LOADER 個を越えたらローダをスレッド待ちキューに入れる
//

namespace JDLIB
{
    bool get_token( JDLIB::Loader* loader );
    void return_token( JDLIB::Loader* loader );

    void push_loader_queue( JDLIB::Loader* loader );
    bool remove_loader_queue( JDLIB::Loader* loader );
    void pop_loader_queue();
}


static std::mutex mutex_token;
static std::mutex mutex_queue;
std::list< JDLIB::Loader* > queue_loader; // スレッド起動待ちの Loader のキュー
int token_loader = 0;
std::vector< JDLIB::Loader* > vec_loader( MAX_LOADER );
bool disable_pop = false;


// トークン取得
bool JDLIB::get_token( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_token );

#ifdef _DEBUG
    std::cout << "JDLIB::get_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif

    if( token_loader >= MAX_LOADER ) return false;

    const std::string& host = loader->data().host;
    const int count = std::count_if( vec_loader.cbegin(), vec_loader.cend(),
                                     [&host]( const JDLIB::Loader* p ) { return p && p->data().host == host; } );
#ifdef _DEBUG
    std::cout << "count = " << count << std::endl;
#endif

    const int max_loader = MIN( MAX_LOADER_SAMEHOST, MAX( 1, CONFIG::get_connection_num() ) );
    if( count >= max_loader ) return false;

#ifdef _DEBUG
    std::cout << "got token\n";
#endif

    ++token_loader;

    auto it = std::find( vec_loader.begin(), vec_loader.end(), nullptr );
    if( it != vec_loader.end() ) *it = loader;

    return true;
}


//　トークン返す
void JDLIB::return_token( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_token );

    --token_loader;
    assert( token_loader >= 0 );

    std::replace( vec_loader.begin(), vec_loader.end(), loader, static_cast< JDLIB::Loader* >( nullptr ) );

#ifdef _DEBUG
    std::cout << "JDLIB::return_token : url = " << loader->data().url << " token = " << token_loader << std::endl;
#endif
}


// スレッド起動待ちキューに Loader を登録
void JDLIB::push_loader_queue( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_queue );

    if( ! loader ) return;

    if( loader->get_low_priority() ) queue_loader.push_back( loader );
    else{

        const auto pos = std::find_if( queue_loader.cbegin(), queue_loader.cend(),
                                       []( const JDLIB::Loader* p ) { return p && p->get_low_priority(); } );
        queue_loader.insert( pos, loader );
    }

#ifdef _DEBUG
    std::cout << "JDLIB::push_loader_queue url = " << loader->data().url << " size = " << queue_loader.size() << std::endl;
#endif    
}


// キューから Loader を取り除いたらtrueを返す
bool JDLIB::remove_loader_queue( JDLIB::Loader* loader )
{
    std::lock_guard< std::mutex > lock( mutex_queue );

    if( ! queue_loader.size() ) return false;
    if( std::find( queue_loader.begin(), queue_loader.end(), loader ) == queue_loader.end() ) return false;

    queue_loader.remove( loader );

#ifdef _DEBUG
    std::cout << "JDLIB::remove_loader_queue url = " << loader->data().url << " size = " << queue_loader.size() << std::endl;
#endif    

    return true;
}


// キューに登録されたスレッドを起動する
void JDLIB::pop_loader_queue()
{
    std::lock_guard< std::mutex > lock( mutex_queue );

    if( disable_pop ) return;
    if( ! queue_loader.size() ) return;

#ifdef _DEBUG
    std::cout << "JDLIB::pop_loader_queue size = " << queue_loader.size() << std::endl;
#endif    

    const auto it = std::find_if( queue_loader.begin(), queue_loader.end(),
                                  []( JDLIB::Loader* p ) { return JDLIB::get_token( p ); } );
    if( it == queue_loader.end() ) return;

    JDLIB::Loader* loader = *it;
    queue_loader.remove( loader );

#ifdef _DEBUG
    std::cout << "pop " << loader->data().url << std::endl;
#endif

    loader->create_thread();
}


//
// ローダの起動待ちキューにあるスレッドを実行しない
// 
// アプリ終了時にこの関数を呼び出さないとキューに登録されたスレッドが起動してしまうので注意
//
void JDLIB::disable_pop_loader_queue()
{
    std::lock_guard< std::mutex > lock( mutex_queue );

#ifdef _DEBUG
    std::cout << "JDLIB::disable_pop_loader_queue\n";
#endif    

    disable_pop = true;
}


//
// mainの最後でローダが動いていないかチェックする関数
//
void JDLIB::check_loader_alive()
{
#ifdef _DEBUG
    std::cout << "JDLIB::check_loader_alive loader = " << token_loader
              << " queue = " << queue_loader.size() << std::endl;
#endif    

    if( token_loader ){
        MISC::ERRMSG( "loaders are still moving." );
        assert( false );
    }

    if( queue_loader.size() ){
        MISC::ERRMSG( "queue of loaders are not empty." );
        assert( false );
    }
}



///////////////////////////////////////////////////

using namespace JDLIB;


//
// low_priority = true の時はスレッド起動待ち状態になった時に、起動順のプライオリティを下げる
//
Loader::Loader( const bool low_priority )
    : m_stop( false ),
      m_loading( false ),
      m_low_priority( low_priority ),
      m_use_zlib ( 0 )
{
#ifdef _DEBUG
    std::cout << "Loader::Loader : loader was created.\n";
#endif

    clear();
}


Loader::~Loader()
{
#ifdef _DEBUG
    std::cout << "Loader::~Loader : url = " << m_data.url << std::endl;
#endif

    clear();

//    assert( ! m_loading );
}


void Loader::clear()
{
#ifdef _DEBUG
    std::cout << "Loader::clear\n";
#endif

    stop();
    wait();

    m_loadable = nullptr;
    
    m_use_chunk = false;

    m_buf.clear();
    m_buf.shrink_to_fit();

    m_buf_zlib_in.clear();
    m_buf_zlib_in.shrink_to_fit();

    m_buf_zlib_out.clear();
    m_buf_zlib_out.shrink_to_fit();

    if( m_use_zlib ) inflateEnd( &m_zstream );
    m_use_zlib = false;
}


void Loader::wait()
{
    m_thread.join();
}


void Loader::stop()
{
    if( ! m_loading ) return;

#ifdef _DEBUG
    std::cout << "Loader::stop : url = " << m_data.url << std::endl;
#endif

    m_stop = true;

    // スレッド起動待ち状態の時は SKELETON::Loadable にメッセージを送る
    if( JDLIB::remove_loader_queue( this ) ){

        m_data.code = HTTP_TIMEOUT;
        m_data.modified = std::string();
        m_data.str_code = "stop loading";
        finish_loading();
    }
}


//
// ダウンロード開始
//
// data_in でセットする必要がある項目 ( url は必須 )
//
// url
// head ( true なら HEAD 送信 )
// port ( == 0 ならプロトコルを見て自動認識 )
// str_post( != empty なら POST する。UTF-8であること )
// modified
// byte_readfrom ( != 0 ならその位置からレジューム)
// agent
// referer
// cookie_for_request
// host_proxy ( != empty ならproxy使用 )
// port_proxy ( == 0 なら 8080 )
// basicauth_proxy
// size_buf ( バッファサイズ, k 単位で指定。 == 0 ならデフォルトサイズ(LNG_BUF_MIN)使用)
// timeout ( タイムアウト秒。==0 ならデフォルト( TIMEOUT )使用  )
// basicauth
//
bool Loader::run( SKELETON::Loadable* cb, const LOADERDATA& data_in )
{
    assert( ! data_in.url.empty() );

#ifdef _DEBUG
    std::cout << "Loader::run : url = " << data_in.url << std::endl;
#endif

    if( m_loading ){
        MISC::ERRMSG( "now loading : " + data_in.url );
        return false;
    }

    clear();
    m_loadable = cb;
    m_data.init();
    m_stop = false;
    
    // バッファサイズ設定
    m_data.size_buf = data_in.size_buf;
    m_lng_buf = MAX( LNG_BUF_MIN, m_data.size_buf * 1024 );
    m_lng_buf_zlib_in = m_lng_buf * 2;
    m_lng_buf_zlib_out = m_lng_buf * 10; // 小さいとパフォーマンスが落ちるので少し多めに10倍位
    
    // protocol と host と path 取得
    m_data.url = data_in.url;
    size_t i = m_data.url.find( "://", 0 );  // "http://" とつけるのは呼び出し側の責任で
    if( i == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could not get protocol : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    i += 3;
    m_data.protocol = data_in.url.substr( 0, i );

    const size_t i2 = m_data.url.find( '/', i );
    if( i2 == std::string::npos ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "could not get hostname and path : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }
    
    m_data.host = m_data.url.substr( i, i2 - i );
    m_data.path = m_data.url.substr( i2 );

    // ポートセット
    // プロトコルを見て自動決定

    // ssl使用指定
    // HACK: httpsから始まるURLで プロキシを使わない or 2ch系サイトでない 場合はhttpsで送信する
    constexpr std::array<const char*, 3> domains{ ".5ch.net", ".2ch.net", ".bbspink.com" };
    const std::string& hostname = m_data.host;
    const auto has_domain = [&hostname]( const char* d ) { return hostname.find( d ) != std::string::npos; };

    // https
    if( m_data.protocol == "https://"
        && ( data_in.host_proxy.empty() || std::none_of( domains.cbegin(), domains.cend(), has_domain ) )
    ) {
        m_data.use_ssl = true;
        m_data.port = 443;
    }

    // http or using proxy
    else if( m_data.protocol == "http://" || m_data.protocol == "https://" ) m_data.port = 80;

    // その他
    else{
        m_data.code = HTTP_ERR;
        m_data.str_code = "unknown protocol : " + m_data.url;
        MISC::ERRMSG( m_data.str_code );
        return false;
    }

    // ポート番号
    if( data_in.port != 0 ) m_data.port = data_in.port;

    // ホスト名の後に指定されている
    if( ( i = m_data.host.find( ':' ) ) != std::string::npos ){
        m_data.port = atoi( m_data.host.substr( i+1 ).c_str() );
        m_data.host = m_data.host.substr( 0, i );
    }

    // 明示的にssl使用指定
    if( data_in.use_ssl ) m_data.use_ssl = true;

    // プロキシ
    m_data.host_proxy = data_in.host_proxy;
    if( ( i = m_data.host_proxy.find( "://" ) ) != std::string::npos ){
        const std::string proto = data_in.host_proxy.substr( 0, i );
        if( proto == "http" ) m_data.protocol_proxy = PROXY_HTTP;
        else if( proto == "socks4" ) m_data.protocol_proxy = PROXY_SOCKS4;
        else if( proto == "socks4a" ) m_data.protocol_proxy = PROXY_SOCKS4A;
        else{
            m_data.code = HTTP_ERR;
            m_data.str_code = "unknown proxy protocol : " + proto;
            MISC::ERRMSG( m_data.str_code );
            return false;
        }
        m_data.host_proxy = m_data.host_proxy.substr( i + 3 );
    }
    else m_data.protocol_proxy = PROXY_HTTP;

    // プロキシのポート番号
    if( data_in.port_proxy != 0 ) m_data.port_proxy = data_in.port_proxy;

    // ホスト名の後に指定されている
    if( ( i = m_data.host_proxy.rfind( ":" ) ) != std::string::npos ){
        m_data.port_proxy = atoi( m_data.host_proxy.substr( i + 1 ).c_str() );
        m_data.host_proxy = m_data.host_proxy.substr( 0, i );
    }

    if( ! m_data.host_proxy.empty() ){
        m_data.basicauth_proxy = data_in.basicauth_proxy;
    }

    // その他
    m_data.head = data_in.head;
    m_data.str_post = data_in.str_post;
    m_data.modified = data_in.modified;
    m_data.byte_readfrom = data_in.byte_readfrom;    
    m_data.contenttype = data_in.contenttype;
    m_data.agent = data_in.agent;
    m_data.origin = data_in.origin;
    m_data.referer = data_in.referer;
    m_data.accept = data_in.accept;
    m_data.cookie_for_request = data_in.cookie_for_request;
    m_data.timeout = MAX( TIMEOUT_MIN, data_in.timeout );
    m_data.ex_field = data_in.ex_field;
    m_data.basicauth = data_in.basicauth;

#ifdef _DEBUG    
    std::cout << "host: " << m_data.host << std::endl;
    std::cout << "protocol: " << m_data.protocol << std::endl;
    std::cout << "path: " << m_data.path << std::endl;
    std::cout << "port: " << m_data.port << std::endl;
    std::cout << "modified: " << m_data.modified << std::endl;
    std::cout << "byte_readfrom: " << m_data.byte_readfrom << std::endl;
    std::cout << "contenttype: " << m_data.contenttype << std::endl;
    std::cout << "agent: " << m_data.agent << std::endl;
    std::cout << "referer: " << m_data.referer << std::endl;
    std::cout << "cookie: " << m_data.cookie_for_request << std::endl;
    std::cout << "protocol of proxy: " << m_data.protocol_proxy << std::endl;
    std::cout << "proxy: " << m_data.host_proxy << std::endl;
    std::cout << "port of proxy: " << m_data.port_proxy << std::endl;
    std::cout << "proxy basicauth : " << m_data.basicauth_proxy << std::endl;
    std::cout << "buffer size: " << m_lng_buf / 1024 << " Kb" << std::endl;
    std::cout << "timeout : " << m_data.timeout << " sec" << std::endl;
    std::cout << "ex_field : " << m_data.ex_field << std::endl;
    std::cout << "basicauth : " << m_data.basicauth << std::endl;
    std::cout << "\n";
#endif

    m_loading = true;

    // トークンを取得出来なかったら、他のスレッドが終了した時に
    // 改めて create_thread() を呼び出す
    if( get_token( this ) ) create_thread();
    else JDLIB::push_loader_queue( this );

    return true;
}


//
// スレッド起動
//
void Loader::create_thread()
{
#ifdef _DEBUG
    std::cout << "Loader::create_thread :  url = " << m_data.url << std::endl;
#endif

    if( ! m_thread.create( ( STARTFUNC ) launcher, ( void * ) this, JDLIB::NODETACH ) ){

        m_data.code = HTTP_ERR;
        m_data.str_code = "Loader::run : could not start thread";
        MISC::ERRMSG( m_data.str_code );
        finish_loading();
        return;
    }
}


//
// スレッドのランチャ (static)
//
void* Loader::launcher( void* dat )
{
    Loader* tt = reinterpret_cast< Loader * >( dat );
    tt->run_main();
    return nullptr;
}


//
// 実際の処理部
//
void Loader::run_main()
{
    // エラーメッセージ
    std::string errmsg;

    const bool use_proxy = ( ! m_data.host_proxy.empty() );

    // 送信メッセージ作成
    const std::string msg_send = create_msg_send();
    
#ifdef _DEBUG    
    std::cout << "Loader::run_main : start loading thread : " << m_data.url << std::endl;;
    if( use_proxy ) std::cout << "use_proxy : " << m_data.host_proxy << std::endl;
    std::cout <<"send :----------\n" <<  msg_send << "\n---------------\n";
#endif

    const bool async = ! m_data.use_ssl || CONFIG::get_tls_nonblocking();
    JDLIB::Socket soc( &m_stop, async );
    soc.set_timeout( m_data.timeout );

    // socket接続
    const bool use_ipv6 = CONFIG::get_use_ipv6();
    if( m_data.host_proxy.empty() ){
        if( ! soc.connect( m_data.host, std::to_string( m_data.port ), use_ipv6 ) ){
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }
    else{
        if( ! soc.connect( m_data.host_proxy, std::to_string( m_data.port_proxy ), use_ipv6 ) ){
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // 受信用バッファの割り当て
    assert( m_buf.empty() );
    m_buf.resize( m_lng_buf );
    
    // Socksのハンドシェーク
    if( use_proxy && m_data.protocol_proxy != PROXY_HTTP ){

        if( ! soc.socks_handshake( m_data.host, std::to_string( m_data.port ), m_data.protocol_proxy ) ){
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // HTTP tunneling
    else if( use_proxy && m_data.protocol_proxy == PROXY_HTTP && m_data.use_ssl ){

        // CONNECT
        std::string msg = "CONNECT ";
        msg += m_data.host + ":" + std::to_string( m_data.port ) + " HTTP/1.1\r\n";
        msg += "Host: " + m_data.host + "\r\n";
        msg += "Proxy-Connection: keep-alive\r\n";
        if( ! m_data.agent.empty() ) msg += "User-Agent: " + m_data.agent + "\r\n";
        msg += "\r\n";

        if( soc.write( msg.c_str(), msg.length() ) < 0 ){

            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }

        // 読み込み
        size_t read_size = 0;
        while( read_size < m_lng_buf && !m_stop ){

            const ssize_t tmpsize = soc.read( m_buf.data() + read_size, m_lng_buf - read_size );
            if( tmpsize < 0 ){
                m_data.code = HTTP_ERR;
                errmsg = soc.get_errmsg();
                goto EXIT_LOADING;
            }

            else if( tmpsize == 0 ) goto EXIT_LOADING;
            else read_size += tmpsize;

            const int ret = receive_header( m_buf.data(), read_size );
            if( ret == HTTP_ERR ){
                m_data.code = HTTP_ERR;
                errmsg = "invalid proxy header : " + m_data.url;
                errmsg.append( get_error_message( errno ) );
                goto EXIT_LOADING;
            }
            else if( ret == HTTP_OK ){
                if( m_data.code < 200 || m_data.code >= 300 ){
                    goto EXIT_LOADING;
                }
                break;
            }
        }
    }

    // TLSのハンドシェーク
    if( m_data.use_ssl ){

        if( ! soc.tls_handshake( m_data.host.c_str(), CONFIG::get_verify_cert() ) ){
            m_data.code = HTTP_ERR;
            errmsg = soc.get_errmsg();
            goto EXIT_LOADING;
        }
    }

    // SEND 又は POST
    if( soc.write( msg_send.c_str(), msg_send.length() ) < 0 ){

        m_data.code = HTTP_ERR;
        errmsg = soc.get_errmsg();
        goto EXIT_LOADING;
    }

#ifdef _DEBUG_TIME
    MISC::start_measurement( 1 );
#endif

    // 受信開始
    bool receiving_header;
    receiving_header = true;
    m_data.length_current = 0;
    m_data.size_data = 0;    
    do{
        // 読み込み
        size_t read_size = 0;
        while( read_size < m_lng_buf && !m_stop ){

            const ssize_t tmpsize = soc.read( m_buf.data() + read_size, m_lng_buf - read_size );
            if( tmpsize < 0 ){
                m_data.code = HTTP_ERR;
                errmsg = soc.get_errmsg();
                goto EXIT_LOADING;
            }

            if( tmpsize == 0 ) break;
            if( tmpsize > 0 ){

                read_size += tmpsize;

                // ヘッダ取得
                if( receiving_header ){

                    const int http_code = receive_header( m_buf.data(), read_size );
                    if( http_code == HTTP_ERR ){

                        m_data.code = HTTP_ERR;
                        errmsg = "invalid header : " + m_data.url;
                        goto EXIT_LOADING;
                    }
                    else if( http_code == HTTP_OK ) receiving_header = false;
                }

                if( m_data.length && m_data.length <= m_data.length_current + read_size ) break;
            }

        }

        // 停止指定
        if( m_stop ) break;

        // サーバ側がcloseした
        if( read_size == 0 ){

            // ヘッダを取得する前にcloseしたらエラー
            if( receiving_header && m_data.size_data == 0 ){
                m_data.code = HTTP_ERR;         
                errmsg = "no data";
                goto EXIT_LOADING;
            }

            // コード304等の場合は終了
            break;
        }

        // ヘッダ取得失敗
        if( receiving_header ){
            m_data.code = HTTP_ERR;         
            errmsg = "no http header";
            goto EXIT_LOADING;
        }

        m_data.length_current += read_size;
        
        //  chunkedな場合
        if( m_use_chunk ){
            
            if( !skip_chunk( m_buf.data(), read_size ) ){

                m_data.code = HTTP_ERR;
                errmsg = "skip_chunk() failed";
                goto EXIT_LOADING;
            }
            if( ! read_size ) break;
        }

        // 圧縮されていない時はそのままコールバック呼び出し
        if( !m_use_zlib ) {

            m_data.size_data += read_size;

            // コールバック呼び出し
            if( m_loadable ) m_loadable->receive( m_buf.data(), read_size );
        }
        
        // 圧縮されているときは unzip してからコールバック呼び出し
        else if( !unzip( m_buf.data(), read_size ) ){
            
            m_data.code = HTTP_ERR;
            errmsg = "unzip() failed";
            goto EXIT_LOADING;
        }

        if( m_data.length && m_data.length <= m_data.length_current ) break;
        
    } while( !m_stop );

#ifdef _DEBUG_TIME
    std::cout << "Loader::run_main loading time(ns) = " << MISC::measurement( 1 ) << std::endl;
#endif

    // 終了処理
EXIT_LOADING:

    // 強制停止した場合
    if( m_stop ){
#ifdef _DEBUG
        std::cout << "Loader::run_main : stop loading\n";
#endif
        m_data.code = HTTP_TIMEOUT;
        m_data.modified = std::string();
        m_data.str_code = "stop loading";
    }
    // エラーあり
    else if( ! errmsg.empty() ){
        m_data.modified = std::string();
        MISC::ERRMSG( errmsg );
        m_data.str_code = errmsg;
    }

    // ソケットクローズ
    soc.close();

    // トークン返す
    return_token( this );

    finish_loading();

#ifdef _DEBUG
    std::cout << "Loader::run_main : finish loading : " << m_data.url << std::endl;;
    std::cout << "read size : " << m_data.length_current << " / " << m_data.length << std::endl;;    
    std::cout << "data size : " << m_data.size_data << std::endl;;
    std::cout << "code : " << m_data.code << std::endl << std::endl;
#endif    
}



//
// 送信メッセージ作成
//
std::string Loader::create_msg_send() const
{
    const bool post_msg = ( !m_data.str_post.empty() && !m_data.head );
    const bool use_proxy = ! m_data.host_proxy.empty() && ! m_data.use_ssl;

    std::ostringstream msg;
    msg.clear();

    if( m_data.head ) msg << "HEAD ";
    else if( ! post_msg ) msg << "GET ";
    else msg << "POST ";
    
    if( ! use_proxy ) msg << m_data.path << " HTTP/1.1\r\n";
    else {
        msg << m_data.protocol << m_data.host << ":" << m_data.port << m_data.path << " HTTP/1.1\r\n";
    }

    msg << "Host: " << m_data.host << "\r\n";
    if( ! m_data.contenttype.empty() ) msg << "Content-Type: " << m_data.contenttype << "\r\n";
    if( ! m_data.agent.empty() ) msg << "User-Agent: " << m_data.agent << "\r\n";
    if( ! m_data.origin.empty() ) msg << "Origin: " << m_data.origin << "\r\n";
    if( ! m_data.referer.empty() ) msg << "Referer: " << m_data.referer << "\r\n";

    // basic認証
    if( ! m_data.basicauth.empty() ) msg << "Authorization: Basic " << MISC::base64( m_data.basicauth ) << "\r\n";

    // proxy basic認証
    if( use_proxy && ! m_data.basicauth_proxy.empty() ) msg << "Proxy-Authorization: Basic " << MISC::base64( m_data.basicauth_proxy ) << "\r\n";

    if( ! m_data.cookie_for_request.empty() ) msg << "Cookie: " << m_data.cookie_for_request << "\r\n";

    if( ! m_data.modified.empty() ) msg << "If-Modified-Since: " << m_data.modified << "\r\n";

    if( ! m_data.accept.empty() ) msg << "Accept: " << m_data.accept << "\r\n";
    else msg << "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n";
    msg << "Accept-Language: ja,en-US;q=0.7,en;q=0.3\r\n";

    // レジュームするときは gzip は受け取らない
    if( m_data.byte_readfrom ){

        msg << "Range: bytes=" << m_data.byte_readfrom << "-\r\n";

        // プロキシ使う場合は no-cache 指定 
        if( use_proxy ) msg << "Cache-Control: no-cache\r\n";
    }
    else msg << "Accept-Encoding: gzip\r\n"; // レジュームしないなら gzip 受け取り可能

    // その他のフィールド
    if( ! m_data.ex_field.empty() ) msg << m_data.ex_field;

    msg << "Connection: close\r\n";

    // POST する文字列
    if( post_msg ){
        
        msg << "Content-Length: " << m_data.str_post.length() << "\r\n";
        msg << "\r\n";
        msg << m_data.str_post;
        msg << "\r\n";
    }
    msg << "Upgrade-Insecure-Requests: 1\r\n";
    msg << "\r\n";
    
    return msg.str();
}


//
// サーバから送られてきた生データからヘッダ取得
//
// 戻り値 : 成功 HTTP_OK、失敗 HTTP_ERR、未処理 HTTP_INIT
//
// 入力
// buf : 生データ
// readsize: 生データサイズ
//
// 出力
// buf : ヘッダが取り除かれたデータ
// readsize: 出力データサイズ
//
int Loader::receive_header( char* buf, size_t& read_size )
{
#ifdef _DEBUG
    std::cout << "Loader::receive_header : read_size = " << read_size << std::endl;
#endif

    m_data.str_header.assign( buf, read_size );
    size_t lng_header = m_data.str_header.find( "\r\n\r\n" );
    if( lng_header != std::string::npos ) lng_header += 4;
    else{

        lng_header = m_data.str_header.find( "\n\n" );
        if( lng_header != std::string::npos ) lng_header +=2;
        else return HTTP_INIT;
    }
        
    m_data.str_header.resize( lng_header ); 

#ifdef _DEBUG    
    std::cout << "header : size = " << lng_header << " byte\n";
    std::cout << m_data.str_header << std::endl;
#endif

    if( ! analyze_header() ) return HTTP_ERR;
                
    // 残りのデータを前に移動
    read_size -= lng_header;
    if( read_size ) memmove( buf, buf + lng_header, read_size );

    return HTTP_OK;
}


//
// ヘッダ解析
//
bool Loader::analyze_header()
{
    // コード
    std::string str_tmp = analyze_header_option( "HTTP/1.1 " );
    if( ! str_tmp.empty() ){
        m_data.str_code = "HTTP/1.1 "  + str_tmp;
    }
    else{
        str_tmp = analyze_header_option( "HTTP/1.0 " );
        if( ! str_tmp.empty() ) m_data.str_code = "HTTP/1.0 "  + str_tmp;
    }

    if( str_tmp.empty() ){
        MISC::ERRMSG( "could not find HTTP/1.1" );
        return false;
    }

    const size_t i = str_tmp.find( ' ' );
    if( i == std::string::npos ) m_data.code = atoi( str_tmp.c_str() );
    else m_data.code = atoi( str_tmp.substr( 0, i ).c_str() );

    if( m_data.code == 0 ){
        MISC::ERRMSG( "could not get http status code" );
        return false;
    }

    // サイズ
    m_data.length = 0;
    str_tmp = analyze_header_option( "Content-Length: " );
    if( ! str_tmp.empty() ) m_data.length = atoi( str_tmp.c_str() );
    
    // date
    m_data.date = analyze_header_option( "Date: " );

    // modified
    m_data.modified = analyze_header_option( "Last-Modified: " );
    
    // cookie
    m_data.list_cookies = analyze_header_option_list( "Set-Cookie: " );

    // Location
    if( m_data.code == HTTP_REDIRECT
        || m_data.code == HTTP_MOVED_PERM ) m_data.location = analyze_header_option( "Location: " );
    else m_data.location = std::string();

    // Content-Type
    m_data.contenttype = analyze_header_option( "Content-Type: " );
    
    // ERROR
    m_data.error = analyze_header_option( "ERROR: " );

    // Thread-Status
    m_data.threadstatus = 0;
    str_tmp = analyze_header_option( "Thread-Status: " );
    if( ! str_tmp.empty() ) m_data.threadstatus = atoi( str_tmp.c_str() );

    // chunked か
    m_use_chunk = false;
    str_tmp = analyze_header_option( "Transfer-Encoding: " );
    if( str_tmp.find( "chunked" ) != std::string::npos ){
        
        m_use_chunk = true;
        m_status_chunk = 0;
        m_pos_sizepart = m_str_sizepart;
    }

    // gzip か
    m_use_zlib = false;
    str_tmp = analyze_header_option( "Content-Encoding: " );
    if( str_tmp.find( "gzip" ) != std::string::npos ){
        if( !init_unzip() ) return false;
    }

#ifdef _DEBUG
    std::cout << "code = " << m_data.code << std::endl;
    std::cout << "length = " << m_data.length << std::endl;    
    std::cout << "date = " << m_data.date << std::endl;
    std::cout << "modified = " << m_data.modified << std::endl;

    for( const std::string& cookie : m_data.list_cookies ) {
        std::cout << "cookie = " << cookie << std::endl;
    }

    std::cout << "location = " << m_data.location << std::endl;
    std::cout << "contenttype = " << m_data.contenttype << std::endl;
    std::cout << "error = " << m_data.error << std::endl;
    std::cout << "threadstatus = " << m_data.threadstatus << std::endl;
    if( m_use_chunk ) std::cout << "m_use_chunk = true\n";
    if( m_use_zlib )  std::cout << "m_use_zlib = true\n";

    std::cout << "authenticate = " << analyze_header_option( "WWW-Authenticate: " ) << std::endl;

    std::cout << "\n";
#endif
    
    return true;
}


//
// analyze_header() から呼ばれるオプションの値を取り出す関数
//
std::string Loader::analyze_header_option( const std::string& option ) const
{
    char accept[3] = { 0, 0, 0 };
    const char ch = option[ 0 ];

    if( ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) ){
        accept[ 0 ] = ch & ~0x20;
        accept[ 1 ] = ch | 0x20;
    }
    else accept[ 0 ] = ch;

    const char *p1, *p2 = m_data.str_header.c_str();

    for( ; ( p1 = strpbrk( p2, accept ) ) != nullptr ; p2 = p1 + 1 ){
        if( strncasecmp( p1, option.c_str(), option.length() ) == 0 ) break;
    }

    if( p1 != nullptr ){
        p2 = strstr( p1, "\r\n" );
        if( p2 == nullptr ) p2 = strchr( p1, '\n' );
        if( p2 != nullptr ){
            p1 += option.length();
            return std::string( p1, p2 - p1 );
        }
    }
    return std::string();
}



//
// analyze_header() から呼ばれるオプションの値を取り出す関数(リスト版)
//
std::list< std::string > Loader::analyze_header_option_list( const std::string& option ) const
{
    std::list< std::string > str_list;

    const std::size_t option_length = option.length();

    std::size_t i2 = 0;
    for(;;){

        const std::size_t i = m_data.str_header.find( option, i2 );
        if( i == std::string::npos ) break;

        i2 = m_data.str_header.find( "\r\n", i );
        if( i2 == std::string::npos ) i2 = m_data.str_header.find( '\n', i );
        if( i2 == std::string::npos ) break;

        str_list.push_back( m_data.str_header.substr( i + option_length, i2 - ( i + option_length ) ) );
    }

    return str_list;
}




//
// chunked なデータを切りつめる関数
//
// 入力
// buf : 生データ
// readsize: 生データサイズ
//
// 出力
// buf : 切りつめられたデータ
// readsize: 出力データサイズ
//
bool Loader::skip_chunk( char* buf, size_t& read_size )
{
#ifdef _DEBUG_CHUNKED    
    std::cout << "\n[[ skip_chunk : start read_size = " << read_size << " ]]\n";
#endif    
    
    size_t pos_chunk = 0;
    size_t pos_data_chunk_start = 0;
    size_t buf_size = 0;
    
    for(;;){

        // サイズ部
        if( m_status_chunk == 0 ){  
        
            // \nが来るまで m_str_sizepart[] に文字をコピーしていく
            for( ; pos_chunk < read_size; ++pos_chunk, ++m_pos_sizepart ){

                // バッファオーバーフローのチェック
                if( ( long )( m_pos_sizepart - m_str_sizepart ) >= 64 ){
                    m_use_chunk = false;
                    MISC::ERRMSG( "chunk specified but maybe no chunk data" );
                    return true;
                }
                
                *m_pos_sizepart =  buf[ pos_chunk ];

                // \n が来たらデータ部のサイズを取得
                if( buf[ pos_chunk ] == '\n' ){

                    ++pos_chunk;
                    
                    *m_pos_sizepart = '\0';
                    m_lng_leftdata = strtol( m_str_sizepart, nullptr, 16 );
                    m_pos_sizepart = m_str_sizepart;
                    
#ifdef _DEBUG_CHUNKED
                    std::cout << "[[ skip_chunk : size chunk finished : str = 0x" << m_str_sizepart << " ]]\n";                    
                    std::cout << "[[ skip_chunk : enter the data chunk, data size = " << m_lng_leftdata << " ]]\n";
#endif
                    pos_data_chunk_start = pos_chunk;
                    m_status_chunk = 1;
                    
                    break;
                }
            }
        }

        // データ部
        if( m_status_chunk == 1 ){ 

            // データを前に詰める
            if( m_lng_leftdata ){
                if( m_lng_leftdata < read_size - pos_chunk ){
                    pos_chunk += m_lng_leftdata;
                    m_lng_leftdata = 0;
                }
                else{
                    m_lng_leftdata -= read_size - pos_chunk;
                    pos_chunk = read_size;
                }
                const size_t buf_size_tmp = pos_chunk - pos_data_chunk_start;
                if( buf_size != pos_data_chunk_start && buf_size_tmp ) memmove( buf + buf_size , buf + pos_data_chunk_start,  buf_size_tmp );
                buf_size +=  buf_size_tmp;
            }

            // データを全部読み込んだらデータ部終わり
            if( m_lng_leftdata == 0 ) m_status_chunk = 2;
        }

        // データ部→サイズ部切り替え中( "\r" と "\n" の間でサーバからの入力が分かれる時がある)
        if( m_status_chunk == 2 && pos_chunk < read_size ){

            if( buf[ pos_chunk++ ] != '\r' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }

            m_status_chunk = 3;
        }

        // データ部→サイズ部切り替え中("\n"の前: "\r" と "\n" の間でサーバからの入力が分かれる時がある)
        if( m_status_chunk == 3 && pos_chunk != read_size ){

            const unsigned char c = buf[ pos_chunk ];
            if( c != '\r' && c != '\n' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }

            // \r\nが来たらサイズ部に戻る
            if( c == '\r' && ++pos_chunk >= read_size ) break;
            if( buf[ pos_chunk ] != '\n' ){
                MISC::ERRMSG( "broken chunked data." );
                return false;
            }
            ++pos_chunk;

#ifdef _DEBUG_CHUNKED
            std::cout << "[[ skip_chunk : data chunk finished. ]]\n";
#endif
            m_status_chunk = 0;
        }

        // バッファ終わり
        if( pos_chunk == read_size ){
            
            read_size = buf_size;
            
#ifdef _DEBUG_CHUNKED
            std::cout << "[[ skip_chunk : output = " << read_size << " ]]\n\n";
#endif
            return true;
        }
    }

    return true;
}



//
// zlib 初期化
//
bool Loader::init_unzip()
{
#ifdef _DEBUG
    std::cout << "Loader::init_unzip\n";
#endif

    m_use_zlib = true;
        
    // zlib 初期化
    m_zstream.zalloc = Z_NULL;
    m_zstream.zfree = Z_NULL;
    m_zstream.opaque = Z_NULL;
    m_zstream.next_in = Z_NULL;
    m_zstream.avail_in = 0;

    if ( inflateInit2( &m_zstream, 15 + 32 ) != Z_OK ) // デフォルトの15に+32する( windowBits = 47 )と自動でヘッダ認識
    {
        MISC::ERRMSG( "inflateInit2 failed." );
        return false;
    }

    assert( m_buf_zlib_in.empty() );
    assert( m_buf_zlib_out.empty() );
    m_buf_zlib_in.resize( m_lng_buf_zlib_in + 64 );
    m_buf_zlib_out.resize( m_lng_buf_zlib_out + 64 );

    return true;
}



//
// unzipしてコールバック呼び出し
//
bool Loader::unzip( char* buf, std::size_t read_size )
{
    // zlibの入力バッファに値セット
    if( ( m_zstream.avail_in + read_size ) > m_lng_buf_zlib_in ){ // オーバーフローのチェック

        MISC::ERRMSG( "buffer over flow at zstream_in : " + m_data.url );
        return false;
    }
    memcpy( m_buf_zlib_in.data() + m_zstream.avail_in , buf, read_size );
    m_zstream.avail_in += read_size;
    m_zstream.next_in = m_buf_zlib_in.data();
            
    size_t byte_out = 0;
    do{

        // 出力バッファセット
        m_zstream.next_out = m_buf_zlib_out.data();
        m_zstream.avail_out = m_lng_buf_zlib_out;

        // 解凍
        const int ret = inflate( &m_zstream, Z_NO_FLUSH );
        if( ret == Z_OK || ret == Z_STREAM_END ){
            
            byte_out = m_lng_buf_zlib_out - m_zstream.avail_out;
            m_data.size_data += byte_out;
            
#ifdef _DEBUG
            std::cout << "inflate ok byte = " << byte_out << std::endl;
#endif
            
            // コールバック呼び出し
            if( byte_out && m_loadable ) m_loadable->receive( ( char* )m_buf_zlib_out.data(), byte_out );
        }
        else return true;
                
    } while ( byte_out );

    // 入力バッファに使ってないデータが残っていたら前に移動
    if( m_zstream.avail_in ) memmove( m_buf_zlib_in.data(), m_buf_zlib_in.data() + ( m_lng_buf_zlib_in - m_zstream.avail_in ),  m_zstream.avail_in );

    return true;
}



//
// ローディング終了処理
//
void Loader::finish_loading()
{
#ifdef _DEBUG
    std::cout << "Loader::finish_loading : url = " << m_data.url << std::endl;
#endif

    // SKELETON::Loadable に終了を知らせる
    if( m_loadable ) m_loadable->finish();

    m_loading = false;

    JDLIB::pop_loader_queue();
}
