// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

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


/**
 * @brief アクションを初期化する
 */
void ARTICLE::ArticleViewBase::setup_action()
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::setup_action\n";
#endif

    // アクショングループを作ってUIマネージャに登録
    action_group().reset();
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "BookMark", "しおりを設定/解除(_B)"), sigc::mem_fun( *this, &ArticleViewBase::slot_bookmark ) );
    action_group()->add( Gtk::Action::create( "PostedMark", "書き込みマークを設定/解除(_P)"), sigc::mem_fun( *this, &ArticleViewBase::slot_postedmark ) );
    action_group()->add( Gtk::Action::create( "OpenBrowser", ITEM_NAME_OPEN_BROWSER "(_W)" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "OpenBrowserRes", ITEM_NAME_OPEN_BROWSER "(_S)" ),   // レスをクリックした時のメニュー用
                         sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "OpenCacheBrowser", ITEM_NAME_OPEN_CACHE_BROWSER "(_X)" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_open_cache_browser ) );
    action_group()->add( Gtk::Action::create( "CopyURL", ITEM_NAME_COPY_URL "(_U)" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_copy_current_url ) );
    action_group()->add( Gtk::Action::create( "CopyTitleURL", ITEM_NAME_COPY_TITLE_URL_THREAD "(_L)" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_copy_title_url ) );
    action_group()->add( Gtk::Action::create( "CopyNAME", "名前コピー(_N)"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_name ) );
    action_group()->add( Gtk::Action::create( "CopyID", "IDコピー(_D)"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_id ) );
    action_group()->add( Gtk::Action::create( "Copy", "Copy"), sigc::mem_fun( *this, &ArticleViewBase::slot_copy_selection_str ) );
    action_group()->add( Gtk::Action::create( "WriteRes", "レスする(_W)" ),sigc::mem_fun( *this, &ArticleViewBase::slot_write_res ) );
    action_group()->add( Gtk::Action::create( "QuoteRes", "引用してレスする(_Q)"),sigc::mem_fun( *this, &ArticleViewBase::slot_quote_res ) );
    action_group()->add( Gtk::Action::create( "QuoteSelectionRes", ITEM_NAME_QUOTE_SELECTION "(_Q)" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_quote_selection_res ) );
    action_group()->add( Gtk::Action::create( "CopyRes", "レスをコピー(_R)"),
                         sigc::bind< bool >( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), false ) );
    action_group()->add( Gtk::Action::create( "CopyResRef", "引用コピー(_F)"),
                         sigc::bind< bool >( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), true ) );
    action_group()->add( Gtk::Action::create( "Delete_Menu", "削除(_D)" ) );
    action_group()->add( Gtk::Action::create( "Delete", "Delete"), sigc::mem_fun( *this, &ArticleViewBase::exec_delete ) );
    action_group()->add( Gtk::Action::create( "DeleteOpen", "スレ情報を消さずにスレ再取得(_R)"), sigc::mem_fun( *this, &ArticleViewBase::delete_open_view ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "AppendFavorite"), sigc::mem_fun( *this, &ArticleViewBase::set_favorite ) );
    action_group()->add( Gtk::Action::create( "Reload", "Reload"), sigc::mem_fun( *this, &ArticleViewBase::exec_reload ) );
    action_group()->add( Gtk::Action::create( "PreferenceArticle", "PreferenceArticle" ), sigc::mem_fun( *this, &ArticleViewBase::show_preference ) );
    action_group()->add( Gtk::Action::create( "PreferenceImage", ITEM_NAME_PREF_IMAGE "(_M)..." ), sigc::mem_fun( *this, &ArticleViewBase::slot_preferences_image ) );

    // 検索
    action_group()->add( Gtk::Action::create( "Search_Menu", ITEM_NAME_SEARCH "(_H)" ) );
    action_group()->add( Gtk::Action::create( "SearchNextArticle", "SearchNextArticle"), sigc::mem_fun( *this, &ArticleViewBase::slot_search_next ) );
    action_group()->add( Gtk::Action::create( "SearchWeb", "SearchWeb" ), sigc::mem_fun( *this, &ArticleViewBase::slot_search_web ) );
    action_group()->add( Gtk::Action::create( "SearchCacheLocal", "SearchCacheLocal" ), sigc::mem_fun( *this, &ArticleViewBase::slot_search_cachelocal ) );
    action_group()->add( Gtk::Action::create( "SearchCacheAll", "SearchCacheAll") );
    action_group()->add( Gtk::Action::create( "ExecSearchCacheAll", "検索する(_E)"), sigc::mem_fun( *this, &ArticleViewBase::slot_search_cacheall ) );
    action_group()->add( Gtk::Action::create( "SearchTitle", "SearchTitle" ), sigc::mem_fun( *this, &ArticleViewBase::slot_search_title ) );

    // 抽出系
    action_group()->add( Gtk::Action::create( "Drawout_Menu", ITEM_NAME_DRAWOUT "(_E)" ) );
    action_group()->add( Gtk::Action::create( "DrawoutWord", "キーワード抽出(_K)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_selection_str ) );
    action_group()->add( Gtk::Action::create( "DrawoutRes", "レス抽出(_R)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_res ) );
    action_group()->add( Gtk::Action::create( "DrawoutNAME", "名前抽出(_E)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_name ) );
    action_group()->add( Gtk::Action::create( "DrawoutID", "ID抽出(_I)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_id ) );
    action_group()->add( Gtk::Action::create( "DrawoutBM", "しおり抽出(_B)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_bm ) );
    action_group()->add( Gtk::Action::create( "DrawoutPost", "書き込み抽出(_W)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_post ) );
    action_group()->add( Gtk::Action::create( "DrawoutHighRefRes", "高参照レス抽出(_H)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_highly_referenced_res ) );
    action_group()->add( Gtk::Action::create( "DrawoutURL", "URL抽出(_U)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_url ) );
    action_group()->add( Gtk::Action::create( "DrawoutRefer", "参照抽出(_E)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_refer ) );
    action_group()->add( Gtk::Action::create( "DrawoutAround", "周辺抽出(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_around ) );
    action_group()->add( Gtk::Action::create( "DrawoutTmp", "テンプレート抽出(_T)"), sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_tmp ) );

    // あぼーん系
    action_group()->add( Gtk::Action::create( "AboneWord_Menu", ITEM_NAME_NGWORD "(_N)" ) );
    action_group()->add( Gtk::Action::create( "AboneRes", "レスをあぼ〜んする(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_res ) );
    action_group()->add( Gtk::Action::create( "AboneSelectionRes", "AboneSelectionRes" ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_abone_selection_res ) );
    action_group()->add( Gtk::Action::create( "AboneID", "NG IDに追加(_G)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_id ) );
    action_group()->add( Gtk::Action::create( "AboneName", "NG 名前に追加 (対象: ローカル)(_L)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name ) );
    action_group()->add( Gtk::Action::create( "AboneWord", "NG ワードに追加 (対象: ローカル)(_L)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_word ) );

    action_group()->add( Gtk::Action::create( "AboneNameBoard", "NG 名前に追加 (対象: 板)(_B)" ) );
    action_group()->add( Gtk::Action::create( "SetAboneNameBoard", "追加する(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name_board ) );
    action_group()->add( Gtk::Action::create( "AboneWordBoard", "NG ワードに追加 (対象: 板)(_B)" ) );
    action_group()->add( Gtk::Action::create( "SetAboneWordBoard", "追加する(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_word_board ) );

    action_group()->add( Gtk::Action::create( "GlobalAboneName", "NG 名前に追加 (対象: 全体)(_A)" ) );
    action_group()->add( Gtk::Action::create( "SetGlobalAboneName", "追加する(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_global_abone_name ) );
    action_group()->add( Gtk::Action::create( "GlobalAboneWord", "NG ワードに追加 (対象: 全体)(_A)" ) );
    action_group()->add( Gtk::Action::create( "SetGlobalAboneWord", "追加する(_A)"), sigc::mem_fun( *this, &ArticleViewBase::slot_global_abone_word ) );

    action_group()->add( Gtk::ToggleAction::create( "TranspAbone", "透明あぼ〜ん(_T)", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp ) );
    action_group()->add( Gtk::ToggleAction::create( "TranspChainAbone", "透明/連鎖あぼ〜ん(_C)", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp_chain ) );

    action_group()->add( Gtk::Action::create( "SetupAbone", "あぼ〜ん設定(対象: ローカル)(_L)..."), sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone ) );
    action_group()->add( Gtk::Action::create( "SetupAboneBoard", "あぼ〜ん設定(対象: 板)(_B)..." ), sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone_board ) );
    action_group()->add( Gtk::Action::create( "SetupAboneAll", "あぼ〜ん設定(対象: 全体)(_A)..." ), sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone_all ) );

    // 移動系
    action_group()->add( Gtk::Action::create( "Move_Menu", ITEM_NAME_GO "(_M)" ) );
    action_group()->add( Gtk::Action::create( "Home", "Home"), sigc::mem_fun( *this, &ArticleViewBase::goto_top ) );
    action_group()->add( Gtk::Action::create( "GotoNew", "GotoNew"), sigc::mem_fun( *this, &ArticleViewBase::goto_new ) );
    action_group()->add( Gtk::Action::create( "End", "End"), sigc::mem_fun( *this, &ArticleViewBase::goto_bottom ) );
    action_group()->add( Gtk::Action::create( "PreBookMark", "PreBookMark"), sigc::mem_fun( *this, &ArticleViewBase::slot_pre_bm ) );
    action_group()->add( Gtk::Action::create( "NextBookMark", "NextBookMark"), sigc::mem_fun( *this, &ArticleViewBase::slot_next_bm ) );
    action_group()->add( Gtk::Action::create( "PrePost", "PrePost"), sigc::mem_fun( *this, &ArticleViewBase::slot_pre_post ) );
    action_group()->add( Gtk::Action::create( "NextPost", "NextPost"), sigc::mem_fun( *this, &ArticleViewBase::slot_next_post ) );
    action_group()->add( Gtk::Action::create( "Jump", "ジャンプ(_J)"), sigc::mem_fun( *this, &ArticleViewBase::slot_jump ) );
    action_group()->add( Gtk::Action::create( "PrevView", "PrevView"),
                         sigc::bind< int >( sigc::mem_fun( *this, &ArticleViewBase::back_viewhistory ), 1 ) );
    action_group()->add( Gtk::Action::create( "NextView", "NextView"),
                         sigc::bind< int >( sigc::mem_fun( *this, &ArticleViewBase::forward_viewhistory ), 1 ) );

    // 画像系
    action_group()->add( Gtk::Action::create( "Cancel_Mosaic", "モザイク解除(_C)"), sigc::mem_fun( *this, &ArticleViewBase::slot_cancel_mosaic ) );
    action_group()->add( Gtk::Action::create( "Show_Mosaic", "モザイクで開く(_M)"), sigc::mem_fun( *this, &ArticleViewBase::slot_show_image_with_mosaic ) );
    action_group()->add( Gtk::Action::create( "ShowSelectImage", "ShowSelectImage" )
                         , sigc::mem_fun( *this, &ArticleViewBase::slot_show_selection_images ) );
    action_group()->add( Gtk::Action::create( "DeleteSelectImage_Menu", ITEM_NAME_SELECTDELIMG "(_T)" ) );
    action_group()->add( Gtk::Action::create( "DeleteSelectImage", "DeleteSelectImage"), sigc::mem_fun( *this, &ArticleViewBase::slot_delete_selection_images ) );
    action_group()->add( Gtk::Action::create( "AboneSelectImage_Menu", ITEM_NAME_SELECTABONEIMG "(_B)" ) );
    action_group()->add( Gtk::Action::create( "AboneSelectImage", "AboneSelectImage"), sigc::mem_fun( *this, &ArticleViewBase::slot_abone_selection_images ) );
    action_group()->add( Gtk::Action::create( "ShowLargeImg", "サイズが大きい画像を表示(_L)"),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_show_large_img ) );
    action_group()->add( Gtk::ToggleAction::create( "ProtectImage", "キャッシュを保護する(_P)", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_protectimage ) );
    action_group()->add( Gtk::Action::create( "DeleteImage_Menu", "削除(_D)" ) );
    action_group()->add( Gtk::Action::create( "DeleteImage", "削除する(_D)"), sigc::mem_fun( *this, &ArticleViewBase::slot_deleteimage ) );
    action_group()->add( Gtk::Action::create( "SaveImage", "名前を付けて保存(_S)..."), sigc::mem_fun( *this, &ArticleViewBase::slot_saveimage ) );
    action_group()->add( Gtk::ToggleAction::create( "AboneImage", "画像をあぼ〜んする(_A)", std::string(), false ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_abone_img ) );

    // その他
    action_group()->add( Gtk::Action::create( "Etc_Menu", ITEM_NAME_ETC "(_O)" ) );
    action_group()->add( Gtk::Action::create( "SaveDat", "SaveDat" ), sigc::mem_fun( *this, &ArticleViewBase::slot_save_dat ) );
    action_group()->add( Gtk::Action::create( "CopyInfo", ITEM_NAME_COPY_THREAD_INFO "(_I)..." ),
                         sigc::mem_fun( *this, &ArticleViewBase::slot_copy_article_info ) );

    m_usrcmd = CORE::get_usrcmd_manager()->create_usrcmd_menu( action_group() );
    const int usrcmd_size = CORE::get_usrcmd_manager()->get_size();
    for( int i = 0; i < usrcmd_size; ++i ){
        Glib::RefPtr< Gtk::Action > act = CORE::get_usrcmd_manager()->get_action( action_group(), i );
        if( act ) act->signal_activate().connect(
            sigc::bind< int >( sigc::mem_fun( *this, &ArticleViewBase::slot_usrcmd ), i ) );
    }

    ui_manager().reset();
    ui_manager() = Gtk::UIManager::create();
    ui_manager()->insert_action_group( action_group() );

    // 削除ボタン押したときのポップアップ
    const std::string menu_delete =
    "<popup name='popup_menu_delete'>"
    "<menuitem action='Delete'/>"
    "<separator/>"
    "<menuitem action='DeleteOpen'/>"
    "</popup>";

    // 壊れていますをクリックしたときのポップアップ
    const std::string menu_broken =
    "<popup name='popup_menu_broken'>"
    "<menuitem action='DeleteOpen'/>"
    "</popup>";

    // レス番号をクリックしたときのメニュー
    const std::string menu_res =
    "<popup name='popup_menu_res'>"
    "<menuitem action='BookMark'/>"
    "<menuitem action='PostedMark'/>"
    "<separator/>"

    "<menu action='Drawout_Menu'>"
    "<menuitem action='Jump'/>"
    "<menuitem action='DrawoutRefer'/>"
    "<menuitem action='DrawoutAround'/>"
    "<menuitem action='DrawoutRes'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='WriteRes'/>"
    "<menuitem action='QuoteRes'/>"
    "<separator/>"

    "<menuitem action='OpenBrowserRes'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<menuitem action='CopyRes'/>"
    "<menuitem action='CopyResRef'/>"
    "<separator/>"

    "<menuitem action='AboneRes'/>"
    "</popup>";

    // レスアンカーをクリックしたときのメニュー
    const std::string menu_anc =
    "<popup name='popup_menu_anc'>"
    "<menuitem action='Jump'/>"
    "<menuitem action='DrawoutAround'/>"
    "<menuitem action='DrawoutRes'/>"
    "</popup>";

    // IDをクリックしたときのメニュー
    const std::string menu_id =
    "<popup name='popup_menu_id'>"
    "<menuitem action='DrawoutID'/>"
    "<menuitem action='CopyID'/>"
    "<separator/>"
    "<menuitem action='AboneID'/>"
    "</popup>";

    // 名前をクリックしたときのメニュー
    const std::string menu_name =
    "<popup name='popup_menu_name'>"
    "<menuitem action='DrawoutNAME'/>"
    "<menuitem action='CopyNAME'/>"
    "<separator/>"

    "<menuitem action='AboneName'/>"

    "<menu action='AboneNameBoard'>"
    "<menuitem action='SetAboneNameBoard'/>"
    "</menu>"

    "<menu action='GlobalAboneName'>"
    "<menuitem action='SetGlobalAboneName'/>"
    "</menu>"

    "</popup>";

    // あぼーんをクリックしたときのメニュー
    const std::string menu_abone =
    "<popup name='popup_menu_abone'>"
    "<menuitem action='TranspAbone'/>"
    "<menuitem action='TranspChainAbone'/>"
    "<separator/>"
    "<menuitem action='SetupAbone'/>"
    "<menuitem action='SetupAboneBoard'/>"
    "<menuitem action='SetupAboneAll'/>"
    "</popup>";


    // 画像メニュー
    const std::string menu_img =
    "<popup name='popup_menu_img'>"
    "<menuitem action='Cancel_Mosaic'/>"
    "<menuitem action='Show_Mosaic'/>"
    "<menuitem action='ShowLargeImg'/>"
    "<separator/>"
    "<menuitem action='OpenBrowser'/>"
    "<menuitem action='OpenCacheBrowser'/>"
    + m_usrcmd
    + std::string(
    "<separator/>"
    "<menuitem action='CopyURL'/>"
    "<separator/>"
    "<menuitem action='SaveImage'/>"
    "<separator/>"
    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteImage_Menu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"
    "<separator/>"
    "<menuitem action='AboneImage'/>"
    "<separator/>"
    "<menuitem action='PreferenceImage'/>"
    "</popup>"
        );

    ui_manager()->add_ui_from_string(
        "<ui>"
        + menu_delete
        + menu_broken
        + menu_res
        + menu_anc
        + menu_id
        + menu_name
        + menu_abone
        + menu_img
        + create_context_menu()
        + "</ui>"
        );

    // ポップアップメニューにショートカットキーやマウスジェスチャを表示
    Gtk::Menu* popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_delete" ) );
    CONTROL::set_menu_motion( popupmenu );
}


/**
 * @brief 通常の右クリックメニューを作成する
 */
std::string ARTICLE::ArticleViewBase::create_context_menu() const
{
    std::list< int > list_menu;

    list_menu.push_back( ITEM_DRAWOUT );
    list_menu.push_back( ITEM_GO );
    list_menu.push_back( ITEM_SEARCH );
    list_menu.push_back( ITEM_NGWORD );
    list_menu.push_back( ITEM_QUOTE_SELECTION );
    list_menu.push_back( ITEM_OPEN_BROWSER );
    list_menu.push_back( ITEM_USER_COMMAND );
    list_menu.push_back( ITEM_COPY_URL );
    list_menu.push_back( ITEM_COPY );
    list_menu.push_back( ITEM_RELOAD );
    list_menu.push_back( ITEM_DELETE );
    list_menu.push_back( ITEM_COPY_TITLE_URL_THREAD );
    list_menu.push_back( ITEM_SAVE_DAT );
    list_menu.push_back( ITEM_COPY_THREAD_INFO );
    list_menu.push_back( ITEM_APPENDFAVORITE );
    list_menu.push_back( ITEM_ABONE_SELECTION );
    list_menu.push_back( ITEM_SELECTIMG );
    list_menu.push_back( ITEM_SELECTDELIMG );
    list_menu.push_back( ITEM_SELECTABONEIMG );
    list_menu.push_back( ITEM_PREF_THREAD );

    // メニューに含まれていない項目を抜き出して「その他」に含める
    int num = 0;
    for(;;){

        const int item = SESSION::get_item_article_menu( num );

        if( item == ITEM_END ) break;
        list_menu.remove( item );

        ++num;
    }

    std::string menu;
    num = 0;
    for(;;){

        const int item = SESSION::get_item_article_menu( num );

        if( item == ITEM_END ) break;
        else if( item == ITEM_ETC && list_menu.size() ){
            menu.append( "<menu action='Etc_Menu'>" );
            for( int etc_item : list_menu ) menu.append( get_menu_item( etc_item ) );
            menu.append( "</menu>" );
        }
        else menu.append( get_menu_item( item ) );

        ++num;
    }

#ifdef _DEBUG
    std::cout << "menu = " << menu << std::endl;
#endif

    return "<popup name='popup_menu'>" + menu + "</popup>";
}


/**
 * @brief メニュー項目に対応するXMLを取得する
 *
 * @param item メニュー項目
 * @return メニュー項目のXML
 */
const char* ARTICLE::ArticleViewBase::get_menu_item( const int item ) const
{
    switch( item ){

        // 抽出
        case ITEM_DRAWOUT:
            return
            "<menu action='Drawout_Menu'>"
            "<menuitem action='DrawoutWord'/>"
            "<menuitem action='DrawoutBM'/>"
            "<menuitem action='DrawoutPost'/>"
            "<menuitem action='DrawoutHighRefRes'/>"
            "<menuitem action='DrawoutURL'/>"
            "<menuitem action='DrawoutTmp'/>"
            "</menu>"
            ;

            // 移動
        case ITEM_GO:
            return
            "<menu action='Move_Menu'>"
            "<menuitem action='PrevView'/>"
            "<menuitem action='NextView'/>"
            "<separator/>"
            "<menuitem action='Home'/>"
            "<menuitem action='End'/>"
            "<separator/>"
            "<menuitem action='GotoNew'/>"
            "<separator/>"
            "<menuitem action='PreBookMark'/>"
            "<menuitem action='NextBookMark'/>"
            "<separator/>"
            "<menuitem action='PrePost'/>"
            "<menuitem action='NextPost'/>"
            "</menu>"
            ;

            // 検索
        case ITEM_SEARCH:
            return
            "<menu action='Search_Menu'>"
            "<menuitem action='SearchWeb'/>"
            "<separator/>"
            "<menuitem action='SearchNextArticle' />"
            "<separator/>"
            "<menuitem action='SearchTitle' />"
            "<separator/>"
            "<menuitem action='SearchCacheLocal'/>"
            "<menu action='SearchCacheAll'>"
            "<menuitem action='ExecSearchCacheAll'/>"
            "</menu>"
            "</menu>"
            ;

            // NGワード
        case ITEM_NGWORD:
            return
            "<menu action='AboneWord_Menu'>"
            "<menuitem action='AboneWord'/>"
            "<menu action='AboneWordBoard'>"
            "<menuitem action='SetAboneWordBoard'/>"
            "</menu>"
            "<menu action='GlobalAboneWord'>"
            "<menuitem action='SetGlobalAboneWord'/>"
            "</menu>"
            "</menu>"
            ;

            // 選択範囲のレスをあぼーん
        case ITEM_ABONE_SELECTION:
            return
            "<menuitem action='AboneSelectionRes' />"
            ;

            // 引用してレス
        case ITEM_QUOTE_SELECTION:
            return "<menuitem action='QuoteSelectionRes' />";

            // リンクをブラウザで開く
        case ITEM_OPEN_BROWSER:
            return "<menuitem action='OpenBrowser'/>";

            // ユーザコマンド
        case ITEM_USER_COMMAND:
            return m_usrcmd.c_str();

            // リンクのURLをコピー
        case ITEM_COPY_URL:
            return "<menuitem action='CopyURL'/>";

            // スレのタイトルとURLをコピー
        case ITEM_COPY_TITLE_URL_THREAD:
            return "<menuitem action='CopyTitleURL'/>";

            // コピー
        case ITEM_COPY:
            return "<menuitem action='Copy'/>";

            // 再読み込み
        case ITEM_RELOAD:
            return "<menuitem action='Reload'/>";

            // dat 保存
        case ITEM_SAVE_DAT:
            return "<menuitem action='SaveDat'/>";

            // スレ情報の引き継ぎ
        case ITEM_COPY_THREAD_INFO:
            return "<menuitem action='CopyInfo'/>";

            // お気に入り
        case ITEM_APPENDFAVORITE:
            return "<menuitem action='AppendFavorite'/>";

            // プロパティ
        case ITEM_PREF_THREAD:
            return "<menuitem action='PreferenceArticle'/>";

            // 選択範囲の画像を開く
        case ITEM_SELECTIMG:
            return "<menuitem action='ShowSelectImage'/>";

            // 選択範囲の画像を削除
        case ITEM_SELECTDELIMG:
            return "<menu action='DeleteSelectImage_Menu'>"
            "<menuitem action='DeleteSelectImage'/>"
            "</menu>";

            // 選択範囲の画像をあぼーん
        case ITEM_SELECTABONEIMG:
            return "<menu action='AboneSelectImage_Menu'>"
            "<menuitem action='AboneSelectImage'/>"
            "</menu>";

            // 区切り
        case ITEM_SEPARATOR:
            return "<separator/>";

            // 削除
        case ITEM_DELETE:
            return "<menu action='Delete_Menu'>"
                "<menuitem action='Delete'/>"
                "<menuitem action='DeleteOpen'/>"
                "</menu>";
    }

    return "";
}


/**
 * @brief ポップアップメニューを表示する前にアクションの状態を更新する
 *
 * @see SKELETON::View::show_popupmenu()
 */
void ARTICLE::ArticleViewBase::activate_act_before_popupmenu( const std::string& url )
{
#ifdef _DEBUG
    std::cout << "ArticleViewBase::activate_act_before_popupmenu url = " << url << std::endl;
#endif

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // 子ポップアップが表示されていて、かつポインタがその上だったら表示しない
    ArticleViewBase* popup_article = nullptr;
    if( is_popup_shown() ) popup_article = dynamic_cast< ArticleViewBase* >( m_popup_win->view() );
    if( popup_article && popup_article->is_mouse_on_view() ) return;
    hide_popup();

    Glib::RefPtr< Gtk::Action > act, act2;
    act = action_group()->get_action( "CopyURL" );
    act2 = action_group()->get_action( "OpenBrowser" );

    // url がセットされてない
    if( url.empty() ) {
        if( act ) act->set_sensitive( false );
        if( act2 ) act2->set_sensitive( false );
        m_url_tmp = std::string();
    }

    // url がセットされている
    else {

        if( act ) act->set_sensitive( true );
        if( act2 ) act2->set_sensitive( true );

        // レス番号クリックの場合
        if( url.rfind( PROTO_RES, 0 ) == 0 ){
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_RES ) ).c_str() ), 0 );
        }

        // アンカークリックの場合
        else if( url.rfind( PROTO_ANCHORE, 0 ) == 0 ){
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_ANCHORE ) ).c_str() ), 0 );
        }

        else m_url_tmp = url;
    }

    // 検索ビューや書き込みログ表示などの場合
    const bool nourl = DBTREE::url_readcgi( m_url_article, 0, 0 ).empty();

    act = action_group()->get_action( "Drawout_Menu" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchCacheLocal" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchCacheAll" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchNextArticle" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "QuoteRes" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SaveDat" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "PreferenceArticle" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }


    // 範囲選択されてない
    const unsigned int max_selection_str = 1024;
    const unsigned int max_selection_str_quote = 8192;

    std::string str_select = m_drawarea->str_selection();
    act = action_group()->get_action( "QuoteSelectionRes" );
    if( act ){
        if( nourl || str_select.empty() || str_select.length() > max_selection_str_quote ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "Copy" );
    if( act ){
        if( str_select.empty() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "DrawoutWord" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "AboneWord_Menu" );
    if( act ){
        if( nourl || str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "AboneSelectionRes" );
    if( act ){
        if( nourl || str_select.empty() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "ShowSelectImage" );
    if( act ){
        if( str_select.empty() || ! m_drawarea->get_selection_imgurls().size() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "DeleteSelectImage_Menu" );
    if( act ){
        if( str_select.empty() || ! m_drawarea->get_selection_imgurls().size() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "AboneSelectImage_Menu" );
    if( act ){
        if( str_select.empty() || ! m_drawarea->get_selection_imgurls().size() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // 検索関係
    act = action_group()->get_action( "SearchWeb" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchCacheLocal" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchCacheAll" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    act = action_group()->get_action( "SearchTitle" );
    if( act ){
        if( str_select.empty() || str_select.length() > max_selection_str ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // ユーザコマンド
    // 選択不可かどうか判断して visible か sensitive にする
    CORE::get_usrcmd_manager()->toggle_sensitive( action_group(), m_url_article, url, str_select );

    // ブックマークがセットされていない
    act = action_group()->get_action( "DrawoutBM" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }
    act = action_group()->get_action( "PreBookMark" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }
    act = action_group()->get_action( "NextBookMark" );
    if( act ){
        if( m_article->get_num_bookmark() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 書き込みしていない
    act = action_group()->get_action( "DrawoutPost" );
    if( act ){
        if( m_article->get_num_posted() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    act = action_group()->get_action( "PrePost" );
    if( act ){
        if( m_article->get_num_posted() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    act = action_group()->get_action( "NextPost" );
    if( act ){
        if( m_article->get_num_posted() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 高参照レス抽出
    act = action_group()->get_action( "DrawoutHighRefRes" );
    if( act ){
        if( nourl ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // 新着移動
    act = action_group()->get_action( "GotoNew" );
    if( act ){
        if( m_article->get_number_new() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 進む、戻る
    act = action_group()->get_action( "PrevView" );
    if( act ){
        if( HISTORY::get_history_manager()->can_back_viewhistory( get_url(), 1 ) ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    act = action_group()->get_action( "NextView" );
    if( act ){
        if( HISTORY::get_history_manager()->can_forward_viewhistory( get_url(), 1 ) ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 透明あぼーん
    act = action_group()->get_action( "TranspAbone" );
    if( act ){

        Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
        if( m_article->get_abone_transparent() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 透明/連鎖あぼーん
    act = action_group()->get_action( "TranspChainAbone" );
    if( act ){

        Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
        if( m_article->get_abone_transparent() && m_article->get_abone_chain() ) tact->set_active( true );
        else tact->set_active( false );
    }


    // 画像
    if( ! url.empty() && DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ){

        // モザイク解除
        act = action_group()->get_action( "Cancel_Mosaic" );
        if( act ){
            if( DBIMG::is_cached( url ) && DBIMG::get_mosaic( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // モザイクで開く
        act = action_group()->get_action( "Show_Mosaic" );
        if( act ){
            if( ! DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // サイズの大きい画像を表示
        act = action_group()->get_action( "ShowLargeImg" );
        if( act ){
            if( DBIMG::get_type_real( url ) == DBIMG::T_LARGE ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // 保護のトグル切替え
        act = action_group()->get_action( "ProtectImage" );
        if( act ){

            if( DBIMG::is_cached( url ) ){

                act->set_sensitive( true );

                Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
                if( DBIMG::is_protected( url ) ) tact->set_active( true );
                else tact->set_active( false );
            }
            else act->set_sensitive( false );
        }

        // 削除
        act = action_group()->get_action( "DeleteImage_Menu" );
        if( act ){

            if( DBIMG::get_code( url ) != HTTP_INIT && ! DBIMG::is_protected( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // 保存
        act = action_group()->get_action( "SaveImage" );
        if( act ){

            if(  DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // プロパティ
        act = action_group()->get_action( "PreferenceImage" );
        if( act ){

            if(  DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }

        // あぼーん
        act = action_group()->get_action( "AboneImage" );
        if( act ){

            if( DBIMG::is_protected( url ) ) act->set_sensitive( false );
            else{

                act->set_sensitive( true );

                Glib::RefPtr< Gtk::ToggleAction > tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
                if( DBIMG::get_abone( url ) ) tact->set_active( true );
                else tact->set_active( false );
            }
        }

        // キャッシュをブラウザで開く
        act = action_group()->get_action( "OpenCacheBrowser" );
        if( act ){
            if( DBIMG::is_cached( url ) ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }
    }

    // スレ情報の引き継ぎ
    act = action_group()->get_action( "CopyInfo" );
    if( act ){

        if( ! url.empty() && ! DBTREE::url_dat( url ).empty() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    m_enable_menuslot = true;
}


/**
 * @brief ポップアップメニューを取得する
 *
 * @see SKELETON::View::show_popupmenu()
 */
Gtk::Menu* ARTICLE::ArticleViewBase::get_popupmenu( const std::string& url )
{
    // 表示
    Gtk::Menu* popupmenu = nullptr;

    // 削除サブメニュー
    if( url == "popup_menu_delete" ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_delete" ) );
    }

    // レス番号ポップアップメニュー
    else if( url.rfind( PROTO_RES, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_res" ) );
    }

    //　アンカーポップアップメニュー
    else if( url.rfind( PROTO_ANCHORE, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_anc" ) );
    }

    // IDポップアップメニュー
    else if( url.rfind( PROTO_ID, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_id" ) );
    }

    // 名前ポップアップメニュー
    else if( url.rfind( PROTO_NAME, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_name" ) );
    }

    // あぼーんポップアップメニュー
    else if( url.rfind( PROTO_ABONE, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_abone" ) );
    }

    // 壊れていますポップアップメニュー
    else if( url.rfind( PROTO_BROKEN, 0 ) == 0 ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_broken" ) );
    }

    // 画像ポップアップメニュー
    else if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ){
        popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu_img" ) );
    }

    // 通常メニュー
    else popupmenu = dynamic_cast< Gtk::Menu* >( ui_manager()->get_widget( "/popup_menu" ) );

    return popupmenu;
}
