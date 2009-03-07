// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "environment.h"

#include "cache.h"
#include "config/globalconf.h"
#include "jdversion.h"
#include "jdlib/miscutil.h"

#include <cstring>

#include "gtkmm.h"

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h> // uname()
#endif


std::string ENVIRONMENT::get_jdcomments(){ return std::string( JDCOMMENT ); }
std::string ENVIRONMENT::get_jdcopyright(){ return std::string( JDCOPYRIGHT ); }
std::string ENVIRONMENT::get_jdbbs(){ return std::string( JDBBS ); }
std::string ENVIRONMENT::get_jd2chlog(){ return std::string( JD2CHLOG ); }
std::string ENVIRONMENT::get_jdhelp(){ return std::string( JDHELP ); }
std::string ENVIRONMENT::get_jdhelpcmd(){ return std::string( JDHELPCMD ); }
std::string ENVIRONMENT::get_jdlicense(){ return std::string( JDLICENSE ); }


//
// CONFIGURE_ARGSを返す
//
// mode: 整形モード(デフォルト = CONFIGURE_OMITTED 省略したものを一行で)
//
std::string ENVIRONMENT::get_configure_args( const int mode )
{
    std::string configure_args;

#ifdef CONFIGURE_ARGS

    const std::string args = CONFIGURE_ARGS;

    // FULLはそのまま返す
    if( mode == CONFIGURE_FULL ) return args;

    size_t search_pos = 0, found_pos = 0;
    const size_t end_quote_pos = args.rfind( "'" );

    // 複数の項目
    bool multi = false;

    // 省略形として"--with-"や"--enable-"などを取り出す
    while( ( found_pos = args.find( "'--with-", search_pos ) ) != std::string::npos ||
            ( found_pos = args.find( "'--enable-", search_pos ) ) != std::string::npos )
    {
        size_t quote_pos = args.find( "'", found_pos + 1 );

        if( quote_pos != std::string::npos && quote_pos != end_quote_pos)
        {
            // 項目が複数の場合は改行かスペースを付与
            if( multi )
            {
                if( mode == CONFIGURE_OMITTED_MULTILINE ) configure_args.append( "\n" );
                else configure_args.append( " " );
            }
            multi = true;

            configure_args.append( args.substr( found_pos, ( quote_pos - found_pos ) + 1 ) );
            search_pos = quote_pos + 1;
        }
        else
        {
            configure_args.append( args.substr( found_pos ) );
            break;
        }
    }

#endif

    return configure_args;
}


//
// SVNリビジョンとして表示する文字列を返す
//
std::string get_svn_revision( const char* rev = NULL )
{
    std::string svn_revision = "SVN:";

    if( rev )
    {
        // "2000:2002MS"など[0-9:MS]の形式かどうか
        bool valid = true;
        unsigned int n;
        const size_t rev_length = strlen( rev );
        for( n = 0; n < rev_length; ++n )
        {
            if( (unsigned char)( rev[n] - 0x30 ) > 0x0A
                && rev[n] != 'M'
                && rev[n] != 'S' )
            {
                valid = false;
                break;
            }
        }

        if( valid ) svn_revision.append( std::string( "Rev." ) + std::string( rev ) );
    }

    if( svn_revision.compare( "SVN:" ) == 0 )
    {
        svn_revision.append( std::string( __DATE__ ) + "-" + std::string( __TIME__ ) );
    }

    return svn_revision;
}


//
// JDのバージョンを取得
//
std::string ENVIRONMENT::get_jdversion()
{
    std::stringstream jd_version;

#ifdef JDVERSION_SVN

#ifdef SVN_REVISION
    jd_version << get_svn_revision( SVN_REVISION );
#else
    jd_version << get_svn_revision( NULL );
#endif // SVN_REVISION

#else
    jd_version << MAJORVERSION << "."
                << MINORVERSION << "."
                << MICROVERSION << "-"
                << JDTAG << JDDATE;
#endif // JDVERSION_SVN

    return jd_version.str();
}


