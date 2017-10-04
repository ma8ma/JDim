// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/jdregex.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"
#include "login2ch.h"

#include <sstream>

using namespace DBTREE;


enum
{
    MODE_NORMAL = 0,
    MODE_CGI,
    MODE_KAKO_GZ,
    MODE_KAKO,
    MODE_KAKO_EXT,
};


NodeTree2ch::NodeTree2ch( const std::string& url, const std::string& org_url,
                          const std::string& date_modified, time_t since_time )
    : NodeTree2chCompati( url, date_modified )
    , m_org_url( org_url )
    , m_since_time( since_time )
    , m_mode( MODE_NORMAL )
    , m_res_number_max( -1 )
{
#ifdef _DEBUG
    std::cout << "NodeTree2ch::NodeTree2ch url = " << url << std::endl
              << "org_url = " << m_org_url << " modified = " << date_modified
              << " since = " << m_since_time << std::endl;
#endif
}



NodeTree2ch::~NodeTree2ch()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::~NodeTree2ch : " << get_url() << std::endl;
#endif
}


//
// キャッシュに保存する前の前処理
//
// 先頭にrawモードのステータスが入っていたら取り除く
//
char* NodeTree2ch::process_raw_lines( char* rawlines, size_t& size )
{
    if( ! is_loading() && memcmp( rawlines, "<dt>", 4 ) == 0 ){
#ifdef _DEBUG
        std::cout << "NodeTree2ch::process_raw_lines ignore HTML lines" << std::endl;
#endif
        *rawlines = '\0';
    }
    else if( ! is_loading() && memcmp( rawlines, "ng (", 4 ) == 0
            && strstr( rawlines, "<>" ) == NULL ){
        set_code( HTTP_ERR );
        set_str_code( rawlines );
        MISC::ERRMSG( rawlines );
        *rawlines = '\0';
    }
    else if( size == 16 && memcmp( rawlines, "Dat doesnt exist", 16 ) == 0 ){
        set_ext_err( rawlines );
        *rawlines = '\0';
    }
    else if( m_mode == MODE_NORMAL && is_loading() && ! id_header() ){
        char *l1, *l2;
        if( ! ( l1 = strchr( rawlines, '\n' ) ) ) return rawlines;
        if( ! ( l2 = strchr( l1 + 1, '\n' ) ) || *( l2 + 1 ) != '\0' ) return rawlines;

        char *id = strstr( l1 + 1, " ID:\?\?\?\?\?\?\?\?<>" );
        if( id && id < l2 ){
            // ログ落ち案内レス
            m_operate_info = l1 + 1;       // レスをプールする
            *( l1 + 1 ) = '\0';
        }
    }

    return rawlines;
}


//
// 拡張属性を取り出す
//
void NodeTree2ch::parse_extattr( const char* str, const int lng )
{
    const char* pos = str;

    while( pos < str + lng && ( pos = (const char *)memchr( pos, '<', str + lng - pos ) ) != NULL ){
        if( memcmp( pos, "<hr>VIPQ2_EXTDAT: ", 18 ) == 0 ){ pos += 18; break; }
        ++pos;
    }

    if( pos != NULL && pos < str + lng ){
        JDLIB::Regex regex;
        constexpr size_t offset = 0;
        constexpr bool icase = false; // 大文字小文字区別しない
        constexpr bool newline = true; // . に改行をマッチさせない
        constexpr bool usemigemo = false; // migemo使用
        constexpr bool wchar = false; // 全角半角の区別をしない

        const std::string extattr( pos, str + lng - pos );
        if( regex.exec( "[^:]+:[^:]+:([^:]+):[^ ]+ EXT was configured",
                    extattr.c_str(), offset, icase, newline, usemigemo, wchar ) ){

            // 最大レス数を取得
            if( regex.str( 1 ) == "V" ) m_res_number_max = 0;
            else if( regex.str( 1 )[ 0 ] >= '0' && regex.str( 1 )[ 0 ] <= '9' ){
                m_res_number_max = atoi( regex.str( 1 ).c_str() );
            }
        }
    }
}


