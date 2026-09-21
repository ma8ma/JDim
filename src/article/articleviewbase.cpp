// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "gtkmmversion.h"

#include "articleadmin.h"
#include "articleviewbase.h"
#include "drawareamain.h"

#include "skeleton/msgdiag.h"

#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscx.h"
#include "jdlib/misccharcode.h"

#include "dbtree/articlebase.h"
#include "dbtree/interface.h"

#include "dbimg/imginterface.h"

#include "skeleton/popupwin.h"

#include "config/globalconf.h"

#include "history/historymanager.h"

#include "message/logmanager.h"

#include "image/imageviewpopup.h"

#include "xml/document.h"
#include "xml/tools.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "global.h"
#include "type.h"
#include "httpcode.h"
#include "command.h"
#include "session.h"
#include "viewfactory.h"
#include "sharedbuffer.h"
#include "prefdiagfactory.h"
#include "usrcmdmanager.h"
#include "linkfiltermanager.h"
#include "compmanager.h"

#include "icons/iconmanager.h"

#include <glib/gi18n.h>

#include <sstream>
#include <cstring>

#ifndef MAX
#define MAX( a, b ) ( a > b ? a : b )
#endif

#ifndef MIN
#define MIN( a, b ) ( a < b ? a : b )
#endif

using namespace ARTICLE;

#define PROTO_URL4REPORT "url4report://"


ArticleViewBase::ArticleViewBase( const std::string& url, const std::string& url_article )
    : SKELETON::View( url )
    , m_url_article( url_article )
    , m_enable_menuslot{ true }
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::ArticleViewBase : " << get_url() << " : " << m_url_article << std::endl;
#endif

    set_id_toolbar( TOOLBAR_ARTICLE );

    // マウスジェスチャ可能
    set_enable_mg( true );

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_ARTICLE );

    // 板名セット
    update_boardname();
}



ArticleViewBase::~ArticleViewBase()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::~ArticleViewBase : " << get_url() << std::endl;
#endif

    hide_popup( true );
    delete_popup();
}


SKELETON::Admin* ArticleViewBase::get_admin()
{
    return ARTICLE::get_admin();
}


//
// コピー用URL( readcgi型 )
//
// メインウィンドウのURLバーなどの表示用にも使う
//
std::string ArticleViewBase::url_for_copy() const
{
    return DBTREE::url_readcgi( m_url_article, 0, 0 );
}


DrawAreaBase* ArticleViewBase::create_drawarea()
{
    return Gtk::manage( new ARTICLE::DrawAreaMain( m_url_article ) );
}


//
// セットアップ
//
// 各派生ビューで初期設定が済んだ後に呼ばれる
//
void ArticleViewBase::setup_view()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::setup_view " << get_url() << " url_article = " << m_url_article << std::endl;
#endif

    m_article = DBTREE::get_article( m_url_article );
    m_drawarea = create_drawarea();
    assert( m_article );
    assert( m_drawarea );

    m_drawarea->add_events( Gdk::SMOOTH_SCROLL_MASK );

    m_drawarea->sig_button_press().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_button_press ));
    m_drawarea->sig_button_release().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_button_release ));
    m_drawarea->sig_motion_notify().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_motion_notify ) );
    m_drawarea->sig_key_press().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_key_press ) );
    m_drawarea->sig_key_release().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_key_release ) );
    m_drawarea->sig_scroll_event().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_scroll_event ));
    m_drawarea->sig_leave_notify().connect(  sigc::mem_fun( *this, &ArticleViewBase::slot_leave_notify ) );

    m_drawarea->sig_on_url().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_on_url ) );
    m_drawarea->sig_leave_url().connect( sigc::mem_fun(*this, &ArticleViewBase::slot_leave_url ) );

    pack_start( *m_drawarea, true, true );
    setup_action();

    show_all_children();
}


//
// クライアント領域幅
//
int ArticleViewBase::width_client() const
{
    if( m_drawarea ) {
        const int width = m_drawarea->width_client();
#ifdef _DEBUG
        std::cout << "ArticleViewBase::width_client : " << width << std::endl;
#endif
        return width;
    }

    return SKELETON::View::width_client();
}


//
// クライアント領高さ
//
int ArticleViewBase::height_client() const
{
    if( m_drawarea ) {
        const int height = m_drawarea->height_client();
#ifdef _DEBUG
        std::cout << "ArticleViewBase::height_client : " << height << std::endl;
#endif
        return height;
    }



    return SKELETON::View::height_client();
}


// アイコンのID取得
int ArticleViewBase::get_icon( const std::string& iconname ) const
{
    int id = ICON::NONE;

    if( iconname == "default" ) id = ICON::THREAD;
    if( iconname == "loading" ) id = ICON::LOADING;
    if( iconname == "loading_stop" ) id = ICON::LOADING_STOP;
    if( iconname == "update" ) id = ICON::THREAD_UPDATE;  // 更新チェックしで更新があった場合
    if( iconname == "updated" ) id = ICON::THREAD_UPDATED;
    if( iconname == "old" ) id = ICON::THREAD_OLD;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::get_icon : " << iconname << " url = " << get_url() << std::endl;
#endif

    return id;
}


//
// コマンド
//
bool ArticleViewBase::set_command( const std::string& command, const std::string& arg1, const std::string& arg2 )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::set_command " << get_url() << std::endl
              << "command = " << command << std::endl;
#endif

    if( command == "append_dat" ) append_dat( arg1, -1 );
    else if( command == "append_html" ) append_html( arg1 );
    else if( command == "clear_screen" )
    {
        if( m_drawarea ) m_drawarea->clear_screen();
    }
    else if( command == "goto_num" ) goto_num( atoi( arg1.c_str() ), atoi( arg2.c_str() ) );
    else if( command == "hide_popup" ) hide_popup( true );
    else if( command == "delete_popup" ) delete_popup();
    else if( command == "clear_highlight" ) clear_highlight();

    else if( command == "reset_popupmenu" ) setup_action();

    // 実況手動切り替え
    else if( command == "live_start_stop" ){

        if( ! get_enable_live() ){

            if( m_enable_live && ! DBTREE::board_get_live_sec( m_url_article ) ){
                SKELETON::MsgDiag mdiag( get_parent_win(), "実況を行うには板のプロパティで更新間隔を設定して下さい。" );
                mdiag.run();
            }

            return true;
        }

        if( get_live() ){
            live_stop();

            // 実況したスレは終了時に削除する
            SESSION::remove_delete_list( m_url_article );
        }
        else{
            live_start();

            // 手動で停止した場合は終了時の削除をキャンセルする
            SESSION::append_delete_list( m_url_article );
        }
    }

    // dat落ちや更新が無い場合は実況停止
    // 終了時にスレを削除する
    else if( command == "live_stop" && get_enable_live() ) live_stop();

    return true;
}


//
// クロック入力
//
// クロックタイマーの本体はコアが持っていて、定期的にadminがアクティブなviewにクロック入力を渡す
//
// virtual
void ArticleViewBase::clock_in()
{
    assert( m_drawarea );

    View::clock_in();

    // ポップアップを時間差で消す
    if( m_hidepopup_counter ){

        --m_hidepopup_counter;
        if( ! m_hidepopup_counter ){

            // ポインタがポップアップ上に戻っていたらキャンセル
            if( ! is_mouse_on_popup() )  slot_hide_popup();
        }
    }

    // ポップアップが出てたらそっちにクロックを回す
    if( is_popup_shown() && m_popup_win->view() ){
        m_popup_win->view()->clock_in();
        return;
    }

    m_drawarea->clock_in();
}


//
// スムーススクロール用クロック入力
//
// タイマー本体はArticleAdminが持っている
//
void ArticleViewBase::clock_in_smooth_scroll()
{
    assert( m_drawarea );

    // ポップアップが出てたらそっちにクロックを回す
    if( is_popup_shown() && m_popup_win->view() ){
        ArticleViewBase* view = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
        if( view ) view->clock_in_smooth_scroll();
        return;
    }

    m_drawarea->clock_in_smooth_scroll();
}


//
// 再読み込みボタンを押した
//
void ArticleViewBase::reload()
{
    if( m_article->empty() ) return;

    if( CONFIG::get_reload_allthreads() ) ARTICLE::get_admin()->set_command( "reload_all_tabs" );
    else exec_reload();
}


//
// 再読み込み実行
//
// virtual
void ArticleViewBase::exec_reload()
{
    reload_article();
}


//
// 元のスレを開く (shift+s)
//
void ArticleViewBase::reload_article()
{
    if( m_article->empty() ) return;

    CORE::core_set_command( "open_article", m_url_article , "newtab", "" );
}


//
// ロード停止
//
void ArticleViewBase::stop()
{
    assert( m_article );
    m_article->stop_load();
}


//
// 再描画
//
void ArticleViewBase::redraw_view()
{
    // 起動中とシャットダウン中は処理しない
    if( SESSION::is_booting() ) return;
    if( SESSION::is_quitting() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::redraw_view " << get_url() << std::endl;
#endif

    assert( m_drawarea );
    m_drawarea->redraw_view_force();

    // ポップアップが表示されていたらポップアップも再描画
    if( is_popup_shown() ) m_popup_win->view()->redraw_view();
}


//
// フォーカスイン
//
void ArticleViewBase::focus_view()
{
    assert( m_drawarea );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::focus_view " << get_url() << std::endl;
#endif

    m_drawarea->focus_view();
    m_drawarea->redraw_view();
}


//
// フォーカスアウト
//
void ArticleViewBase::focus_out()
{
    SKELETON::View::focus_out();

#ifdef _DEBUG
    std::cout << "ArticleViewBase::focus_out " << get_url() << std::endl;
#endif

    m_drawarea->focus_out();

    if( is_popup_shown() ){

        // フォーカスアウトした瞬間に、子ポップアップが表示されていて、かつ
        // ポインタがその上だったらポップアップは消さない
        if( is_mouse_on_popup() ) return;

        if( CONFIG::get_hide_popup_msec() ) m_hidepopup_counter = CONFIG::get_hide_popup_msec() / TIMER_TIMEOUT;
        else hide_popup();
    }
}


//
// 閉じる
//
void ArticleViewBase::close_view()
{
    if( m_article->is_loading() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "読み込み中です" );
        mdiag.run();
        return;
    }

    ARTICLE::get_admin()->set_command( "close_view", get_url() );
}


//
// 削除ボタンを押した
//
void ArticleViewBase::delete_view()
{
    // IDに紐づけたツールバーボタンを取得してポップアップメニューの表示位置に指定します。
    show_popupmenu( "popup_menu_delete", SKELETON::PopupMenuPosition::toolbar_button,
                    get_admin()->get_anchor_widget( kToolbarWidgetDelete ) );
}


