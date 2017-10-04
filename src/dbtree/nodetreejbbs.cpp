// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetreejbbs.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/miscutil.h"
#include "jdlib/loaderdata.h"

#include "config/globalconf.h"

#include "httpcode.h"
#include "session.h"

#include <strings.h>


#define APPEND_SECTION( num ) do {\
if( lng_sec[ num ] ){ \
assert( m_decoded_lines.size() + lng_sec[ num ] < BUF_SIZE_ICONV_OUT ); \
m_decoded_lines.append( lines + pos_sec[ num ], lng_sec[ num ] ); \
} } while( 0 )


using namespace DBTREE;


enum
{
    MODE_NORMAL = 0,
    MODE_KAKO
};


NodeTreeJBBS::NodeTreeJBBS( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
    , m_mode( MODE_NORMAL )
{
#ifdef _DEBUG
    std::cout << "NodeTreeJBBS::NodeTreeJBBS url = " << get_url() << " modified = " << date_modified << std::endl;
#endif
}


NodeTreeJBBS::~NodeTreeJBBS()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::~NodeTreeJBBS : " << get_url() << std::endl;
#endif

    NodeTreeJBBS::clear();
}


//
// バッファなどのクリア
//
void NodeTreeJBBS::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::clear : " << get_url() << std::endl;
#endif
    NodeTreeBase::clear();

    // iconv 削除
    m_iconv.reset();

    m_decoded_lines.clear();
    m_decoded_lines.shrink_to_fit();
}



//
// ロード実行前に呼ぶ初期化関数
//
void NodeTreeJBBS::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // iconv 初期化
    if( ! m_iconv ) m_iconv.reset( new JDLIB::Iconv( CHARCODE_UTF8, DBTREE::article_charcode( get_url() ) ) );

    if( m_decoded_lines.capacity() < BUF_SIZE_ICONV_OUT ) {
        m_decoded_lines.reserve( BUF_SIZE_ICONV_OUT );
    }
}