//
// ロード用データ作成
//
void NodeTree2ch::create_loaderdata( JDLIB::LOADERDATA& data )
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::create_loaderdata : mode = " << m_mode << " url = " << get_url() << std::endl;

#endif

    data.byte_readfrom = 0;

    JDLIB::Regex regex;
    const size_t offset = 0;
    const bool icase = false;
    const bool newline = true;
    const bool usemigemo = false;
    const bool wchar = false;

    // ( rokka, 旧offlaw, offlaw2は廃止 )
    // 過去ログ倉庫使用
    if( m_mode == MODE_KAKO_GZ || m_mode == MODE_KAKO ){

        if( ! regex.exec( "(https?://[^/]*)(/.*)/dat(/.*)\\.dat$", m_org_url, offset, icase, newline, usemigemo, wchar ) ) return;
        const int id = atoi( regex.str( 3 ).c_str() + 1 );

        std::string url;

        // スレIDが10桁の場合 → http://サーバ/板ID/kako/IDの上位4桁/IDの上位5桁/ID.dat.gz
        if( id / 1000000000 ) url = regex.str( 1 ) + regex.str( 2 ) + "/kako/" + std::to_string( id / 1000000 )
                                    + "/" + std::to_string( id / 100000 ) + regex.str( 3 );

        // スレIDが9桁の場合 → http://サーバ/板ID/kako/IDの上位3桁/ID.dat.gz
        else url = regex.str( 1 ) + regex.str( 2 ) + "/kako/" + std::to_string( id / 1000000 ) + regex.str( 3 );

        if( m_mode == MODE_KAKO_GZ ) url += ".dat.gz";
        else url += ".dat";

        data.url = url;

        // レジューム設定
        if( get_lng_dat() ) set_resume( true );
        else set_resume( false );
    }

    else if( m_mode == MODE_KAKO_EXT ){
        if( ! regex.exec( "https?://([^./]+)\\.[^/]+/(.*)/dat/(.*)\\.dat$", m_org_url, offset, icase, newline, usemigemo, wchar ) ) return;

        std::string cmd = CONFIG::get_url_external_log();
        cmd = MISC::replace_str( cmd, "$OLDHOST", regex.str( 1 ) );
        cmd = MISC::replace_str( cmd, "$BBSNAME", regex.str( 2 ) );
        cmd = MISC::replace_str( cmd, "$DATNAME", regex.str( 3 ) );

        data.url = cmd;

        // レジューム設定
        if( get_lng_dat() ) set_resume( true );
        else set_resume( false );
    }

    // 普通の読み込み
    else{

        data.url = get_url();

        // レジューム設定
        // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
        if( get_lng_dat() ) {
            data.byte_readfrom = get_lng_dat() -1;
            set_resume( true );
        }
        else set_resume( false );
    }

#ifdef _DEBUG    
    std::cout << "load from " << data.url << std::endl;
#endif

    if( !CONFIG::get_url_login2ch().compare( 0, CONFIG::get_url_login2ch().length(), data.url, 0, CONFIG::get_url_login2ch().length() ) ){
        if( data.agent.empty() ) data.agent = CONFIG::get_agent_for2ch();

        if( CONFIG::get_use_proxy_for2ch() ){
            data.host_proxy = CONFIG::get_proxy_for2ch();
            data.port_proxy = CONFIG::get_proxy_port_for2ch();
        }
        data.basicauth_proxy = CONFIG::get_proxy_basicauth_for2ch();
    }
    else{
        if( data.agent.empty() ) data.agent = DBTREE::get_agent( data.url );

        data.host_proxy = DBTREE::get_proxy_host( data.url );
        data.port_proxy = DBTREE::get_proxy_port( data.url );
        data.basicauth_proxy = DBTREE::get_proxy_basicauth( data.url );
    }

    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();