//
// 削除実行
//
void ArticleViewBase::exec_delete()
{
    CORE::core_set_command( "delete_article", m_url_article );
    ARTICLE::get_admin()->set_command( "switch_admin" );
}


//
// スレ再取得
//
void ArticleViewBase::delete_open_view()
{
    if( ! SESSION::is_online() ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
        mdiag.run();
        return;
    }

    if( DBTREE::article_status( m_url_article ) & STATUS_OLD ){
        SKELETON::MsgDiag mdiag( get_parent_win(), "DAT落ちしています。\n\nログが消える恐れがあります。実行しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
        mdiag.set_default_response( Gtk::RESPONSE_NO );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;
    }

    const std::string str_tab = "false";
    const std::string mode = "reget";
    CORE::core_set_command( "open_article", m_url_article, str_tab, mode );
}


// 実況可能か
bool ArticleViewBase::get_enable_live() const
{
    return ( m_enable_live && DBTREE::board_get_live_sec( m_url_article ) );
}


//
// マウスポインタをポップアップの上に移動する
//
void ArticleViewBase::warp_pointer_to_popup()
{
    if( is_popup_shown() ){

        const int mrg = 32;

        int x, y;
        MISC::get_pointer_at_window( m_popup_win->get_window(), x, y );

        if( x < mrg ) x = mrg;
        else if( x > m_popup_win->get_width() - mrg ) x = m_popup_win->get_width() - mrg;

        if( y < 0 ) y = mrg;
        else y = m_popup_win->get_height() - mrg;

        MISC::WarpPointer( get_window(), m_popup_win->get_window(), x, y );
    }
}


//
// viewの操作
//
bool ArticleViewBase::operate_view( const int control )
{
    assert( m_drawarea );

    // スレタイ検索
    // CONTROL::operate_common()の中で処理しないようにCONTROL::operate_common()の前で処理
    if( control == CONTROL::SearchTitle ){
        slot_search_title();
        return true;
    }

    if( CONTROL::operate_common( control, get_url(), ARTICLE::get_admin() ) ) return true;

    if( control == CONTROL::NoOperation ) return false;

    // スクロール系操作
    if( m_drawarea->set_scroll( control ) ) return true;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::operate_view control = " << control << std::endl;
#endif

    // その他の処理
    switch( control ){

        // リロード
        case CONTROL::Reload:
            exec_reload();
            break;

        // 元のスレを開く
        case CONTROL::ReloadArticle:
            reload_article();
            break;

            // コピー
        case CONTROL::Copy:
            slot_copy_selection_str();
            break;

            // 全て選択
        case CONTROL::SelectAll:
            slot_select_all();
            break;

            // お気に入りに追加
        case CONTROL::AppendFavorite:
            set_favorite();
            break;

            // 検索
        case CONTROL::Search:
            open_searchbar( false );
            break;

        case CONTROL::SearchInvert:
            open_searchbar( true );
            break;

        case CONTROL::SearchNext:
            down_search();
            break;

        case CONTROL::SearchPrev:
            up_search();
            break;

            // datを保存
        case CONTROL::Save:
            slot_save_dat();
            break;

            // 閉じる
        case CONTROL::Quit:
            close_view();
            break;

            // 書き込み
        case CONTROL::WriteMessage:
            write();
            break;

            // 削除
        case CONTROL::Delete:
        {
            if( m_article->empty() ) break;

            if( ! CONFIG::get_show_delartdiag() ){
                exec_delete();
                break;
            }

            SKELETON::MsgCheckDiag mdiag( get_parent_win(),
                                          "ログを削除しますか？\n\n「スレ再取得」を押すと\nあぼ〜んなどのスレ情報を削除せずにスレを再取得します。",
                                          "今後表示しない(常に削除)(_D)",
                                          Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE );
            mdiag.add_button( g_dgettext( GTK_DOMAIN, "_No" ), Gtk::RESPONSE_NO );
            mdiag.add_default_button( g_dgettext( GTK_DOMAIN, "_Yes" ), Gtk::RESPONSE_YES );

            mdiag.add_button( "スレ再取得(_R)", Gtk::RESPONSE_YES + 100 );
            mdiag.set_default_response( Gtk::RESPONSE_YES );
            int ret = mdiag.run();
            if( ret == Gtk::RESPONSE_YES ) exec_delete();
            else if( ret == Gtk::RESPONSE_YES + 100 ) delete_open_view();

            if( mdiag.get_chkbutton().get_active() ) CONFIG::set_show_delartdiag( false );

            break;
        }

            // Board に切り替え
        case CONTROL::Left:
            CORE::core_set_command( "switch_leftview" );
            break;

        case CONTROL::ToggleArticle:
            CORE::core_set_command( "toggle_article" );
            break;

            // image に切り替え
        case CONTROL::Right:
            CORE::core_set_command( "switch_rightview" );
            break;

        // 戻る、進む
        case CONTROL::PrevView:
            back_viewhistory( 1 );
            break;

        case CONTROL::NextView:
            forward_viewhistory( 1 );
            break;

        // ポップアップメニューをビューの左上に表示
        case CONTROL::ShowPopupMenu:
            show_popupmenu( "", SKELETON::PopupMenuPosition::view_top_left );
            break;

            // ブックマーク移動
        case CONTROL::PreBookMark:
            slot_pre_bm();
            break;

        case CONTROL::NextBookMark:
            slot_next_bm();
            break;

            // 書き込み移動
        case CONTROL::PrePost:
            slot_pre_post();
            break;

        case CONTROL::NextPost:
            slot_next_post();
            break;

            // 親にhideを依頼する and ローディング停止
        case CONTROL::StopLoading:
            stop();
            sig_hide_popup().emit();
            break;

            // 実況モード切り替え
        case CONTROL::LiveStartStop:
            if( ! m_article->empty() ) ARTICLE::get_admin()->set_command( "live_start_stop", get_url() );
            break;

            // 次スレ検索
        case CONTROL::SearchNextArticle:
            slot_search_next();
            break;

            // Web検索
        case CONTROL::SearchWeb:
            slot_search_web();
            break;

            // ログ検索(板内)
        case CONTROL::SearchCacheLocal:
            slot_search_cachelocal();
            break;

            // ログ検索(全て)
        case CONTROL::SearchCacheAll:
            slot_search_cacheall();
            break;

            // 選択範囲の画像を開く
        case CONTROL::ShowSelectImage:
            slot_show_selection_images();
            break;

            // 選択範囲の画像を削除
        case CONTROL::DeleteSelectImage:
            slot_delete_selection_images();
            break;

            // 選択範囲の画像をあぼ〜ん
        case CONTROL::AboneSelectImage:
            slot_abone_selection_images();
            break;

            // 選択範囲のレスをあぼ〜ん
        case CONTROL::AboneSelectionRes:
            slot_abone_selection_res();
            break;

            // スレのプロパティ
        case CONTROL::PreferenceView:
            show_preference();
            break;

        default:
            return false;
    }

    return true;
}


//
// 一番上へ
//
void ArticleViewBase::goto_top()
{
    assert( m_drawarea );
    m_drawarea->goto_top();
}



//
// 一番下へ
//
void ArticleViewBase::goto_bottom()
{
    assert( m_drawarea );
    m_drawarea->goto_bottom();
}



//
// num_from から num_to番へジャンプ
//
// num_from が 0 の時は m_drawarea->get_seen_current() からジャンプすると見なす
//
void ArticleViewBase::goto_num( const int num_to, const int num_from )
{
    assert( m_drawarea );

    const int from = ( ( num_from > 0 && num_to != num_from )  ? num_from : m_drawarea->get_seen_current() );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::goto_num to = " << num_to << " from = " << from << std::endl;
#endif

    m_drawarea->set_jump_history( from );
    m_drawarea->goto_num( num_to );
}


//
// 新着に移動
//
void ArticleViewBase::goto_new()
{
    assert( m_drawarea );
    m_drawarea->goto_new();
}



//
// 検索バーを開く
//
void ArticleViewBase::open_searchbar( bool invert )
{
    ARTICLE::get_admin()->set_command( "open_searchbar", get_url() );
    m_search_invert = invert;
}


//
// 前を検索
//
void ArticleViewBase::up_search()
{
    m_search_invert = true;
    exec_search();
    redraw_view();
}



//
// 次を検索
//
void ArticleViewBase::down_search()
{
    m_search_invert = false;
    exec_search();
    redraw_view();
}


//
// ハイライト解除
//
void ArticleViewBase::clear_highlight()
{
    assert( m_drawarea );
    if( get_pre_query().empty() ) return;

    set_pre_query( std::string() );
    m_drawarea->clear_highlight();
}


//
// メッセージ書き込みボタン
//
void ArticleViewBase::write()
{
    if( ! is_writeable() ) return;

    CORE::core_set_command( "open_message" ,m_url_article, std::string() );
}


//
// プロパティ表示
//
void ArticleViewBase::show_preference()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_preference\n";
#endif

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_ARTICLE, m_url_article );
    pref->run();
}


//
// 画像プロパティ表示
//
void ArticleViewBase::slot_preferences_image()
{
    if( m_url_tmp.empty() ) return;
    std::string url = m_url_tmp;

    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_IMAGE, url );
    pref->run();
}


//
// 戻る
//
void ArticleViewBase::back_viewhistory( const int count )
{
    ARTICLE::get_admin()->set_command( "back_viewhistory", get_url(), std::to_string( count ) );
}


//
// 進む
//
void ArticleViewBase::forward_viewhistory( const int count )
{
    ARTICLE::get_admin()->set_command( "forward_viewhistory", get_url(), std::to_string( count ) );
}



//
// 前のブックマークに移動
//
void ArticleViewBase::slot_pre_bm()
{
    assert( m_article );

    if( m_current_bm == 0 ) m_current_bm = m_drawarea->get_seen_current();

    for( int i = m_current_bm -1 ; i >= 1 ; --i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i, 0 );
            m_current_bm = i;
            return;
        }
    }

    for( int i = m_article->get_number_load() ; i > m_current_bm ; --i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i, 0 );
            m_current_bm = i;
            return;
        }
    }
}



//
// 後ろのブックマークに移動
//
void ArticleViewBase::slot_next_bm()
{
    assert( m_article );

    if( m_current_bm == 0 ) m_current_bm = m_drawarea->get_seen_current();

    for( int i = m_current_bm + 1; i <= m_article->get_number_load() ; ++i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i, 0 );
            m_current_bm = i;
            return;
        }
    }

    for( int i = 1; i <= m_current_bm ; ++i ){
        if( m_article->is_bookmarked( i ) ){
            goto_num( i, 0 );
            m_current_bm = i;
            return;
        }
    }
}