//
// ロード用データ作成
//
void NodeTreeJBBS::create_loaderdata( JDLIB::LOADERDATA& data )
{
    if( m_mode == MODE_KAKO ) data.url = MISC::replace_str( get_url(), "rawmode", "read_archive" ) + "/";
    else data.url = get_url() + "/";
    if( id_header() ) data.url += std::to_string( id_header() + 1 ) + "-";

    // レジュームはしない
    set_resume( false );

    data.agent = DBTREE::get_agent( get_url() );
    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.cookie_for_request = DBTREE::board_cookie_for_request( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();

#ifdef _DEBUG    
    std::cout << "NodeTreeJBBS::create_loader : " << data.url << std::endl;
#endif
}


//
// キャッシュに保存する前の前処理
//
char* NodeTreeJBBS::process_raw_lines( char* rawlines, size_t& size )
{
    if( m_mode == MODE_KAKO ) html2dat( rawlines );

    return rawlines;
}


//
// ロード完了
//
void NodeTreeJBBS::receive_finish()
{
#ifdef _DEBUG
    std::cout << "NodeTreeJBBS::receive_finish : " << get_url() << std::endl
              << "mode = " << m_mode << " code = " << get_code() << std::endl;
#endif

    // 更新チェックではない、オンラインの場合はログ倉庫から取得出来るか試みる
    if( ! is_checking_update() && SESSION::is_online()
            && m_mode == MODE_NORMAL && get_error() == "STORAGE IN" ){

        m_mode = MODE_KAKO;
#ifdef _DEBUG
        std::cout << "switch mode to " << m_mode << std::endl;
#endif
        set_date_modified( std::string() );
        download_dat( false );
        return;
    }

    if( ! get_error().empty() ) set_str_code( get_error() );

    // 過去ログから読み込んだ場合は DAT 落ちにする
    if( m_mode != MODE_NORMAL ){
        set_code( HTTP_OLD );
    }

    NodeTreeBase::receive_finish();
    m_mode = MODE_NORMAL;
}


//
// raw データを dat 形式に変換
//

enum
{
    MIN_SECTION = 5,
    MAX_SECTION = 8
};

const char* NodeTreeJBBS::raw2dat( char* rawlines, int& byte )
{
    assert( m_iconv );

    int byte_lines;
    const char* lines = m_iconv->convert( rawlines, strlen( rawlines ), byte_lines );

    int number = id_header() + 1;

#ifdef _DEBUG
    std::cout << "NodeTreeJBBS::raw2dat : byte_lines = " << byte_lines << std::endl;
#endif

    // セクション分けして再合成する
    m_decoded_lines.clear();
    byte = 0;
    int pos = 0;
    int section = 0;
    int pos_sec[ MAX_SECTION ];
    int lng_sec[ MAX_SECTION ];
    memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );

    while( pos < byte_lines ){

        // セクション分け
        pos_sec[ section ] = pos;
        while( !( lines[ pos ] == '<' && lines[ pos +1 ] == '>' ) && lines[ pos ] != '\n' && pos < byte_lines ) ++pos;
        lng_sec[ section ] = pos - pos_sec[ section ];

        // 最後の行で、かつ壊れている場合
        if( pos >= byte_lines ){
            set_broken( true );
            break;
        }

        // スレを2ch型に再構築して改行
        if( lines[ pos ] == '\n' ){

            // セクション数が MIN_SECTION より小さい時はスレが壊れている
            if( section >= MIN_SECTION ){

                // 透明あぼーんの判定
                char number_str[ 64 ];
                memset( number_str, 0, 64 );
                memcpy( number_str, lines + pos_sec[ 0 ], MIN( lng_sec[ 0 ], 64 -1 ) );
                int number_in = atoi( number_str );

                while( number_in > number ){
#ifdef _DEBUG
                    std::cout << "abone : number = "<< number << " : " << number_in << std::endl;
#endif
                    constexpr char broken_str[] = "あぼ〜ん<><>あぼ〜ん<> あぼ〜ん <>\n";
                    m_decoded_lines.append( broken_str );
                    ++number;
                }

                // 名前
                APPEND_SECTION( 1 );
                m_decoded_lines.append( "<>" );

                // メアド
                APPEND_SECTION( 2 );
                m_decoded_lines.append( "<>" );

                // 日付
                APPEND_SECTION( 3 );

                // ID
                constexpr int i = 6;
                if( lng_sec[ i ] ){

                    m_decoded_lines.append( " ID:" );

                    m_decoded_lines.append( lines + pos_sec[ i ], lng_sec[ i ] );
                }
                m_decoded_lines.append( "<>" );

                // 本文
                APPEND_SECTION( 4 );
                m_decoded_lines.append( "<>" );

                // タイトル
                APPEND_SECTION( 5 );

                m_decoded_lines.push_back( '\n' );
                ++number;
            }

            // 新しい行へ移動
            ++pos;
            section = 0;
            memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );
        }

        // 次のセクションへ移動
        else{

            pos += 2;
            ++section;

            // 壊れている
            if( section >= MAX_SECTION ){

#ifdef _DEBUG
                std::cout << "NodeTreeJBBS::raw2dat : broken section = " << section-1 << std::endl;
#endif
                set_broken( true );

                // その行は飛ばす
                while( pos < byte_lines && lines[ pos ] != '\n' ) ++pos;
                ++pos;
                section = 0;
                memset( lng_sec, 0, sizeof( int ) * MAX_SECTION );
            }
        }
    }

    byte = m_decoded_lines.size();
#ifdef _DEBUG
    std::cout << "byte = " << byte << std::endl;
#endif

    return m_decoded_lines.c_str();
}