#ifdef _DEBUG
    std::cout << "agent = " << data.agent << std::endl;
#endif

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
}


//
// ロード完了
//
void NodeTree2ch::receive_finish()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2ch::receive_finish : " << get_url() << std::endl
              << "mode = " << m_mode << " code = " << get_code() << std::endl;
#endif

    // 保留データを処理する
    sweep_buffer();

    // 更新チェックではない、オンラインの場合は read.cgi や 過去ログ倉庫から取得出来るか試みる
    if( ! is_checking_update()
        && SESSION::is_online()
        && ( get_code() == HTTP_REDIRECT || get_code() == HTTP_MOVED_PERM
            || get_code() == HTTP_NOT_FOUND || get_code() == HTTP_NOT_IMPREMENTED
            || ( ( m_mode == MODE_KAKO || m_mode == MODE_KAKO_GZ )
                     && get_code() == HTTP_AUTH_REQ ) // 過去ログ読み込み失敗
            || ( m_mode == MODE_CGI && get_code() == HTTP_ORIGN_ERR )
            || ! get_ext_err().empty() // スレが存在しない
            )
        ){

/*
  (1) 読み込み( dat直接, read.cgi, API )
  (2) 過去ログ倉庫(gz)から読み込み (※1)
    ・スレIDが10桁の場合 → http://サーバ/板ID/kako/IDの上位4桁/IDの上位5桁/ID.dat.gz
    ・スレIDが9桁の場合 → http://サーバ/板ID/kako/IDの上位3桁/ID.dat.gz
  (3) 過去ログ倉庫から読み込み (※2)
    ・スレIDが10桁の場合 → http://サーバ/板ID/kako/IDの上位4桁/IDの上位5桁/ID.dat
    ・スレIDが9桁の場合 → http://サーバ/板ID/kako/IDの上位3桁/ID.dat
  (4) 外部のログ保存サービスから読み込み
  (5) read.cgiで読み込んでない場合にはread.cgiからの読み込み


  (※1)ただし 2008年1月1日以降に立てられたスレは除く
  (※2)少なくても rokka導入(2013年9月27日)以降に立てられたスレ除く

  (注) 古すぎる(2000年頃)のdatは形式が違う(<>ではなくて,で区切られている)ので読み込みに考慮が必要

*/

        // 過去ログ倉庫(gz圧縮)
        if( m_mode <= MODE_CGI && m_since_time < 1380246829 ){
            // ただし 2008年1月1日以降に立てられたスレは除く
            if( m_since_time < 1199113200 ) m_mode = MODE_KAKO_GZ;
            else m_mode = MODE_KAKO;
        }

        // 過去ログ倉庫
        else if( m_mode == MODE_KAKO_GZ ) m_mode = MODE_KAKO;

        // 外部のログ保存サービス
        else if( m_mode != MODE_KAKO_EXT && CONFIG::get_use_external_log() ) m_mode = MODE_KAKO_EXT;

        // 失敗
        else m_mode = MODE_NORMAL;

        if( m_mode != MODE_NORMAL ){
#ifdef _DEBUG
            std::cout << "switch mode to " << m_mode << std::endl;
#endif
            set_date_modified( std::string() );
            set_ext_err( std::string() );
            download_dat( false );
            return;
        }

        // プールしていたレスを追加
        if( ! m_operate_info.empty() ){
            NodeTreeBase::receive_data( m_operate_info.c_str(), m_operate_info.length() );
        }
    }

    m_operate_info = std::string();

    // 過去ログから読み込んだ場合は DAT 落ちにする
    if( m_mode >= MODE_KAKO_GZ ){
        set_code( HTTP_OLD );
    }

    NodeTreeBase::receive_finish();
    m_mode = MODE_NORMAL;
}