//
// 前の書き込みに移動
//
void ArticleViewBase::slot_pre_post()
{
    assert( m_article );

    if( m_current_post == 0 ) m_current_post = m_drawarea->get_seen_current();

    for( int i = m_current_post -1 ; i >= 1 ; --i ){
        if( m_article->is_posted( i ) ){
            goto_num( i, 0 );
            m_current_post = i;
            return;
        }
    }

    for( int i = m_article->get_number_load() ; i > m_current_post ; --i ){
        if( m_article->is_posted( i ) ){
            goto_num( i, 0 );
            m_current_post = i;
            return;
        }
    }
}



//
// 後ろの書き込みに移動
//
void ArticleViewBase::slot_next_post()
{
    assert( m_article );

    if( m_current_post == 0 ) m_current_post = m_drawarea->get_seen_current();

    for( int i = m_current_post + 1; i <= m_article->get_number_load() ; ++i ){
        if( m_article->is_posted( i ) ){
            goto_num( i, 0 );
            m_current_post = i;
            return;
        }
    }

    for( int i = 1; i <= m_current_post ; ++i ){
        if( m_article->is_posted( i ) ){
            goto_num( i, 0 );
            m_current_post = i;
            return;
        }
    }
}


//
// ジャンプ
//
// 呼び出す前に m_jump_to に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_jump()
{
    const std::string str_tab = "newtab";
    const std::string str_mode = "auto";
    CORE::core_set_command( "open_article", m_url_article , str_tab, str_mode, m_jump_to, m_jump_from );
}


//
// dat 保存
//
void ArticleViewBase::slot_save_dat()
{
    m_article->save_dat( std::string() );
}


//
// スレ情報の引き継ぎ
//
void ArticleViewBase::slot_copy_article_info()
{
    if( m_url_tmp.empty() ) return;
    if( ! DBTREE::article_is_cached( m_url_tmp ) ) return;

    m_article->copy_article_info( m_url_tmp );
    CORE::core_set_command( "replace_favorite_thread", "", m_url_tmp, m_url_article );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// あぼーん設定
//
void ArticleViewBase::slot_setup_abone()
{
    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_ARTICLE, m_url_article, "show_abone" );
    pref->run();
}

void ArticleViewBase::slot_setup_abone_board()
{
    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_BOARD,
                                       DBTREE::url_boardbase( m_url_article ), "show_abone_article" );
    pref->run();
}

void ArticleViewBase::slot_setup_abone_all()
{
    auto pref = CORE::PrefDiagFactory( get_parent_win(), CORE::PREFDIAG_GLOBALABONE, "" );
    pref->run();
}


//
// 荒らし報告用のURLリストをHTML形式で取得
//
std::string ArticleViewBase::get_html_url4report( const std::list< int >& list_resnum )
{
    std::string html;

    for( const int num : list_resnum ) {

        const std::string time_str = m_article->get_time_str( num );
        const std::string id_str = m_article->get_id_name( num );

        html += url_for_copy() + std::to_string( num );
        if( ! time_str.empty() ) html += " " + MISC::remove_str_regex( time_str, "\\([^\\)]+\\)" ); // 曜日を取り除く
        if( ! id_str.empty() ) html += " " + id_str.substr( strlen( PROTO_ID ) );
        html += "<br>";
    }

    return html;
}


//
// レスを抽出して表示
//
// num は "from-to"　の形式。"a+b"の時はaとbを連結する(bのヘッダ行を取り除く)
//
// (例) 3から10を表示したいなら "3-10"
//      3と4を連結して表示したい時は "3+4"
//
// show_title == trueの時は 板名、スレ名を表示
//
void ArticleViewBase::show_res( const std::string& num, const bool show_title )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_res num = " << num << " show_title = " << show_title << std::endl;
#endif

    // 板名、スレ名
    if( show_title ){

        std::string html;
        const std::string& tmpstr = DBTREE::board_name( m_url_article );
        if( ! tmpstr.empty() ) html += "[ " + MISC::html_escape( tmpstr ) + " ] ";

        html += DBTREE::article_modified_subject( m_url_article );

        if( ! html.empty() ) append_html( html );
    }

    std::list< bool > list_joint;
    std::list< int > list_resnum = m_article->get_res_str_num( num, list_joint );

    if( !list_resnum.empty() ) append_res( list_resnum, list_joint );
    else if( !show_title ) append_html( "未取得レス" );
}


//
// 名前 で抽出して表示
//
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::show_name( const std::string& name, bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_name " << name << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_name( name );

    std::ostringstream comment;
    comment << "名前：" << name << "  " << list_resnum.size() << " 件<br>";
    comment << "総参照数:" << m_article->get_res_reference( list_resnum ).size() << " 件";

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << "<br><br><a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy();
        if( url_for_copy()[ url_for_copy().size() - 1 ] != '/' ) comment << "/";
        comment << MISC::intlisttostr( list_resnum ) << "<br><hr>";
    }

    append_html( comment.str() );

    if( ! list_resnum.empty() ) append_res( list_resnum );
}


//
// ID で抽出して表示
//
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::show_id( const std::string& id_name, const bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_id " << id_name << std::endl;
#endif

    const std::list< int > list_resnum = m_article->get_res_id_name( id_name );

    const std::string raw_id = id_name.substr( strlen( PROTO_ID ) );

    std::ostringstream comment;
    comment << raw_id << "  " << list_resnum.size() << " 件<br>";
    comment << "総参照数:" << m_article->get_res_reference( list_resnum ).size() << " 件";

    // 末尾判定
    if( raw_id.size() >= 12 ) { // 特定の日時に表示される末尾があると12以上になる
        // 新方式でのID末尾
        const char c = raw_id[11];
        switch( c ) {

            case '0': comment << "<br>末尾:" << c << " (固定回線)";  break;

            case 'd': comment << "<br>末尾:" << c << " (docomo)";  break;
            case 'D': comment << "<br>末尾:" << c << " (docomo mopera)";  break;
            case 'a': comment << "<br>末尾:" << c << " (au)";  break;
            case 'p': comment << "<br>末尾:" << c << " (SoftBank iPhone)";  break;
            case 'r': comment << "<br>末尾:" << c << " (SoftBank Android)";  break;
            case 'x': comment << "<br>末尾:" << c << " (SoftBank)";  break;
            case 'e':
            case 'E': comment << "<br>末尾:" << c << " (Y!mobile)";  break;

            case 'F':
            case 'A': comment << "<br>末尾:" << c << " (公衆Fi-Wi)";  break;
            case 'S': comment << "<br>末尾:" << c << " (SafeComNet)";  break;
            case 'M': comment << "<br>末尾:" << c << " (MVNO)";  break;
            case 'W': comment << "<br>末尾:" << c << " (WiMAX1)";  break;
            case 'X': comment << "<br>末尾:" << c << " (Android GoogleChrome プロクシー)";  break;

            case 'K':
            case 'Q': comment << "<br>末尾:" << c << " (ガラケー)";  break;

            case '6': comment << "<br>末尾:" << c << " (大学)";  break;
            case '7': comment << "<br>末尾:" << c << " (大学以外の学校)";  break;
            case '8': comment << "<br>末尾:" << c << " (VPN)";  break;
            case '9': comment << "<br>末尾:" << c << " (運営公認ボランティア)";  break;

            case 'h': comment << "<br>末尾:" << c << " (逆引きできない)";  break;
            case 'H': comment << "<br>末尾:" << c << " (逆引きが変)";  break;

            case 'n': comment << "<br>末尾:" << c << " (ショッピングセンター)";  break;
            case 'C': comment << "<br>末尾:" << c << " (会社内)";  break;
            case 'G': comment << "<br>末尾:" << c << " (役所内)";  break;
            case 'V': comment << "<br>末尾:" << c << " (外国の役所内)";  break;
            case 'L': comment << "<br>末尾:" << c << " (米軍関係)";  break;
        }
    }
    else if( raw_id.size() == 9 ) {
        // 従来方式でのID末尾
        const char c = raw_id[8];
        switch( c ) {

            case 'O': comment << "<br>末尾:" << c << " 携帯";  break;
            case 'o': comment << "<br>末尾:" << c << " 携帯(WILLCOM)";  break;
            case 'Q': comment << "<br>末尾:" << c << " ibisBrowserDX";  break;
            case 'I':
            case 'i': comment << "<br>末尾:" << c << " iPhone";  break;
            case 'P': comment << "<br>末尾:" << c << " p2";  break;
            case '0': comment << "<br>末尾:" << c << " PC";  break;
        }
    }

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << "<br><br><a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy();
        if( url_for_copy()[ url_for_copy().size() - 1 ] != '/' ) comment << "/";
        comment << MISC::intlisttostr( list_resnum ) << "<br><hr>";
    }

    append_html( comment.str() );

    if( ! list_resnum.empty() ) append_res( list_resnum );
}



//
// ブックマークを抽出して表示
//
void ArticleViewBase::show_bm()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_bm " << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_bm();

    if( ! list_resnum.empty() ) append_res( list_resnum );
    else append_html( "しおりはセットされていません" );
}


//
// 書き込みを抽出して表示
//
void ArticleViewBase::show_post()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_post " << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_posted();

    if( ! list_resnum.empty() ){

        std::ostringstream comment;
        comment << "書き込み数：" << list_resnum.size() << " 件<br>";
        comment << "総参照数:" << m_article->get_res_reference( list_resnum ).size() << " 件";
        append_html( comment.str() );

        append_res( list_resnum );
    }
    else append_html( "このスレでは書き込みしていません" );
}


//
// 書き込みログを表示
//
void ArticleViewBase::show_postlog( const int num )
{
    const int maxno = MESSAGE::get_log_manager()->get_max_num_of_log() + 1;

    std::string html_header;
    for( int i = 1; i <= maxno; ++i ){

        int no;
        if( i == 1 ) no = 0;
        else no = maxno - ( i -1 );

        if( no == num ) html_header += std::to_string( i ) + " ";
        else {
            html_header += std::string( "<a href=\"" ) + PROTO_POSTLOG + std::to_string( no ) + "\">"
                + std::to_string( i ) + "</a> ";
        }
    }

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_postlog " << num << " / " << maxno << std::endl
              << html_header << std::endl;
#endif

    std::string html = MESSAGE::get_log_manager()->get_post_log( num );
    if( html.empty() ) html = "書き込みログがありません";
    else html = html_header + "<br>" + html + "<hr><br>" + html_header;

    append_html( html );
}