//
// ファイル等からディストリ名を取得
//
std::string ENVIRONMENT::get_distname()
{
#ifdef _DEBUG
    std::cout << "SESSION::get_dist_name\n";
#endif

    std::string tmp;
    std::string text_data;

    // LSB系 ( Ubuntu ..etc )
    if( CACHE::load_rawdata( "/etc/lsb-release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::reverse_iterator it = lines.rbegin();
        while( it != lines.rend() )
        {
            std::string lsb_name, lsb_data;

            size_t e;
            if( ( e = (*it).find( "=" ) ) != std::string::npos )
            {
                lsb_name = MISC::remove_spaces( (*it).substr( 0, e ) );
                lsb_data = MISC::remove_spaces( (*it).substr( e + 1 ) );
            }

            // 「DISTRIB_DESCRIPTION="Ubuntu 7.10"」などから「Ubuntu 7.10」を取得
            if( lsb_name == "DISTRIB_DESCRIPTION" && ! lsb_data.empty() )
            {
                tmp = MISC::cut_str( lsb_data, "\"", "\"" );
                break;
            }

            ++it;
        }
    }
    // KNOPPIX (LSB？)
    else if( CACHE::load_rawdata( "/etc/knoppix-version", text_data ) )
    {
        tmp = "KNOPPIX ";
        tmp.append( text_data );
    }
    // Debian
    else if( CACHE::load_rawdata( "/etc/debian_version", text_data ) )
    {
        tmp = "Debian GNU/Linux ";
        tmp.append( text_data );
    }
    // Arch Linux (2009/02/23現在"/etc/arch-release"は空)
    else if( CACHE::file_exists( "/etc/arch-release" ) == CACHE::EXIST_FILE )
    {
        tmp = "Arch Linux";
    }
    // Solaris系
    else if( CACHE::load_rawdata( "/etc/release", text_data ) )
    {
        std::list< std::string > lines = MISC::get_lines( text_data );
        std::list< std::string >::iterator it = lines.begin();
        while( it != lines.end() )
        {
            // 名前が含まれている行を取得
            if( (*it).find( "BeleniX" ) != std::string::npos
                || (*it).find( "Nexenta" ) != std::string::npos
                || (*it).find( "SchilliX" ) != std::string::npos
                || (*it).find( "Solaris" ) != std::string::npos )
            {
                tmp = *it;
                break;
            }

            ++it;
        }
    }
    // ファイルの中身がそのままディストリ名として扱える物
    else
    {
        // ディストリ名が書かれているファイル
        std::string dist_files[] =
        {
            "/etc/fedora-release",
            "/etc/gentoo-release",
            "/etc/lfs-release",
            "/etc/mandriva-release",
            "/etc/momonga-release",
            "/usr/lib/setup/plamo-version",
            "/etc/puppyversion",
            "/etc/redhat-release", // Redhat, CentOS, WhiteBox, PCLinuxOS
            "/etc/sabayon-release",
            "/etc/slackware-version",
            "/etc/SuSE-release",
            "/etc/turbolinux-release",
            "/etc/vine-release",
            "/etc/zenwalk-version"
        };

        unsigned int i;
        for( i = 0; i < sizeof( dist_files ) / sizeof( std::string ); ++i )
        {
            if( CACHE::load_rawdata( dist_files[i], text_data ) )
            {
                tmp = text_data;
                break;
            }
        }
    }

    // 文字列両端のスペースなどを削除する
    std::string dist_name = MISC::remove_spaces( tmp );

    // 取得した文字が異常に長い場合は空にする
    if( dist_name.length() > 50 ) dist_name.clear();

#ifdef HAVE_SYS_UTSNAME_H

    char *sysname = 0, *release = 0, *machine = 0;

    // システムコール uname() 準拠：SVr4, POSIX.1-2001.
    struct utsname* uts;
    uts = (struct utsname*)malloc( sizeof( struct utsname ) );
    if( uname( uts ) != -1 )
    {
        sysname = uts->sysname;
        release = uts->release;
        machine = uts->machine;
    }

    // FreeBSD等やディストリ名が取得できなかった場合は"$ uname -rs"と同じ様式
    if( dist_name.empty() && sysname && release )
    {
        dist_name = std::string( sysname ) + " " + std::string( release );
    }

    // アーキテクチャがx86でない場合
    if( machine &&
        ( strlen( machine ) != 4
          || ! ( machine[0] == 'i'
               && machine[1] >= '3' && machine[1] <= '6'
               && machine[2] == '8' && machine[3] == '6' ) ) )
    {
        dist_name.append( " (" + std::string( machine ) + ")" );
    }

    free( uts );
    uts = NULL;

#endif

    return dist_name;
}


//
// WM 判定
// TODO: 環境変数で判定できない場合の判定方法を考える
//
int ENVIRONMENT::get_wm()
{
    int window_manager = WM_UNKNOWN;
    const std::string str_wm = MISC::getenv_limited( "DESKTOP_SESSION", 5 );

    if( str_wm.find( "xfce" ) != std::string::npos ) window_manager = WM_XFCE;
    else if( str_wm.find( "gnome" ) != std::string::npos ) window_manager = WM_GNOME;
    else if( str_wm.find( "kde" ) != std::string::npos ) window_manager = WM_KDE;

    if( window_manager == WM_UNKNOWN )
    {
        if( ! MISC::getenv_limited( "GNOME_DESKTOP_SESSION_ID" ).empty() )
        {
            window_manager = WM_GNOME;
        }
        else
        {
            const std::string str_wm = MISC::getenv_limited( "KDE_FULL_SESSION", 4 );
            if( str_wm == "true" ) window_manager = WM_KDE;
        }
    }

    return window_manager;
}


//
// WM名を文字列で返す
//
std::string ENVIRONMENT::get_wm_str()
{
    std::string desktop;

    switch( get_wm() )
    {
        case WM_GNOME : desktop = "GNOME"; break;
        case WM_XFCE  : desktop = "XFCE";  break;
        case WM_KDE   : desktop = "KDE";   break;
    }

    return desktop;
}


//
// gtkmmのバージョンを取得
//
std::string ENVIRONMENT::get_gtkmm_version()
{
    std::stringstream gtkmm_ver;
    gtkmm_ver << GTKMM_MAJOR_VERSION << "."
               << GTKMM_MINOR_VERSION << "."
               << GTKMM_MICRO_VERSION;

    return gtkmm_ver.str();
}


//
// glibmmのバージョンを取得
//
std::string ENVIRONMENT::get_glibmm_version()
{
    std::stringstream glibmm_ver;
    glibmm_ver << GLIBMM_MAJOR_VERSION << "."
                << GLIBMM_MINOR_VERSION << "."
                << GLIBMM_MICRO_VERSION;

    return glibmm_ver.str();
}


//
// 動作環境を取得
//
std::string ENVIRONMENT::get_jdinfo()
{
    std::stringstream jd_info;

    // バージョンを取得(jdversion.h)
    const std::string version = get_jdversion();

    // ディストリビューション名を取得
    const std::string distribution = get_distname();

    // デスクトップ環境を取得( 環境変数から判別可能の場合 )
    std::string desktop = get_wm_str();

    // その他
    std::string other;

    // $LANG が ja_JP.UTF-8 でない場合は"その他"に追加する。
    const std::string lang = MISC::getenv_limited( "LANG", 11 );
    if( lang.empty() ) other.append( "LANG 未定義" );
    else if( lang != "ja_JP.utf8" && lang != "ja_JP.UTF-8" ) other.append( "LANG = " + lang );

    jd_info <<
    "[バージョン] " << version << "\n" <<
//#ifdef SVN_REPOSITORY
//    "[リポジトリ ] " << SVN_REPOSITORY << "\n" <<
//#endif
    "[ディストリ ] " << distribution << "\n" <<
    "[パッケージ] " << "バイナリ/ソース( <配布元> )" << "\n" <<
    "[ DE／WM ] " << desktop << "\n" <<
    "[　gtkmm 　] " << get_gtkmm_version() << "\n" <<
    "[　glibmm 　] " << get_glibmm_version()<< "\n" <<
#ifdef CONFIGURE_ARGS
    "[オプション ] " << get_configure_args( CONFIGURE_OMITTED_MULTILINE ) << "\n" <<
#endif
    "[ そ の 他 ] " << other << "\n";

    return jd_info.str();
}

