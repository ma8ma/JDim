// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "iconmanager.h"
#include "iconfiles.h"

#include "cache.h"

#include <cstring>

ICON::ICON_Manager* instance_icon_manager = nullptr;


ICON::ICON_Manager* ICON::get_icon_manager()
{
    if( ! instance_icon_manager ) instance_icon_manager = new ICON::ICON_Manager();
    assert( instance_icon_manager );

    return instance_icon_manager;
}


void ICON::delete_icon_manager()
{
    if( instance_icon_manager ) delete instance_icon_manager;
    instance_icon_manager = nullptr;
}


Glib::RefPtr< Gdk::Pixbuf > ICON::get_icon( int id )
{
    return get_icon_manager()->get_icon( id );
}


///////////////////////////////////////////////

using namespace ICON;


ICON_Manager::ICON_Manager()
{
    m_list_icons.resize( NUM_ICONS );

    m_list_icons[ ICON::JD16 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd16.png" );
    m_list_icons[ ICON::JD32 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd32.png" );
    m_list_icons[ ICON::JD48 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd48.png" );
    m_list_icons[ ICON::JD96 ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/jd96.png" );

    // サイドバーで使用するアイコン
    m_list_icons[ ICON::DIR ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/dir.png" );
    m_list_icons[ ICON::IMAGE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/image.png" );
    m_list_icons[ ICON::LINK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/link.png" );

    // サイドバーやタブで使用するアイコン
    m_list_icons[ ICON::BOARD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board.png" );
    m_list_icons[ ICON::BOARD_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board_update.png" );
    m_list_icons[ ICON::THREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread.png" );
    m_list_icons[ ICON::THREAD_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_update.png" );
    m_list_icons[ ICON::THREAD_OLD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_old.png" );

    // タブで使用するアイコン
    m_list_icons[ ICON::BOARD_UPDATED ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/board_updated.png" );
    m_list_icons[ ICON::THREAD_UPDATED ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread_updated.png" );
    m_list_icons[ ICON::LOADING ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/loading.png" );
    m_list_icons[ ICON::LOADING_STOP ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/loading_stop.png" );

    // スレ一覧で使用するアイコン
    m_list_icons[ ICON::BKMARK_UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_update.png" );
    m_list_icons[ ICON::BKMARK_BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_broken_subject.png" );
    m_list_icons[ ICON::BKMARK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark.png" );
    m_list_icons[ ICON::UPDATE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/update.png" );
    m_list_icons[ ICON::NEWTHREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/newthread.png" );
    m_list_icons[ ICON::NEWTHREAD_HOUR ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/newthread_hour.png" );
    m_list_icons[ ICON::BROKEN_SUBJECT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/broken_subject.png" );
    m_list_icons[ ICON::CHECK ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/check.png" );
    m_list_icons[ ICON::OLD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/down.png" );
    m_list_icons[ ICON::INFO ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/info.png" );

    // スレビューで使用するアイコン
    m_list_icons[ ICON::BKMARK_THREAD ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/bkmark_thread.png" );
    m_list_icons[ ICON::POST ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/post.png" );
    m_list_icons[ ICON::POST_REFER ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/post_refer.png" );

    // その他
    m_list_icons[ ICON::DOWN ] = m_list_icons[ ICON::OLD ];

    m_list_icons[ ICON::TRANSPARENT ] = Gdk::Pixbuf::create( Gdk::COLORSPACE_RGB, true, 8, 1, 1 );
    m_list_icons[ ICON::TRANSPARENT ]->fill( 0 );


    //////////////////////////////
    // ツールバーのアイコン

    // アイコン名はfreedesktop.orgの規格とGTK3デフォルトテーマのAdwaitaを参照する
    // https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
    // https://gitlab.gnome.org/GNOME/adwaita-icon-theme

    const auto icon_theme = Gtk::IconTheme::get_default();
    constexpr int size_menu = 16; // Gtk::ICON_SIZE_MENU

    // 共通
    m_list_icons[ ICON::SEARCH_PREV ] = icon_theme->load_icon( "go-up", size_menu );
    m_list_icons[ ICON::SEARCH_NEXT ] = icon_theme->load_icon( "go-down", size_menu );
    m_list_icons[ ICON::STOPLOADING ] = icon_theme->load_icon( "process-stop", size_menu );
    m_list_icons[ ICON::WRITE ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/write.png" );
    m_list_icons[ ICON::RELOAD ] = icon_theme->load_icon( "view-refresh", size_menu );
    m_list_icons[ ICON::APPENDFAVORITE ] = icon_theme->load_icon( "edit-copy", size_menu );
    m_list_icons[ ICON::DELETE ] = icon_theme->load_icon( "edit-delete", size_menu );
    m_list_icons[ ICON::QUIT ] = icon_theme->load_icon( "window-close", size_menu );
    m_list_icons[ ICON::BACK ] = icon_theme->load_icon( "go-previous", size_menu );
    m_list_icons[ ICON::FORWARD ] = icon_theme->load_icon( "go-next", size_menu );
    try {
        // changes-prevent is not a standard icon name.
        m_list_icons[ ICON::LOCK ] = icon_theme->load_icon( "changes-prevent-symbolic", size_menu );
    }
    catch( Gtk::IconThemeError& ) {
        m_list_icons[ ICON::LOCK ] = icon_theme->load_icon( "window-close", size_menu );
    }

    // メイン
    m_list_icons[ ICON::BBSLISTVIEW ] = m_list_icons[ ICON::DIR ];
    m_list_icons[ ICON::FAVORITEVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/favorite.png" );
    m_list_icons[ ICON::HISTVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist.png" );
    m_list_icons[ ICON::HIST_BOARDVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_board.png" );
    m_list_icons[ ICON::HIST_CLOSEVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_close.png" );
    m_list_icons[ ICON::HIST_CLOSEBOARDVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_closeboard.png" );
    m_list_icons[ ICON::HIST_CLOSEIMGVIEW ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/hist_closeimg.png" );
    m_list_icons[ ICON::BOARDVIEW ] = m_list_icons[ ICON::BOARD ];
    m_list_icons[ ICON::ARTICLEVIEW ] = m_list_icons[ ICON::THREAD ];
    m_list_icons[ ICON::IMAGEVIEW ] = m_list_icons[ ICON::IMAGE ];
    m_list_icons[ ICON::GO ] = icon_theme->load_icon( "go-jump", size_menu );
    m_list_icons[ ICON::UNDO ] = icon_theme->load_icon( "edit-undo", size_menu );
    m_list_icons[ ICON::REDO ] = icon_theme->load_icon( "edit-redo", size_menu );

    // サイドバー
    m_list_icons[ ICON::CHECK_UPDATE_ROOT ] = icon_theme->load_icon( "view-refresh", size_menu );
    m_list_icons[ ICON::CHECK_UPDATE_OPEN_ROOT ] = Gdk::Pixbuf::create_from_resource( "/com/github/jdimproved/JDim/thread.png" );

    // スレビュー
    m_list_icons[ ICON::SEARCH ] = icon_theme->load_icon( "edit-find", size_menu );
    m_list_icons[ ICON::LIVE ] = icon_theme->load_icon( "media-playback-start", size_menu );

    // 検索バー
    m_list_icons[ ICON::CLOSE_SEARCH ] = icon_theme->load_icon( "edit-undo", size_menu );
    m_list_icons[ ICON::CLEAR_SEARCH ] = icon_theme->load_icon( "edit-clear", size_menu );
    m_list_icons[ ICON::SEARCH_AND ] = icon_theme->load_icon( "edit-cut", size_menu );
    m_list_icons[ ICON::SEARCH_OR ] = icon_theme->load_icon( "list-add", size_menu );

    // 書き込みビュー
    m_list_icons[ ICON::PREVIEW ] = m_list_icons[ ICON::THREAD ];
    m_list_icons[ ICON::INSERTTEXT ] = icon_theme->load_icon( "document-open", size_menu );

    load_theme();
}


ICON_Manager::~ICON_Manager()
{
#ifdef _DEBUG
    std::cout << "ICON::~ICON_Manager\n";
#endif
}


Glib::RefPtr< Gdk::Pixbuf > ICON_Manager::get_icon( const int id )
{
    return m_list_icons[ id ];
}


//
// アイコンテーマ読み込み
//
void ICON_Manager::load_theme()
{
    if( CACHE::file_exists( CACHE::path_theme_icon_root() ) != CACHE::EXIST_DIR ) return;

    const std::list< std::string > files = CACHE::get_filelist( CACHE::path_theme_icon_root() );
    if( ! files.size() ) return;

#ifdef _DEBUG
    std::cout << "ICON::load_theme\n";
#endif

    std::list< std::string >::const_iterator it = files.begin();
    for(; it != files.end(); ++it ){

#ifdef _DEBUG
        std::cout << *it << std::endl;
#endif

        int id = 0;

        // 拡張子を削除
        std::string filename = (*it);
        size_t i = (*it).rfind( '.' );
        if( i != std::string::npos ) filename = filename.substr( 0, i );

        while( iconfiles[ id ][ 0 ] != '\0' ){

            if( iconfiles[ id ][ 0 ] == filename[ 0 ]
                && filename.compare( iconfiles[ id ] ) == 0 ){
#ifdef _DEBUG
                std::cout << "hit : " << iconfiles[ id ] << " id = " << id << std::endl;
#endif
                m_list_icons[ id ]  = Gdk::Pixbuf::create_from_file( CACHE::path_theme_icon_root() + (*it) );
                break;
            }

            ++id;
        }
    }
}