//
// 高参照レスを抽出して表示
//
void ArticleViewBase::show_highly_referenced_res()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_highly_referenced_res " << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_highly_referenced_res();

    if( ! list_resnum.empty() ){

        std::ostringstream comment;
        comment << "高参照レス数：" << list_resnum.size() << " 件<br>";
        append_html( comment.str() );

        append_res( list_resnum );
    }
    else append_html( "このスレでは高参照レスはありません" );
}



//
// URLを含むレスを抽出して表示
//
void ArticleViewBase::show_res_with_url()
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_res_with_url\n";
#endif

    std::list< int > list_resnum = m_article->get_res_with_url();

    if( ! list_resnum.empty() ) append_res( list_resnum );
    else append_html( "リンクを含むレスはありません" );
}



//
// num 番のレスを参照してるレスを抽出して表示
//
void ArticleViewBase::show_refer( int num )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::show_refer " << num << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_reference( num );

    std::ostringstream comment;
    comment << "参照数：" << list_resnum.size() << " 件";

    append_html( comment.str() );

    // num 番は先頭に必ず表示
    list_resnum.push_front( num );

    append_res( list_resnum );
}




//
// キーワードで抽出して表示
//
// mode_or = true の時は or 検索
// show_option = true の時は URL 表示などのオプションが表示される
//
void ArticleViewBase::drawout_keywords( const std::string& query, bool mode_or, bool show_option )
{
    assert( m_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::drawout_keywords " << query << std::endl;
#endif

    std::list< int > list_resnum = m_article->get_res_query( query, mode_or );

    std::ostringstream comment;
    comment << query << "  " << list_resnum.size() << " 件";

    if( show_option && ! list_resnum.empty() ){
        if( !m_show_url4report ) comment << " <a href=\"" << PROTO_URL4REPORT << "\">抽出したレスのURLをリスト表示</a>";
        else comment << "<br><br>" + get_html_url4report( list_resnum );

        comment << "<br><br>" << url_for_copy();
        if( url_for_copy()[ url_for_copy().size() - 1 ] != '/' ) comment << "/";
        comment << MISC::intlisttostr( list_resnum ) << "<br><hr>";
    }

    append_html( comment.str() );

    if( ! list_resnum.empty() ){

        append_res( list_resnum );

        // ハイライト表示
        std::list< std::string > list_query = MISC::split_line( query );
        m_drawarea->search( list_query, false );
    }
}



//
// html をappend
//
void ArticleViewBase::append_html( const std::string& html )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::append_html html = " << html << std::endl;
#endif

    assert( m_drawarea );
    m_drawarea->append_html( html );
    redraw_view();
}



//
// dat をレス番号 num 番として append
//
// num < 0 の時は現在の最大スレ番号の後に追加
//
void ArticleViewBase::append_dat( const std::string& dat, int num )
{
    assert( m_drawarea );
    if( num < 0 ) num = get_article()->get_number_load() +1;
    m_drawarea->append_dat( dat, num );
    redraw_view();
}


//
// リストで指定したレスの表示
//
void ArticleViewBase::append_res( const std::list< int >& list_resnum )
{
    assert( m_drawarea );
    m_drawarea->append_res( list_resnum );
    redraw_view();
}


//
// リストで指定したレスを表示(連結情報付き)
//
// list_joint で連結指定したレスはヘッダを取り除いて前のレスに連結する
//
void ArticleViewBase::append_res( const std::list< int >& list_resnum, const std::list< bool >& list_joint )
{
    assert( m_drawarea );
    m_drawarea->append_res( list_resnum, list_joint );
    redraw_view();
}



//
// drawareaから出た
//
bool ArticleViewBase::slot_leave_notify( GdkEventCrossing* event )
{
    // クリックしたときやホイールを回すと event->mode に　GDK_CROSSING_GRAB
    // か GDK_CROSSING_UNGRAB がセットされてイベントが発生する場合がある
    if( event->mode == GDK_CROSSING_GRAB ) return false;
    if( event->mode == GDK_CROSSING_UNGRAB ) return false;

    // クリックしたりホイールを回すとイベントが発生する時があるのでキャンセルする
    if( event->state
        & ( GDK_BUTTON1_MASK | GDK_BUTTON2_MASK | GDK_BUTTON3_MASK |
            GDK_BUTTON4_MASK | GDK_BUTTON5_MASK ) ) return false;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_leave_drawarea :";
    std::cout << " type = " << event->type;
    std::cout << " mode = " << event->mode;
    std::cout << " detail = " << event->detail;
    std::cout << " state = " << event->state << std::endl;
#endif

    focus_out();

    // ステータスバーの表示を戻す
    if( m_url_show_status )
    {
        CORE::core_set_command( "restore_status", m_url_article );
        m_url_show_status = false;
    }

    return false;
}



//
// drawarea のクリックイベント
//
bool ArticleViewBase::slot_button_press( const std::string& url, int res_number, GdkEventButton* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_button_press url = " << get_url() << std::endl;
#endif

    // マウスジェスチャ
    get_control().MG_start( event );

    // ホイールマウスジェスチャ
    get_control().MG_wheel_start( event );

    ARTICLE::get_admin()->set_command( "switch_admin" );

    return true;
}



//
// drawarea でのマウスボタンのリリースイベント
//
bool ArticleViewBase::slot_button_release( std::string url, int res_number, GdkEventButton* event )
{
    /// マウスジェスチャ
    const int mg = get_control().MG_end( event );

    // ホイールマウスジェスチャ
    // 実行された場合は何もしない
    if( get_control().MG_wheel_end( event ) ) return true;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_button_release mg = " << mg << " url = " << get_url()
              << " url = " << url << " res = " << res_number
              << std::endl;
#endif

    if( event->type == GDK_BUTTON_RELEASE ){

        if( ! is_mouse_on_popup() ){

            // マウスジェスチャ
            if( mg != CONTROL::NoOperation && enable_mg() ){
                hide_popup();
                operate_view( mg );
            }

            // リンクをクリック
            else if( click_url( url, res_number, event ) );

            // コンテキストメニューをマウスポインターの位置に表示
            else if( get_control().button_alloted( event, CONTROL::PopupmenuButton ) ) {
                show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
            }

            // 実況中の場合は停止
            else if( get_control().button_alloted( event, CONTROL::ClickButton ) && get_live() )
                ARTICLE::get_admin()->set_command( "live_start_stop", get_url() );

            // その他
            else operate_view( get_control().button_press( event ) );
        }
    }

    return true;
}



//
// drawarea でマウスが動いた
//
bool ArticleViewBase::slot_motion_notify( GdkEventMotion* event )
{
    /// マウスジェスチャ
    get_control().MG_motion( event );

    return true;
}



//
// drawareaのキープレスイベント
//
bool ArticleViewBase::slot_key_press( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_key_press\n";
#endif

    // ポップアップはキーフォーカスを取れないので親からキー入力を送ってやる
    if( is_popup_shown() ){

        ArticleViewBase* popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
        if( popup_article ){

            if( DBTREE::article_is_cached( popup_article->url_article() ) ) return popup_article->slot_key_press( event );
        }
        else{

            IMAGE::ImageViewPopup* popup_image = dynamic_cast< IMAGE::ImageViewPopup* >( m_popup_win->view() );
            if( popup_image ) return popup_image->slot_key_press( event );
        }
    }

    int key = get_control().key_press( event );

    if( key != CONTROL::NoOperation ){
        if( operate_view( key ) ) return true;
    }
    else if( release_keyjump_key( event->keyval ) ) return true;

    return false;
}



//
// drawareaのキーリリースイベント
//
bool ArticleViewBase::slot_key_release( GdkEventKey* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_key_release\n";
#endif

    // ポップアップはキーフォーカスを取れないのでキー入力を送ってやる
    ArticleViewBase* popup_article = nullptr;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article ) return popup_article->slot_key_release( event );

    return true;
}



//
// drawareaのマウスホイールイベント
//
bool ArticleViewBase::slot_scroll_event( GdkEventScroll* event )
{
    // ポップアップしているときはそちらにイベントを送ってやる
    ArticleViewBase* popup_article = nullptr;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article ) return popup_article->slot_scroll_event( event );

    // ホイールマウスジェスチャ
    int control = get_control().MG_wheel_scroll( event );
    if( enable_mg() && control != CONTROL::NoOperation ){
        operate_view( control );
        return true;
    }

    m_drawarea->wheelscroll( event );
    return true;
}