void NodeTreeJBBS::html2dat( char* lines )
{
    char* pd( lines );
    char* nextline( lines );
    char* pos;
    std::string title;
    int num_next = id_header() + 1;

    while( (pos = strchr( nextline, '\n' ) ) != NULL ){
        *pos = '\0';
        char* ps = nextline;
        nextline = pos + 1;
        if( ( pos = strstr( ps, "<dt><a name=\"" ) ) == NULL ){
            if( ( pos = strstr( ps, "<title>" ) ) != NULL ){
                // title
                pos += 7;
                char* epos;
                if( ( epos = strstr( pos, "</title>" ) ) != NULL )
                    title.assign( pos, epos - pos );
            }
        }
        else{
            int num;
            pos += 13;

            // number
            while( *pos != '>' && *pos != '\0' ) ++pos;
            if( *pos == '\0' ) continue;
            ps = pos + 1;
            pos = strstr( ps, "</a>" );
            if( pos == NULL || *pos == '\0' ) continue;
            *pos = '\0';
            num = atoi( ps );

            // 既に読み込んでいる場合は飛ばす
            if( num < num_next ) continue;

            while( ps != pos ) *(pd++) = *(ps++);
            if( *( pd - 1 ) == ' ' ) --pd;
            *(pd++) = '<'; *(pd++) = '>';
            pos += 4;
            while( *pos != '<' && *pos != '\0' ) ++pos;
            if( *pos == '\0' ) continue;
            ps = pos;

            // mail
            std::string mail;
            if( memcmp( ps, "<a href=\"mailto:", 16 ) == 0 ){
                pos = ps + 16;
                while( *pos != '\"' && *pos != '\0' ) mail.push_back( *pos++ );
                if( *pos == '\0' ) continue;
                while( *pos != '>' && *pos != '\0' ) ++pos;
                if( *pos == '\0' ) continue;
                else ++pos;
                ps = pos;
            }

            char* body;
            if( ( body = strstr( ps, "<dd>" ) ) == NULL ) continue;
            else *body = '\0';

            // name
            if( memcmp( ps, "<font color=\"#008800\">", 22 ) == 0 ) ps += 22;
            if( memcmp( ps, "<b>", 3 ) == 0 ) ps += 3;
            pos = body;
            while( pos != ps && !( *pos == '\241' && *( pos + 1 ) == '\247' ) ) --pos;
            if( pos == ps ) continue;
            char* date = pos + 2;
            while( pos != ps && !( *pos == ' ' && *( pos + 1 ) == '\305'
                                    && *( pos + 2 ) == '\352' ) ) --pos;
            if( pos == ps ) pos = date - 2;
            if( *ps == ' ' ) ++ps;
            memmove( pd, ps, pos - ps ); pd += pos - ps;
            if( strncasecmp( pd - 7, "</font>", 7 ) == 0 ) pd -= 7;
            if( strncasecmp( pd - 4, "</a>", 4 ) == 0 ) pd -= 4;
            if( strncasecmp( pd - 4, "</b>", 4 ) == 0 ) pd -= 4;
            if( *( pd - 1 ) == ' ' ) --pd;
            *(pd++) = '<'; *(pd++) = '>';
            mail.copy( pd, mail.length() );
            pd += mail.length();
            *(pd++) = '<'; *(pd++) = '>';
            ps = date;
            if( *ps == ' ' ) ++ps;

            // date
            pos = strstr( ps, " ID:" );
            if( pos == NULL ) pos = strstr( ps, " <!--" );
            if( pos == NULL || *pos == '\0' ) pos = body;
            memmove( pd, ps, pos - ps );
            pd += pos - ps;
            if( *( pd - 1 ) == ' ' ) --pd;
            if( strncasecmp( pd - 4, "<br>", 4 ) == 0 ) pd -= 4;
            *(pd++) = '<'; *(pd++) = '>';

            // id
            std::string id;
            if( pos != body ){
                pos += 4;
                if( *pos == '-' ) pos += 4;
                ps = pos;
                while( *pos != ' ' && *pos != '\0' ) ++pos;
                *pos = '\0';
                id = ps;
                if( *id.end() == ' ' ) id.resize( id.size() - 1 );
            }
            ps = body + 4;
            if( *ps == ' ' ) ++ps;

            // message
            while( ( pos = strchr( ps, '<' ) ) != NULL ){

                if( pos != ps ){
                    memmove( pd, ps, pos - ps );
                    pd += pos - ps;
                    ps = pos;
                }

                else if( ( memcmp( ps, "<a href=\"", 9 ) == 0 )
                        && ( *( ps + 9 ) != '.' && *( ps + 9 ) != '/' ) ){
                    // 外部リンクのURL
                    if( ( pos = strchr( ps + 9, '>' ) ) != NULL ){
                        ps = pos + 1;
                        if( ( pos = strstr( ps, "</a>" ) ) != NULL ){
                            memmove( pd, ps, pos - ps );
                            pd += pos - ps;
                            ps = pos + 4;
                        }
                    }
                }

                else {
                    *(pd++) = '<';
                    ps += 1;
                }
            }

            pos = ps;
            while( *pos != '\0' ) *(pd++) = *(pos++);
            if( memcmp( pd - 8, "<br><br>", 8 ) == 0 ) pd -= 8;
            if( *( pd - 1 ) == ' ' ) --pd;
            *(pd++) = '<'; *(pd++) = '>';

            if( num == 1 ){
                title.copy( pd, title.length() );
                pd += title.length();
            }
            *(pd++) = '<'; *(pd++) = '>';

            if( ! id.empty() ){
                id.copy( pd, id.length() );
                pd += id.length();
            }

            *(pd++) = '\n';
            num_next = num + 1;
        }
    }
    *pd = '\0';
}
