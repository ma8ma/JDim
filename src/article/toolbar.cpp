// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbar.h"
#include "articleadmin.h"
#include "articleviewbase.h"

#include "config/globalconf.h"
#include "control/controlid.h"
#include "control/controlutil.h"
#include "icons/iconmanager.h"
#include "jdlib/gtk_compat.h"

#include "command.h"
#include "session.h"
#include "compmanager.h"
#include "global.h"

using namespace ARTICLE;


ArticleToolBar::ArticleToolBar()
    : SKELETON::ToolBar( ARTICLE::get_admin() )
    , m_enable_slot( true )
    , m_button_drawout_and( ICON::SEARCH_AND )
    , m_button_drawout_or( ICON::SEARCH_OR )
{
    // 検索バー
    set_tooltip( m_button_drawout_and, CONTROL::get_label_motions( CONTROL::DrawOutAnd ) );
    set_tooltip( m_button_drawout_or, CONTROL::get_label_motions( CONTROL::DrawOutOr ) );

    JDLIB::compat::box_append_expand( *get_searchbar(), *get_tool_search( CORE::COMP_SEARCH_ARTICLE ) );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_down_search() );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_up_search() );
    JDLIB::compat::box_append_shrink( *get_searchbar(), m_button_drawout_and );
    JDLIB::compat::box_append_shrink( *get_searchbar(), m_button_drawout_or );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_clear_highlight() );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_close_searchbar() );

    m_button_drawout_or.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_or ) );
    m_button_drawout_and.signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_drawout_and ) );

    ArticleToolBar::pack_buttons();
}
        

ArticleToolBar::~ArticleToolBar() noexcept = default;


//
// タブが切り替わった時にDragableNoteBook::set_current_toolbar()から呼び出される( Viewの情報を取得する )
//
// virtual
void ArticleToolBar::set_view( SKELETON::View * view )
{
    SKELETON::ToolBar::set_view( view, CONFIG::get_share_query_among_tabs() );

    m_enable_slot = false;

    // ArticleViewBase固有の情報をコピー
    const ArticleViewBase* articleview = dynamic_cast<ArticleViewBase*>( view );
    if( articleview ){

        m_url_article = articleview->url_article();

        if( m_button_live_play_stop ){
            bool sensitive = true;
            if( ! articleview->get_enable_live() ) sensitive = false;
            m_button_live_play_stop->set_sensitive( sensitive );

            if( articleview->get_live() ) m_button_live_play_stop->set_active( true );
            else m_button_live_play_stop->set_active( false );
        }
    }

    m_enable_slot = true;
}


/**
 * @brief ボタンのアイコンを再読み込み
 */
void ArticleToolBar::reload_ui_icon()
{
    SKELETON::ToolBar::reload_ui_icon();

    set_button_icon( &m_button_drawout_and, ICON::SEARCH_AND );
    set_button_icon( &m_button_drawout_or, ICON::SEARCH_OR );
    set_button_icon( m_button_live_play_stop, ICON::LIVE );
}


//
// ボタンのパッキング
//
// virtual
void ArticleToolBar::pack_buttons()
{
    int num = 0;
    for(;;){
        int item = SESSION::get_item_article_toolbar( num );
        if( item == ITEM_END ) break;
        switch( item ){

            case ITEM_WRITEMSG:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_write() );
                break;

            case ITEM_OPENBOARD:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_board() );
                break;

            case ITEM_NAME:
                get_label()->set_margin_start( 8 );
                JDLIB::compat::box_append_expand( get_buttonbar(), *get_label() );
                get_label()->set_margin_end( 8 );
                break;

            case ITEM_SEARCH:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_open_searchbar() );
                break;

            case ITEM_RELOAD:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_reload() );
                break;

            case ITEM_STOPLOADING:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_stop() );
                break;

            case ITEM_APPENDFAVORITE:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_favorite() );
                set_tooltip( *get_button_favorite(), CONTROL::get_label_motions( CONTROL::AppendFavorite )
                             + "\n\nスレのタブをお気に入りに直接Ｄ＆Ｄしても登録可能" );

                break;

            case ITEM_DELETE:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_delete() );
                // ポップアップメニューの配置に利用するためツールバーボタンをIDに紐づけします。
                get_admin()->set_anchor_widget( kToolbarWidgetDelete, get_button_delete() );

                break;

            case ITEM_QUIT:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_close() );
                break;

            case ITEM_BACK:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_back() );
                break;

            case ITEM_FORWARD:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_forward() );
                break;

            case ITEM_LOCK:
                JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_lock() );
                break;

            case ITEM_LIVE:
                if( ! m_button_live_play_stop ){
                    m_button_live_play_stop = Gtk::manage( new SKELETON::ImgToggleToolButton( ICON::LIVE ) );
                    set_tooltip( *m_button_live_play_stop, CONTROL::get_label_motions( CONTROL::LiveStartStop ) );
                    m_button_live_play_stop->signal_clicked().connect( sigc::mem_fun(*this, &ArticleToolBar::slot_live_play_stop ) );
                }
                JDLIB::compat::box_append_shrink( get_buttonbar(), *m_button_live_play_stop );

                break;

            case ITEM_SEPARATOR:
                pack_separator();
                break;
        }
        ++num;
    }

    set_relief();
    show_all_children();
}


//
// キーワード抽出 (AND)
//
void ArticleToolBar::slot_drawout_and()
{
    if( ! m_enable_slot ) return;    

    std::string query = get_search_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
}


//
// キーワード抽出 (OR)
//
void ArticleToolBar::slot_drawout_or()
{
    if( ! m_enable_slot ) return;    

    std::string query = get_search_text();
    if( query.empty() ) return;

    CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "true" );
}


//
// 実況開始/停止
//
void ArticleToolBar::slot_live_play_stop()
{
    if( ! m_enable_slot ) return;    

    ARTICLE::get_admin()->set_command( "live_start_stop", get_url() );
}