//
// リンクの上にポインタが来た
//
// drawareaのsig_on_url()シグナルとつなぐ
//
void ArticleViewBase::slot_on_url( const std::string& url, const std::string& imgurl, int res_number )
{

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_on_url = " << url
              << " imgurl = " << imgurl
              << std::endl;
#endif

    // ポップアップが消えるまでに同じリンク上にポインタを乗せたら
    // カウンタをリセットする
    if( m_hidepopup_counter && m_popup_url == url ){
        m_hidepopup_counter = 0;
        return;
    }

    CORE::VIEWFACTORY_ARGS args;
    SKELETON::View* view_popup = nullptr;
    int margin_popup_x = 0;
    int margin_popup_y = CONFIG::get_margin_popup();

    m_popup_url = url;

    // 画像ポップアップ
    if( DBIMG::get_type_ext( imgurl ) != DBIMG::T_UNKNOWN ){

        // あぼーん
        if( DBIMG::get_abone( imgurl ) ){
            args.arg1 = "あぼ〜んされています";
            std::string abone_reason = DBIMG::get_img_abone_reason( imgurl );
            if( ! abone_reason.empty() ) {
                args.arg1.append( "<br>" );
                args.arg1.append( MISC::replace_str( abone_reason, "://", "&#58;//" ) );
            }
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPHTML, m_url_article, args );
        }

        else if(
            ( ! DBIMG::is_cached( imgurl ) || CONFIG::get_use_image_popup() )
            && ( DBIMG::is_loading( imgurl ) || DBIMG::is_wait( imgurl ) || DBIMG::get_code( imgurl ) != HTTP_INIT ) ){

#ifdef _DEBUG
            std::cout << "image " << DBIMG::get_code( imgurl) << " " << DBIMG::is_loading( imgurl ) << "\n";
#endif

            view_popup = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP,  imgurl );
            margin_popup_x = CONFIG::get_margin_imgpopup_x();
            margin_popup_y = CONFIG::get_margin_imgpopup();
        }
    }

    // レスポップアップ
    else if( url.rfind( PROTO_ANCHORE, 0 ) == 0 ){

        args.arg1 = url.substr( strlen( PROTO_ANCHORE) );
        args.arg2 = "false"; // 板名、スレ名非表示
        args.arg3 = "false"; // あぼーんしたレスの内容は非表示(あぼーんと表示)

#ifdef _DEBUG
        std::cout << "anchore = " << args.arg1 << std::endl;
#endif

        view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, m_url_article, args );
    }

    // レス番号
    else if( url.rfind( PROTO_RES, 0 ) == 0 ){

        args.arg1 = url.substr( strlen( PROTO_RES ) );
        int tmp_num = atoi( args.arg1.c_str() );

#ifdef _DEBUG
        std::cout << "res = " << args.arg1 << std::endl;
#endif

        // あぼーんされたレスの内容をポップアップ表示
        if( m_article->get_abone( tmp_num ) ){

            args.arg2 = "false"; // 板名、スレ名非表示
            args.arg3 = "true"; // あぼーんレスの内容を表示


            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, m_url_article, args );
        }

        // 参照ポップアップ
        else if( CONFIG::get_refpopup_by_mo() && m_article->get_res_reference( tmp_num ).size() ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPREFER, m_url_article, args );
        }
    }

    // 抽出(or)
    else if( url.rfind( PROTO_OR, 0 ) == 0 ){

        std::string url_tmp = url.substr( strlen( PROTO_OR ) );

        int i = url_tmp.find( KEYWORD_SIGN );

        std::string url_dat = DBTREE::url_dat( url_tmp.substr( 0, i ) );
        args.arg1 = url_tmp.substr( i + strlen( KEYWORD_SIGN ) );
        args.arg2 = "OR";

        if( ! url_dat.empty() ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPDRAWOUT, url_dat, args );
        }
    }

    // しおり
    else if( url.rfind( PROTO_BM, 0 ) == 0 ){

        const std::string url_tmp = url.substr( strlen( PROTO_BM ) );
        const std::string url_dat = DBTREE::url_dat( url_tmp );
        if( ! url_dat.empty() ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPBM, url_dat );
        }
    }

    // 名前ポップアップ
    else if( CONFIG::get_namepopup_by_mo() && url.rfind( PROTO_NAME, 0 ) == 0 ){

        int num_name = m_article->get_num_name( res_number );
        args.arg1 = m_article->get_name( res_number );

        if( num_name >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPNAME, m_url_article, args );
        }
    }

    // IDポップアップ
    else if( CONFIG::get_idpopup_by_mo() && url.rfind( PROTO_ID, 0 ) == 0 ){

        args.arg1 = m_article->get_id_name( res_number );
        int num_id = m_article->get_num_id_name( res_number );

        if( num_id >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
        }
    }

    // ID:〜の範囲選択の上にポインタがあるときIDポップアップ
    else if( url.rfind( "ID:", 0 ) == 0 ){

        args.arg1 = PROTO_ID + url;
        int num_id = m_article->get_num_id_name( args.arg1 );

#ifdef _DEBUG
        std::cout << "num_id = " << num_id << std::endl;
#endif

        if( num_id >= 1 ){
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
        }
    }

    // その他のリンク
    else{

        // dat 又は板の場合
        int num_from, num_to;
        std::string num_str;

        const std::string url_dat = DBTREE::url_dat( url, num_from, num_to, num_str );
        const std::string boardbase = DBTREE::url_boardbase( url );

#ifdef _DEBUG
        std::cout << "ArticleViewBase::slot_on_url " << url << std::endl;
        std::cout << "url_dat = " << url_dat << std::endl;
        std::cout << "boardbase = " << boardbase << std::endl;
        std::cout << "num_from = " << num_from << std::endl;
        std::cout << "num_to = " << num_to << std::endl;
        std::cout << "num = " << num_str << std::endl;
#endif

        // 他スレ
        if( ! url_dat.empty() ){
            if( num_from == 0 ) args.arg1 = "1"; // 最低でも1レス目は表示
            else if( num_str.empty() ) args.arg1 = "1";
            else args.arg1 = num_str;

            args.arg2 = "true"; // 板名、スレ名表示
            args.arg3 = "false"; // あぼーんしたレスの内容は非表示(あぼーんと表示)
            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPRES, url_dat, args );
        }

        // 板
        else if( ! boardbase.empty() ){

            const std::string& tmpstr = DBTREE::board_name( url );
            args.arg1 = "[ " + tmpstr + " ] ";

            view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPHTML, m_url_article, args );
        }
    }

    if( view_popup ) show_popup( view_popup, margin_popup_x, margin_popup_y );

    // リンクとして扱うURLをステータスバーに表示する
    if( MISC::is_url_scheme( url.c_str() ) != MISC::SCHEME_NONE )
    {
        std::string status_url;

        // デコードが必要な物
        if( url.find( '%' ) != std::string::npos )
        {
            std::string tmp = MISC::url_decode( url );

            const auto enc = MISC::detect_encoding( tmp );

            switch( enc )
            {
                case Encoding::sjis:
                case Encoding::eucjp:
                case Encoding::jis:

                    status_url = MISC::Iconv( tmp, Encoding::utf8, enc );
                    break;

                case Encoding::ascii:
                case Encoding::utf8:

                    status_url = tmp;
                    break;

                case Encoding::unknown: // suppress -Wswitch-enum
                default:

                    status_url = url;
            }

            // 改行はスペースに変えておく
            status_url = MISC::replace_newlines_to_str( status_url, " " );
        }
        // デコードの必要がない物
        else
        {
            status_url = url;
        }

        // 一時的にステータスバーにURLを表示( 長くても全部表示しちゃう？ )
        CORE::core_set_command( "set_status_temporary", m_url_article, status_url );

        m_url_show_status = true;
    }
}



//
// リンクからマウスが出た
//
void ArticleViewBase::slot_leave_url()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_leave_url\n";
#endif

    if( m_url_show_status )
    {
        // ステータスバーの表示を戻す
        CORE::core_set_command( "restore_status", m_url_article );
        m_url_show_status = false;
    }

    if( is_popup_shown() ){
        if( CONFIG::get_hide_popup_msec() ) m_hidepopup_counter = CONFIG::get_hide_popup_msec() / TIMER_TIMEOUT;
        else hide_popup();
    }
}



//
// リンクをクリック
//
bool ArticleViewBase::click_url( std::string url, int res_number, GdkEventButton* event )
{
    assert( m_article );
    if( url.empty() ) return false;

    CONTROL::Control& control = get_control();

#ifdef _DEBUG
    std::cout << "ArticleViewBase::click_url " << url << std::endl;
#endif

    // プレビュー画面など、レスが存在しないときはいくつかの機能を無効にする
    const bool res_exist = ( ! m_article->empty() && m_article->res_header( res_number ) );

    // ssspの場合は PROTO_SSSP が前に付いているので取り除く
    const bool sssp = ( url.rfind( PROTO_SSSP, 0 ) == 0 );
    if( sssp ) url = url.substr( strlen( PROTO_SSSP ) );

    /////////////////////////////////////////////////////////////////
    // ID クリック
    if( url.rfind( PROTO_ID, 0 ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            const int num_id = m_article->get_num_id_name( res_number );
            m_id_name = m_article->get_id_name( res_number );

            // ID ポップアップ
            if( num_id >= 1 && control.button_alloted( event, CONTROL::PopupIDButton ) ){
                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_id_name;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPID, m_url_article, args );
                const int margin_popup_x = 0;
                const int margin_popup_y = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup_x, margin_popup_y );
            }
            else if( control.button_alloted( event, CONTROL::DrawoutIDButton ) ) slot_drawout_id();
            else if( control.button_alloted( event, CONTROL::PopupmenuIDButton ) ){
                show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // 名前クリック
    else if( url.rfind( PROTO_NAME, 0 ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            int num_name = m_article->get_num_name( res_number );
            m_name = m_article->get_name( res_number );

            // 名前ポップアップ
            if( num_name >= 1 && control.button_alloted( event, CONTROL::PopupIDButton ) ){
                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_name;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPNAME, m_url_article, args );
                const int margin_popup_x = 0;
                const int margin_popup_y = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup_x, margin_popup_y );
            }
            else if( control.button_alloted( event, CONTROL::DrawoutIDButton ) ) slot_drawout_name();
            else if( control.button_alloted( event, CONTROL::PopupmenuIDButton ) ){
                show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
            }
        }
    }

    /////////////////////////////////////////////////////////////////
    // あぼーんクリック
    else if( url.rfind( PROTO_ABONE, 0 ) == 0 ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ){
            show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
        }
    }

    /////////////////////////////////////////////////////////////////
    // 壊れていますクリック
    else if( url.rfind( PROTO_BROKEN, 0 ) == 0 ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ){
            show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
        }
    }

    /////////////////////////////////////////////////////////////////
    // 荒らし報告用URL表示クリック
    else if( url.rfind( PROTO_URL4REPORT, 0 ) == 0 ){

        hide_popup();
        m_show_url4report = true;
        relayout();
    }

    /////////////////////////////////////////////////////////////////
    // 書き込みログ表示クリック
    else if( url.rfind( PROTO_POSTLOG, 0 ) == 0 ){

        hide_popup();

        CORE::core_set_command( "open_article_postlog" ,"", url.substr( strlen( PROTO_POSTLOG ) ) );
    }

    /////////////////////////////////////////////////////////////////
    // BE クリック
    else if( url.rfind( PROTO_BE, 0 ) == 0 ){

        if( ! res_exist ) return true;

        hide_popup();

        const std::string openurl = "https://be.5ch.io/user/" + url.substr( std::strlen( PROTO_BE ) );
#ifdef _DEBUG
        std::cout << "open  " << openurl << std::endl;
#endif
        if( control.button_alloted( event, CONTROL::OpenBeButton ) ) CORE::core_set_command( "open_url_browser", openurl );
        else if( control.button_alloted( event, CONTROL::PopupmenuBeButton ) ){
            show_popupmenu( openurl, SKELETON::PopupMenuPosition::mouse_pointer );
        }
    }

    /////////////////////////////////////////////////////////////////
    // アンカーをクリック
    else if( url.rfind( PROTO_ANCHORE, 0 ) == 0 ){

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

        // ジャンプ先セット
        m_jump_to = url.substr( strlen( PROTO_ANCHORE ) );
        m_jump_from = std::to_string( res_number );
        m_str_num = m_jump_to;

#ifdef _DEBUG
        std::cout << "anchor num = " << m_str_num << std::endl;
#endif

            hide_popup();

            if( control.button_alloted( event, CONTROL::JumpAncButton ) ) slot_jump();
            else if( control.button_alloted( event, CONTROL::PopupmenuAncButton ) ) {
                show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
            }
            else if( control.button_alloted( event, CONTROL::DrawoutAncButton ) ) slot_drawout_around();
        }
    }

    /////////////////////////////////////////////////////////////////
    // レス番号クリック
    else if( url.rfind( PROTO_RES, 0 ) == 0 ){

        if( ! res_exist ) return true;

        if( is_popup_shown() && control.button_alloted( event, CONTROL::PopupWarpButton ) ) warp_pointer_to_popup();

        else{

            hide_popup();

            m_str_num = std::to_string( res_number );
            m_url_tmp = DBTREE::url_readcgi( m_url_article, res_number, 0 );

            // ジャンプ先セット
            m_jump_to = m_str_num;
            m_jump_from = m_str_num;

            if( control.button_alloted( event, CONTROL::PopupmenuResButton ) ) {
                show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
            }

            // ブックマークセット
            else if( control.button_alloted( event, CONTROL::BmResButton ) ) slot_bookmark();

            // 書き込みマークの設定/解除
            else if( control.button_alloted( event, CONTROL::PostedMarkButton ) ) slot_postedmark();

            // 参照ポップアップ表示
            else if( control.button_alloted( event, CONTROL::ReferResButton ) ){

                CORE::VIEWFACTORY_ARGS args;
                args.arg1 = m_str_num;
                SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_ARTICLEPOPUPREFER, m_url_article, args );
                const int margin_popup_x = 0;
                const int margin_popup_y = CONFIG::get_margin_popup();
                show_popup( view_popup, margin_popup_x, margin_popup_y );
            }
        }
    }


    /////////////////////////////////////////////////////////////////
    // OR抽出
    else if( url.rfind( PROTO_OR, 0 ) == 0 ){

        const std::string url_tmp = url.substr( strlen( PROTO_OR ) );
        int i = url_tmp.find( KEYWORD_SIGN );
        const std::string url_dat = DBTREE::url_dat( url_tmp.substr( 0, i ) );

        if( ! url_dat.empty() ){
            const std::string query = url_tmp.substr( i + strlen( KEYWORD_SIGN ) );
            CORE::core_set_command( "open_article_keyword" ,url_dat, query, "true" );
        }
    }

    /////////////////////////////////////////////////////////////////
    // しおり抽出
    else if( url.rfind( PROTO_BM, 0 ) == 0 ){

        const std::string url_tmp = url.substr( strlen( PROTO_BM ) );
        const std::string url_dat = DBTREE::url_dat( url_tmp );
        if( ! url_dat.empty() ){
            CORE::core_set_command( "open_article_bm" , url_dat );
       }
    }

    /////////////////////////////////////////////////////////////////
    // リンクフィルタ
    else if( control.button_alloted( event, CONTROL::ClickButton )
             && CORE::get_linkfilter_manager()->exec( m_url_article, url, m_drawarea->str_pre_selection() ) ){}


    /////////////////////////////////////////////////////////////////
    // 画像クリック
    else if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN
             && ( CONFIG::get_use_image_view()
                  || CONFIG::get_use_inline_image()
                  || ( sssp && CONFIG::get_show_ssspicon() ) ) ){

        hide_popup();

        if( control.button_alloted( event, CONTROL::PopupmenuImageButton ) ){
            m_str_num = std::to_string( res_number );
            show_popupmenu( url, SKELETON::PopupMenuPosition::mouse_pointer );
        }

        else if( ! DBIMG::is_cached( url ) && ! SESSION::is_online() ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "オフラインです" );
            mdiag.run();
        }

        else if( DBIMG::get_abone( url )){
            SKELETON::MsgDiag mdiag( get_parent_win(), "あぼ〜んされています", false,
                                     Gtk::MESSAGE_INFO, Gtk::BUTTONS_YES_NO );
            std::string abone_reason = DBIMG::get_img_abone_reason( url );
            if( ! abone_reason.empty() ) {
                abone_reason = MISC::replace_str( abone_reason, "<br>", "\n" );
                abone_reason.append( "\n\n" );
            }
            abone_reason.append( DBIMG::kTemporaryMosaicQuestion );
            mdiag.set_secondary_text( abone_reason );
            const int response = mdiag.run();
            if( response == Gtk::RESPONSE_YES ) {

                const bool open_imageview{ ! sssp // sssp の時は画像ビューを開かない
                                           && CONFIG::get_use_image_view()
                                           && ( control.button_alloted( event, CONTROL::ClickButton )
                                                || control.button_alloted( event, CONTROL::OpenBackImageButton ) ) };

                const bool open_browser{ ! open_imageview
                                         && control.button_alloted( event, CONTROL::ClickButton ) };

                const int mosaic_mode{ sssp ? 0 // sssp の時はモザイクをかけない
                                            : DBIMG::kForceMosaic };

                // 画像ビューに切り替える
                const bool switch_image{ open_imageview
                                         && ! control.button_alloted( event, CONTROL::OpenBackImageButton ) };

                open_image( url, res_number, open_imageview, open_browser, mosaic_mode, switch_image );
            }
        }

        else if( DBIMG::get_type_real( url ) == DBIMG::T_LARGE ){
            SKELETON::MsgDiag mdiag( get_parent_win(), "画像サイズが大きすぎます。\n\n表示するにはリンクの上でコンテキストメニューを開いて\n「サイズが大きい画像を表示」をクリックしてください。" );
            mdiag.run();
        }

        else{

            const bool open_imageview = ( ! sssp // sssp の時は画像ビューを開かない
                                          && CONFIG::get_use_image_view()
                                          && (
                                              control.button_alloted( event, CONTROL::ClickButton )
                                              || control.button_alloted( event, CONTROL::OpenBackImageButton )
                                              )
                );

            const bool open_browser = ( ! open_imageview
                                          && control.button_alloted( event, CONTROL::ClickButton )
                );

            const bool mosaic = ( CONFIG::get_use_mosaic()
                                  && ! sssp // sssp の時はモザイクをかけない
                );

            // 画像ビューに切り替える
            const bool switch_image = ( open_imageview && ! control.button_alloted( event, CONTROL::OpenBackImageButton ) );

            open_image( url, res_number, open_imageview, open_browser, mosaic, switch_image );
        }
    }

    /////////////////////////////////////////////////////////////////
    // ブラウザで開く
    else if( control.button_alloted( event, CONTROL::ClickButton ) ){

        std::string tmp_url = url;

        // 相対パス
        if( url.rfind( "./", 0 ) == 0 ) tmp_url = DBTREE::url_boardbase( m_url_article ) + url.substr( 1 );

        // 絶対パス
        else if( url.rfind( '/', 0 ) == 0 ) tmp_url = DBTREE::url_root( m_url_article ) + url.substr( 1 );

        hide_popup();

        // tmp_urlが他スレのアドレスなら、datロード終了時に次スレ移行チェックを行う
        DBTREE::article_set_url_pre_article( tmp_url, m_url_article );

        CORE::core_set_command( "open_url", tmp_url );
    }

    /////////////////////////////////////////////////////////////////
    // 失敗
    else return false;

    return true;
}


//
// 画像表示
//
void ArticleViewBase::open_image( const std::string& url, const int res_number,
                                  const bool open_imageview, const bool open_browser,
                                  const int mosaic_mode, const bool switch_image )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::open_image url = " << url << " num = " << res_number
              << " open_view = " << open_imageview << " open_browser = " << open_browser << " mosaic = " << mosaic_mode
              << " switch_image = " << switch_image << std::endl;
#endif

    bool load = false;

    // キャッシュに無かったらロード
    if( ! DBIMG::is_cached( url ) ){

        DBIMG::download_img( url, DBTREE::url_readcgi( m_url_article, res_number, 0 ), mosaic_mode );

        // ポップアップ表示してダウンロードサイズを表示
        hide_popup();
        SKELETON::View* view_popup = CORE::ViewFactory( CORE::VIEW_IMAGEPOPUP,  url );
        const int margin_popup_x = CONFIG::get_margin_imgpopup_x();
        const int margin_popup_y = CONFIG::get_margin_imgpopup();
        show_popup( view_popup, margin_popup_x, margin_popup_y );
        load = true;
    }

    // 画像ビューを開く
    if( open_imageview ){

        CORE::core_set_command( "open_image", url, mosaic_mode == DBIMG::kForceMosaic ? "force_mosaic" : "" );
        if( ! load && switch_image ) CORE::core_set_command( "switch_image" );
    }

    // ブラウザで画像を開く
    else if( ! load && open_browser ) CORE::core_set_command( "open_url", url );

    redraw_view();
}


//
// ポップアップが表示されているか
//
bool ArticleViewBase::is_popup_shown() const
{
    return ( m_popup_win && m_popup_shown );
}


//
// ポップアップが表示されていてかつマウスがその上にあるか
//
bool ArticleViewBase::is_mouse_on_popup()
{
    if( ! is_popup_shown() ) return false;
    return m_popup_win->view()->is_mouse_on_view();
}



//
// ポップアップ表示
//
// view にあらかじめ内容をセットしてから呼ぶこと
// viewは SKELETON::PopupWin のデストラクタで削除される
//
void ArticleViewBase::show_popup( SKELETON::View* view, const int mrg_x, const int mrg_y )
{
    hide_popup();
    if( !view ) return;

    delete_popup();

    m_popup_win = std::make_unique<SKELETON::PopupWin>( this, view, mrg_x, mrg_y );
    m_popup_win->signal_leave_notify_event().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_popup_leave_notify_event ) );
    m_popup_win->sig_hide_popup().connect( sigc::mem_fun( *this, &ArticleViewBase::slot_hide_popup ) );
    m_popup_win->show_all();
    m_popup_shown = true;
}



//
// 子 popup windowの外にポインタが出た
//
bool ArticleViewBase::slot_popup_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_popup_leave_notify_event\n";
#endif

    // クリックしたときやホイールを回すと event->mode に　GDK_CROSSING_GRAB
    // か GDK_CROSSING_UNGRAB がセットされてイベントが発生する場合がある
    if( event->mode == GDK_CROSSING_GRAB ) return false;
    if( event->mode == GDK_CROSSING_UNGRAB ) return false;

    // ポインタがポップアップ上だったらポップアップは消さない
    // 時々ポインタがポップアップの上にあってもイベントが発生する場合がある
    if( is_mouse_on_popup() ) return false;


    if( CONFIG::get_hide_popup_msec() ) m_hidepopup_counter = CONFIG::get_hide_popup_msec() / TIMER_TIMEOUT;
    else slot_hide_popup();

    return true;
}


//
// 子 popup windowからhide依頼が来た
//
void ArticleViewBase::slot_hide_popup()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_hide_popup\n";
#endif

    hide_popup();

    // ポインタがwidgetの外にあったら親に知らせて自分も閉じてもらう
    if( ! is_mouse_on_view() ) sig_hide_popup().emit();
}



//
// popup のhide
//
// force = true ならチェック無しで強制 hide
//
void ArticleViewBase::hide_popup( const bool force )
{
    if( ! is_popup_shown() ) return;

    m_hidepopup_counter = 0;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::hide_popup force = " << force << " " << get_url() << std::endl;
#endif

    // ArticleView をポップアップ表示している場合
    ArticleViewBase* popup_article = nullptr;
    popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );

    if( popup_article ){

        if( ! force ){

            // 孫のpopupが表示されてたらhideしない
            if( popup_article->is_popup_shown() ) return;

            // ポップアップメニューが表示されてたらhideしない
            // ( ポップアップメニューがhideしたときにhideする )
            if( SESSION::is_popupmenu_shown() ) return;

#ifdef _DEBUG
            std::cout << "target = " << popup_article->get_url() << std::endl;
#endif
        }

        popup_article->hide_popup( force );
    }

    m_popup_win->hide();
    if( m_popup_win->view() ) m_popup_win->view()->stop();
    m_popup_shown = false;
    m_drawarea->redraw_view();
}



//
// ポップアップの削除
//
void ArticleViewBase::delete_popup()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::delete_popup " << get_url() << std::endl;
#endif

    m_popup_win.reset();
    m_popup_shown = false;
}


//
// クリップボードに選択文字コピーのメニュー
//
void ArticleViewBase::slot_copy_selection_str()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_copy_selection_str " << get_url() << std::endl;
#endif

    if( m_drawarea->str_selection().empty() ) return;
    MISC::CopyClipboard( m_drawarea->str_selection() );
}


//
// 全選択
//
void ArticleViewBase::slot_select_all()
{
    if( m_drawarea ) m_drawarea->select_all();
}


//
// 選択して抽出
//
void ArticleViewBase::slot_drawout_selection_str()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::ascii_trim( query );
    query = MISC::replace_str( query, "\n", "" );
    if( query.find( ' ' ) != std::string::npos ) query = "\"" + query + "\"";

    if( query.empty() ) return;

    const bool escape = false;  // \ を エスケープ文字として考慮しない
    CORE::core_set_command( "open_article_keyword" ,m_url_article, MISC::regex_escape( query, escape ), "false" );
}


//
// 全ログ検索実行
//
void ArticleViewBase::slot_search_cacheall()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_cacheall "<< query << std::endl;
#endif

    CORE::core_set_command( "open_article_searchlog", URL_SEARCH_ALLBOARD , query, "exec" );
}


//
// ログ検索実行
//
void ArticleViewBase::slot_search_cachelocal()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

    const std::string url = DBTREE::url_boardbase( m_url_article );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_cachelocal " << url << std::endl
              << query << std::endl;
#endif

    CORE::core_set_command( "open_article_searchlog", url , query, "exec" );
}


//
// 次スレ検索
//
void ArticleViewBase::slot_search_next()
{
    if( m_article->empty() ) return;

    CORE::core_set_command( "open_board_next", DBTREE::url_boardbase( m_url_article ), m_url_article );
}


//
// web検索実行
//
void ArticleViewBase::slot_search_web()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

    if( query.empty() ) return;

    std::string url = CORE::get_usrcmd_manager()->replace_cmd( CONFIG::get_url_search_web(), "", "", query, 0 );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_web query = " << query << std::endl;
    std::cout << "url = " << url << std::endl;
#endif

    CORE::core_set_command( "open_url_browser", url );
}


//
// スレタイ検索実行
//
void ArticleViewBase::slot_search_title()
{
    std::string query = m_drawarea->str_selection();
    query = MISC::replace_str( query, "\n", "" );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_search_title query = " << query << std::endl;
#endif

    if( query.empty() ) CORE::core_set_command( "open_article_searchtitle", "", "", "noexec" );
    else CORE::core_set_command( "open_article_searchtitle", "" , query, "exec" );
}


//
// ユーザコマンド実行
//
void ArticleViewBase::slot_usrcmd( int num )
{
    std::string current = "0";
    std::string selection;
    if( m_drawarea ) {
        current = std::to_string( m_drawarea->get_current_res_num() );
        selection = m_drawarea->str_selection();
    }

    CORE::core_set_command( "exec_usr_cmd", m_url_article, std::to_string( num ), m_url_tmp, selection, current );
}



//
// ブックマーク設定、解除
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_bookmark()
{
    if( m_str_num.empty() ) return;

    int number = atoi( m_str_num.c_str() );
    bool bookmark = ! DBTREE::is_bookmarked( m_url_article, number );
    DBTREE::set_bookmark( m_url_article, number, bookmark );
    if( bookmark ) m_current_bm = number;
    redraw_view();
    ARTICLE::get_admin()->set_command( "redraw_views", m_url_article );
}



//
// 書き込みマーク設定、解除
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_postedmark()
{
    if( m_str_num.empty() ) return;

    int number = atoi( m_str_num.c_str() );
    bool postedmark = ! m_article->is_posted( number );
    m_article->set_posted( number, postedmark );
    redraw_view();
    ARTICLE::get_admin()->set_command( "redraw_views", m_url_article );
}



//
// ポップアップメニューでブラウザで開くを選択
//
void ArticleViewBase::slot_open_browser()
{
    if( m_url_tmp.empty() ) return;

    CORE::core_set_command( "open_url_browser", m_url_tmp );
}


//
// ポップアップメニューで画像のキャッシュをブラウザで開くを選択
//
void ArticleViewBase::slot_open_cache_browser()
{
    if( m_url_tmp.empty() ) return;
    if( ! DBIMG::is_cached( m_url_tmp ) ) return;

    const std::string url = "file://" + DBIMG::get_cache_path( m_url_tmp );
    CORE::core_set_command( "open_url_browser", url );
}


//
// レスをする
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_write_res()
{
    if( m_article->empty() ) return;
    if( m_str_num.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_write_res number = " << m_str_num << std::endl;
#endif

    CORE::core_set_command( "open_message" ,m_url_article, ">>" + m_str_num + "\n" );
}


//
// 参照レスをする
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_quote_res()
{
    assert( m_article );
    if( m_str_num.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_quote_res number = " << m_str_num << std::endl;
#endif

    CORE::core_set_command( "open_message" ,m_url_article,
                            ">>" + m_str_num + "\n" + m_article->get_res_str( atoi( m_str_num.c_str() ), true ) );
}


//
// 選択部分を引用してレスする
//
void ArticleViewBase::slot_quote_selection_res()
{
    assert( m_article );

    const int num_from = m_drawarea->get_selection_resnum_from();
    if( ! num_from ) return;

    const int num_to = m_drawarea->get_selection_resnum_to();

    std::string str_num = std::to_string( num_from );
    if( num_from < num_to ) str_num += "-" + std::to_string( num_to );

    std::string str_res;
    str_res = CONFIG::get_ref_prefix();

    std::string query = m_drawarea->str_selection();
    if( query.empty() ) return;

    query = MISC::replace_str( query, "\n", "\n" + str_res );

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_quote_selection_res number = " << str_num << std::endl;
#endif

    CORE::core_set_command( "open_message", m_url_article, ">>" + str_num + "\n" + str_res + query + "\n" );
}



//
// リンクのURLをコピーのメニュー
//
void ArticleViewBase::slot_copy_current_url()
{
    if( m_url_tmp.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_copy_current_url url = " << m_url_tmp << std::endl;
#endif

    MISC::CopyClipboard( m_url_tmp );
}


//
// 名前をコピー
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_copy_name()
{
    std::string name = m_name;
    MISC::CopyClipboard( name );
}


//
// IDをコピー
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_copy_id()
{
    std::string id = m_id_name.substr( strlen( PROTO_ID ) );
    MISC::CopyClipboard( id );
}



//
// レス番号クリック時のレスのコピーのメニュー
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_copy_res( bool ref )
{
    assert( m_article );
    if( m_str_num.empty() ) return;

#ifdef _DEBUG
    std::cout << "ArticleViewBase::copy_res number = " << m_str_num << std::endl;
#endif

    std::string tmpstr = m_url_tmp + "\n";
    if( ref ) tmpstr += CONFIG::get_ref_prefix();
    const std::string& board_name = DBTREE::board_name( m_url_article );
    if( ! board_name.empty() ) tmpstr += "[ " + board_name + " ] ";
    tmpstr += MISC::to_plain( DBTREE::article_subject( m_url_article ) ) + "\n\n";
    tmpstr += m_article->get_res_str( atoi( m_str_num.c_str() ), ref );

    MISC::CopyClipboard( tmpstr );
}


//
// スレのタイトルとURLをコピー
//
void ArticleViewBase::slot_copy_title_url()
{
    MISC::CopyClipboard( MISC::to_plain( DBTREE::article_subject( m_url_article ) ) + '\n' + url_for_copy() );
}


//
// お気に入り登録
//
void ArticleViewBase::set_favorite()
{
    if( m_article->empty() ) return;

    CORE::DATA_INFO info;
    info.type = TYPE_THREAD;
    info.parent = ARTICLE::get_admin()->get_win();
    info.url = m_url_article;
    info.name = MISC::to_plain( DBTREE::article_modified_subject( m_url_article ) );
    info.path = Gtk::TreePath( "0" ).to_string();

    CORE::DATA_INFO_LIST list_info;
    list_info.push_back( info );
    CORE::SBUF_set_list( list_info );

    CORE::core_set_command( "append_favorite", URL_FAVORITEVIEW );
}



//
// 別のタブを開いてレス抽出
//
// 呼び出す前に m_str_num に抽出するレスをセットしておくこと
//
void ArticleViewBase::slot_drawout_res()
{
    CORE::core_set_command( "open_article_res" ,m_url_article, m_str_num );
}



//
// 別のタブを開いて周辺のレスを抽出
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_drawout_around()
{
    const int range = 10;
    int center = atoi( m_str_num.c_str() );
    int from = MAX( 0, center - range );
    int to = center + range;
    std::stringstream ss;
    ss << from << "-" << to;
    CORE::core_set_command( "open_article_res" ,m_url_article, ss.str(), m_str_num );
}


//
// 別のタブを開いてテンプレート表示
//
void ArticleViewBase::slot_drawout_tmp()
{
    const int to = 20;
    std::stringstream ss;
    ss << "1-" << to;
    CORE::core_set_command( "open_article_res" ,m_url_article, ss.str() );
}


//
// 別のタブを開いて名前抽出
//
// 呼び出す前に m_name にIDをセットしておくこと
//
void ArticleViewBase::slot_drawout_name()
{
    CORE::core_set_command( "open_article_name" ,m_url_article, m_name );
}



//
// 別のタブを開いてID抽出
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_drawout_id()
{
    CORE::core_set_command( "open_article_id" ,m_url_article, m_id_name );
}



//
// 別のタブを開いてブックマーク抽出
//
void ArticleViewBase::slot_drawout_bm()
{
    CORE::core_set_command( "open_article_bm" ,m_url_article );
}


//
// 別のタブを開いて自分の書き込みを抽出
//
void ArticleViewBase::slot_drawout_post()
{
    CORE::core_set_command( "open_article_post" ,m_url_article );
}


//
// 別のタブを開いて高参照レスを抽出
//
void ArticleViewBase::slot_drawout_highly_referenced_res()
{
    CORE::core_set_command( "open_article_highly_referened_res", m_url_article );
}


//
// 別のタブを開いて参照抽出
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_drawout_refer()
{
    CORE::core_set_command( "open_article_refer" ,m_url_article, m_str_num );
}



//
// 別のタブを開いてURL抽出
//
void ArticleViewBase::slot_drawout_url()
{
    CORE::core_set_command( "open_article_url" ,m_url_article );
}



//
// レス番号であぼ〜ん
//
// 呼び出す前に m_str_num に対象のレス番号を入れておくこと
//
void ArticleViewBase::slot_abone_res()
{
    const int number = atoi( m_str_num.c_str() );

    DBTREE::set_abone_res( m_url_article, number, number, true );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 選択範囲内のレス番号であぼ〜ん
//
void ArticleViewBase::slot_abone_selection_res()
{
    const int num_from = m_drawarea->get_selection_resnum_from();
    if( ! num_from ) return;

    const int num_to = m_drawarea->get_selection_resnum_to();

    DBTREE::set_abone_res( m_url_article, num_from, num_to, true );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// IDであぼ〜ん(ローカル、板(一時的) )
//
// 呼び出す前に m_id_name にIDをセットしておくこと
//
void ArticleViewBase::slot_abone_id()
{
    DBTREE::add_abone_id( m_url_article, m_id_name );
    DBTREE::add_abone_id_board( m_url_article, m_id_name );
}



//
// 名前であぼ〜ん
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_abone_name()
{
    DBTREE::add_abone_name( m_url_article, m_name );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 範囲選択した文字列であぼ〜ん
//
void ArticleViewBase::slot_abone_word()
{
    DBTREE::add_abone_word( m_url_article, m_drawarea->str_selection() );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 名前であぼ〜ん(板レベル)
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_abone_name_board()
{
    DBTREE::add_abone_name_board( m_url_article, m_name );
    DBTREE::board_save_info( m_url_article );
}


//
// 範囲選択した文字列であぼ〜ん(板レベル)
//
void ArticleViewBase::slot_abone_word_board()
{
    DBTREE::add_abone_word_board( m_url_article, m_drawarea->str_selection() );
    DBTREE::board_save_info( m_url_article );
}


//
// 名前であぼ〜ん(全体)
//
// 呼び出す前に m_name に名前をセットしておくこと
//
void ArticleViewBase::slot_global_abone_name()
{
    CORE::core_set_command( "set_global_abone_name", "", m_name );
}


//
// 範囲選択した文字列であぼ〜ん(全体)
//
void ArticleViewBase::slot_global_abone_word()
{
    CORE::core_set_command( "set_global_abone_word", "", m_drawarea->str_selection() );
}



//
// 透明あぼーん
//
void ArticleViewBase::slot_toggle_abone_transp()
{
    if( ! m_enable_menuslot ) return;

    assert( m_article );

    bool setval = true;

    if( m_article->get_abone_transparent() ) setval = false;
    m_article->set_abone_transparent( setval );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}


//
// 透明/連鎖あぼーん
//
void ArticleViewBase::slot_toggle_abone_transp_chain()
{
    if( ! m_enable_menuslot ) return;

    assert( m_article );

    bool setval = true;

    if( m_article->get_abone_transparent() && m_article->get_abone_chain() ) setval = false;
    m_article->set_abone_transparent( setval );
    m_article->set_abone_chain( setval );

    // 再レイアウト
    ARTICLE::get_admin()->set_command( "relayout_views", m_url_article );
}



//
// 画像のモザイク解除
//
void ArticleViewBase::slot_cancel_mosaic()
{
    if( ! DBIMG::is_cached( m_url_tmp ) ) return;

    if( DBIMG::is_fake( m_url_tmp ) ){

        SKELETON::MsgDiag mdiag( get_parent_win(),
                                 "拡張子が偽装されています。モザイクを解除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );

        mdiag.set_default_response( Gtk::RESPONSE_NO );
        if( mdiag.run() != Gtk::RESPONSE_YES ) return;
    }

    DBIMG::set_mosaic( m_url_tmp, false );
    CORE::core_set_command( "redraw", m_url_tmp );
}


//
// 画像をモザイク付きでダウンロード及び表示
//
void ArticleViewBase::slot_show_image_with_mosaic()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_show_image_with_mosaic url = " << m_url_tmp << " num = " << m_str_num << std::endl;
#endif

    if( DBIMG::is_cached( m_url_tmp ) ) return;

    const int res_number = atoi( m_str_num.c_str() );
    if( ! res_number ) return;

    const bool open_imageview = CONFIG::get_use_image_view();
    const bool open_browser = ! open_imageview;
    constexpr int mosaic_mode = DBIMG::kForceMosaic;
    const bool switch_image = open_imageview;

    open_image( m_url_tmp, res_number, open_imageview, open_browser, mosaic_mode, switch_image );
}



//
// 選択範囲の画像を開く
//
void ArticleViewBase::slot_show_selection_images()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_show_selection_images\n";
#endif

    if( m_drawarea->get_selection_imgurls().size() ){

        bool first = true;
        for( const URLINFO& info : m_drawarea->get_selection_imgurls() ) {

            const std::string& url = info.url;
            const int res_number = info.res_number;

            if( DBIMG::get_abone( url ) ) continue;

            // オフラインでキャッシュが無い場合はスキップ
            if( ! SESSION::is_online() && ! DBIMG::is_cached( url ) ) continue;

            const std::string refurl = DBTREE::url_readcgi( m_url_article, res_number, 0 );
            const bool open_imageview = CONFIG::get_use_image_view();
            const bool mosaic = CONFIG::get_use_mosaic();

#ifdef _DEBUG
            std::cout << url << " - " << res_number << " : " << refurl << std::endl;
#endif

            if( ! DBIMG::is_cached( url ) ){

                DBIMG::download_img_wait( url, refurl, mosaic, first );
                first = false;
            }

            if( open_imageview ) CORE::core_set_command( "open_image", url );
        }

        redraw_view();
    }
}


//
// 選択範囲の画像を削除
//
void ArticleViewBase::slot_delete_selection_images()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_delete_selection_images\n";
#endif

    if( m_drawarea->get_selection_imgurls().size() ){

        for( const URLINFO& info : m_drawarea->get_selection_imgurls() ) {

            const std::string& url = info.url;
            if( ! DBIMG::is_protected( url ) ){
                CORE::core_set_command( "delete_image", url );
            }
        }

        redraw_view();
    }
}


//
// 選択範囲の画像をあぼーん
//
void ArticleViewBase::slot_abone_selection_images()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::slot_abone_selection_images\n";
#endif

    if( m_drawarea->get_selection_imgurls().size() ){

        for( const URLINFO& info : m_drawarea->get_selection_imgurls() ) {

            const std::string& url = info.url;
            if( ! DBIMG::is_protected( url ) ){
                DBIMG::set_abone( url, true );
                CORE::core_set_command( "delete_image", url );
            }
        }

        redraw_view();
    }
}



//
// 大きい画像を表示
//
void ArticleViewBase::slot_show_large_img()
{
    DBIMG::show_large_img( m_url_tmp );
}



//
// 画像削除
//
void ArticleViewBase::slot_deleteimage()
{
    if( ! m_url_tmp.empty() ) CORE::core_set_command( "delete_image", m_url_tmp );
}


//
// 画像保存
//
void ArticleViewBase::slot_saveimage()
{
    DBIMG::save( m_url_tmp, nullptr, std::string() );
}


//
// 画像保護
//
void ArticleViewBase::slot_toggle_protectimage()
{
    if( ! m_enable_menuslot ) return;

    DBIMG::set_protect( m_url_tmp , ! DBIMG::is_protected( m_url_tmp ) );
}


//
// 画像あぼ〜ん
//
void ArticleViewBase::slot_abone_img()
{
    if( ! m_enable_menuslot ) return;
    if( m_url_tmp.empty() ) return;

    DBIMG::set_abone( m_url_tmp , ! DBIMG::get_abone( m_url_tmp ) );
    if( DBIMG::get_abone( m_url_tmp ) ) CORE::core_set_command( "delete_image", m_url_tmp );
    redraw_view();
}


//
// 検索entryでenterを押した
//
void ArticleViewBase::exec_search()
{
    std::string query = get_search_query();
    if( query.empty() ){
        clear_highlight();
        // 検索entryが空欄の状態で検索したときは、キャレット（検索開始位置）を一番上に移動します。
        m_drawarea->reset_caret_position();
        focus_view();
        CORE::core_set_command( "set_info", "", "" );
        return;
    }

    std::list< std::string > list_query;
    list_query = MISC::split_line( query );

    if( get_pre_query() != query ){
        set_pre_query( query );
        CORE::get_completion_manager()->set_query( CORE::COMP_SEARCH_ARTICLE, query );
        m_drawarea->set_jump_history( m_drawarea->get_seen_current() );
        m_drawarea->search( list_query, m_search_invert );
    }

    int hit = m_drawarea->search_move( m_search_invert );

    if( ! hit ){
        clear_highlight();
        CORE::core_set_command( "set_info", "", "検索結果： ヒット無し" );
    }
    else{
        focus_view();
        CORE::core_set_command( "set_info", "", "検索結果： " + std::to_string( hit ) + "件" );
    }
}



//
// 検索entryの操作
//
void ArticleViewBase::operate_search( const std::string& controlid )
{
    const int id = atoi( controlid.c_str() );

    if( id == CONTROL::Cancel ){
        focus_view();
        ARTICLE::get_admin()->set_command( "close_searchbar" );
    }

    // AND 抽出
    else if( id == CONTROL::DrawOutAnd ){
        std::string query = get_search_query();
        if( query.empty() ) return;

        CORE::core_set_command( "open_article_keyword" ,m_url_article, query, "false" );
    }
}


//
// 実況モードのセット
//
void ArticleViewBase::set_live( const bool live )
{
    if( ! m_enable_live ) return;

    m_live = live;
    if( m_live ) SESSION::append_live( m_url_article );
    else SESSION::remove_live( m_url_article );
}
