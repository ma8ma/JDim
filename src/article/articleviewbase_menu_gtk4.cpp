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
    // アクショングループを作って登録
    m_action_group = Gio::SimpleActionGroup::create();

    m_action_group->add_action( "BookMark", sigc::mem_fun( *this, &ArticleViewBase::slot_bookmark ) );
    m_action_group->add_action( "PostedMark", sigc::mem_fun( *this, &ArticleViewBase::slot_postedmark ) );
    m_action_group->add_action( "OpenBrowser", sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    m_action_group->add_action( "OpenBrowserRes",  // レスをクリックした時のメニュー用
                                sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    m_action_group->add_action( "OpenCacheBrowser", sigc::mem_fun( *this, &ArticleViewBase::slot_open_cache_browser ) );
    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &ArticleViewBase::slot_copy_current_url ) );
    m_action_group->add_action( "CopyNAME", sigc::mem_fun( *this, &ArticleViewBase::slot_copy_name ) );
    m_action_group->add_action( "CopyID", sigc::mem_fun( *this, &ArticleViewBase::slot_copy_id ) );
    m_action_group->add_action( "WriteRes",sigc::mem_fun( *this, &ArticleViewBase::slot_write_res ) );
    m_action_group->add_action( "QuoteRes",sigc::mem_fun( *this, &ArticleViewBase::slot_quote_res ) );
    m_action_group->add_action( "CopyRes", sigc::bind<bool>( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), false ) );
    m_action_group->add_action( "CopyResRef", sigc::bind<bool>( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), true ) );
    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &ArticleViewBase::exec_delete ) );
    m_action_group->add_action( "DeleteOpen", sigc::mem_fun( *this, &ArticleViewBase::delete_open_view ) );
    m_action_group->add_action( "PreferenceImage", sigc::mem_fun( *this, &ArticleViewBase::slot_preferences_image ) );

    // 検索

    // 抽出系
    m_action_group->add_action( "Drawout_Menu" );
    m_action_group->add_action( "DrawoutRes", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_res ) );
    m_action_group->add_action( "DrawoutNAME", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_name ) );
    m_action_group->add_action( "DrawoutID", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_id ) );
    m_action_group->add_action( "DrawoutRefer", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_refer ) );
    m_action_group->add_action( "DrawoutAround", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_around ) );

    // あぼーん系
    m_action_group->add_action( "AboneRes", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_res ) );
    m_action_group->add_action( "AboneID", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_id ) );
    m_action_group->add_action( "AboneName", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name ) );

    m_action_group->add_action( "AboneNameBoard" );
    m_action_group->add_action( "SetAboneNameBoard", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_name_board ) );

    m_action_group->add_action( "GlobalAboneName" );
    m_action_group->add_action( "SetGlobalAboneName", sigc::mem_fun( *this, &ArticleViewBase::slot_global_abone_name ) );

    m_action_group->add_action_bool( "TranspAbone", sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp ), false );
    m_action_group->add_action_bool( "TranspChainAbone", sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_abone_transp_chain ), false );

    m_action_group->add_action( "SetupAbone", sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone ) );
    m_action_group->add_action( "SetupAboneBoard", sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone_board ) );
    m_action_group->add_action( "SetupAboneAll", sigc::mem_fun( *this, &ArticleViewBase::slot_setup_abone_all ) );

    // 移動系
    m_action_group->add_action( "Jump", sigc::mem_fun( *this, &ArticleViewBase::slot_jump ) );

    // 画像系
    m_action_group->add_action( "Cancel_Mosaic", sigc::mem_fun( *this, &ArticleViewBase::slot_cancel_mosaic ) );
    m_action_group->add_action( "Show_Mosaic", sigc::mem_fun( *this, &ArticleViewBase::slot_show_image_with_mosaic ) );
    m_action_group->add_action( "ShowLargeImg", sigc::mem_fun( *this, &ArticleViewBase::slot_show_large_img ) );
    m_action_group->add_action_bool( "ProtectImage", sigc::mem_fun( *this, &ArticleViewBase::slot_toggle_protectimage ), false );
    m_action_group->add_action( "DeleteImage_Menu" );
    m_action_group->add_action( "DeleteImage", sigc::mem_fun( *this, &ArticleViewBase::slot_deleteimage ) );
    m_action_group->add_action( "SaveImage", sigc::mem_fun( *this, &ArticleViewBase::slot_saveimage ) );
    m_action_group->add_action_bool( "AboneImage", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_img ), false );

    // その他

    // TODO: GTK4 ユーザーコマンドは現段階では省略します。GTKMM4 版をマージ完了後に対応します。

    insert_action_group( "article", m_action_group );

    // Gio::Menu を Gtk::Menu にバインドする
    // UI 定義: src/ui/articleview_menu.ui
    // リソース URI: /com/github/jdimproved/JDim/articleview_menu.ui (src/ui/jdim-ui.gresource.xml)
    auto builder = Gtk::Builder::create_from_resource( "/com/github/jdimproved/JDim/articleview_menu.ui" );

    auto bind_menumodel = [this, &builder]( Gtk::Menu& menu, const Glib::ustring& menu_id ) {
        auto menumodel = builder->get_object( menu_id );
        assert( menumodel );
        menu.bind_model( Glib::RefPtr<Gio::MenuModel>::cast_dynamic( menumodel ), true );
        menu.attach_to_widget( *this );
    };

    // 削除ボタン押したときのポップアップ
    bind_menumodel( m_popup_menu_delete, "popup_menu_delete" );

    // 壊れていますをクリックしたときのポップアップ
    bind_menumodel( m_popup_menu_broken, "popup_menu_broken" );

    // レス番号をクリックしたときのメニュー
    bind_menumodel( m_popup_menu_res, "popup_menu_res" );

    // レスアンカーをクリックしたときのメニュー
    bind_menumodel( m_popup_menu_anc, "popup_menu_anc" );

    // IDをクリックしたときのメニュー
    bind_menumodel( m_popup_menu_id, "popup_menu_id" );

    // 名前をクリックしたときのメニュー
    bind_menumodel( m_popup_menu_name, "popup_menu_name" );

    // あぼーんをクリックしたときのメニュー
    bind_menumodel( m_popup_menu_abone, "popup_menu_abone" );

    // 画像メニュー
    bind_menumodel( m_popup_menu_img, "popup_menu_img" );

    // 通常メニュー
    m_popup_menu.bind_model( create_context_menu(), true );
    m_popup_menu.attach_to_widget( *this );

    // ポップアップメニューにショートカットキーやマウスジェスチャを表示
}


/**
 * @brief 通常の右クリックメニューを作成して返す
 *
 * @return セッション情報から作成した Gio::Menu
 */
Glib::RefPtr<Gio::Menu> ARTICLE::ArticleViewBase::create_context_menu() const
{
    std::list<int> etc_menu_items = {
        ITEM_DRAWOUT,
        ITEM_GO,
        ITEM_SEARCH,
        ITEM_NGWORD,
        ITEM_QUOTE_SELECTION,
        ITEM_OPEN_BROWSER,
        ITEM_USER_COMMAND,
        ITEM_COPY_URL,
        ITEM_COPY,
        ITEM_RELOAD,
        ITEM_DELETE,
        ITEM_COPY_TITLE_URL_THREAD,
        ITEM_SAVE_DAT,
        ITEM_COPY_THREAD_INFO,
        ITEM_APPENDFAVORITE,
        ITEM_ABONE_SELECTION,
        ITEM_SELECTIMG,
        ITEM_SELECTDELIMG,
        ITEM_SELECTABONEIMG,
        ITEM_PREF_THREAD,
    };

    // メニューに含まれる項目を取り除いて「その他」サブメニューに含める項目を残す
    int pos = 0;
    for(;; ++pos ) {
        const int item = SESSION::get_item_article_menu( pos );
        if( item == ITEM_END ) break;

        etc_menu_items.remove( item );
    }

    auto menumodel = Gio::Menu::create();
    auto section = Gio::Menu::create();
    pos = 0;
    for(;; ++pos ) {

        const int item = SESSION::get_item_article_menu( pos );

        if( item == ITEM_END ) break;

        else if( item == ITEM_ETC && ! etc_menu_items.empty() ){
            // 「その他」サブメニューを追加する
            auto submenu = Gio::Menu::create();
            for( const int i : etc_menu_items ) {
                add_menu_item( i, submenu );
            }
            section->append_submenu( "その他(_O)", submenu );
        }
        else {
            // item がセパレーターだったらセクションを区切る
            if( add_menu_item( item, section ) ) {
                menumodel->append_section( section );
                section = Gio::Menu::create();
            }
        }
    }

    if( section->get_n_items() > 0 ) {
        menumodel->append_section( section );
    }

    return menumodel;
}


/**
 * @brief メニュー項目に対応する Gio::Menu を追加する
 *
 * @param item メニュー項目
 * @param section item に対応するメニューを追加するセクション
 * @return セクションの区切りなら true を返す
 */
bool ARTICLE::ArticleViewBase::add_menu_item( const int item, const Glib::RefPtr<Gio::Menu>& section ) const
{
    switch( item ) {

        // 抽出
        case ITEM_DRAWOUT:
            {
                auto submenu = Gio::Menu::create();
                submenu->append( "キーワード抽出(_K)", "article.DrawoutWord" );
                submenu->append( "しおり抽出(_B)", "article.DrawoutBM" );
                submenu->append( "書き込み抽出(_W)", "article.DrawoutPost" );
                submenu->append( "高参照レス抽出(_H)", "article.DrawoutHighRefRes" );
                submenu->append( "URL抽出(_U)", "article.DrawoutURL" );
                submenu->append( "テンプレート抽出(_T)", "article.DrawoutTmp" );
                section->append_submenu( "抽出(_E)", submenu );
            }
            return false;

            // 移動
        case ITEM_GO:
            {
                auto submenu = Gio::Menu::create();

                auto subsection = Gio::Menu::create();
                subsection->append( "前へ戻る(_P)", "article.PrevView" );
                subsection->append( "次へ進む(_N)", "article.NextView" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "先頭へ移動(_H)", "article.Home" );
                subsection->append( "最後へ移動(_E)", "article.End" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "新着へ移動(_W)", "article.GotoNew" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "前のしおりヘ移動(_R)", "article.PreBookMark" );
                subsection->append( "次のしおりヘ移動(_X)", "article.NextBookMark" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "前の書き込みヘ移動", "article.PrePost" );
                subsection->append( "次の書き込みヘ移動", "article.NextPost" );
                submenu->append_section( subsection );

                section->append_submenu( "移動(_M)", submenu );
            }
            return false;

            // 検索
        case ITEM_SEARCH:
            {
                auto submenu = Gio::Menu::create();

                auto subsection = Gio::Menu::create();
                // TODO: CONFIG::get_menu_search_web() の設定で項目名を変更できるがフェーズ1では省略します。
                subsection->append( "WEB検索(_W)", "article.SearchWeb" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "次スレ検索(_N)", "article.SearchNextArticle" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                // TODO: CONFIG::get_menu_search_title() の設定で項目名を変更できるがフェーズ1では省略します。
                subsection->append( "スレタイ検索(_T)", "article.SearchTitle" );
                submenu->append_section( subsection );

                subsection = Gio::Menu::create();
                subsection->append( "ログ検索(対象: 板)(_L)", "article.SearchCacheLocal" );
                auto nest_menu = Gio::Menu::create();
                nest_menu->append( "検索する(_E)", "article.ExecSearchCacheAll" );
                subsection->append_submenu( "ログ検索(対象: 全ログ)(_A)", nest_menu );
                submenu->append_section( subsection );

                section->append_submenu( "検索(_H)", submenu );
            }
            return false;

            // NGワード
        case ITEM_NGWORD:
            {
                auto submenu = Gio::Menu::create();
                submenu->append( "NG ワードに追加 (対象: ローカル)(_L)", "article.AboneWord" );

                auto nest_menu = Gio::Menu::create();
                nest_menu->append( "追加する(_A)", "article.SetAboneWordBoard" );
                submenu->append_submenu( "NG ワードに追加 (対象: 板)(_B)", nest_menu );

                nest_menu = Gio::Menu::create();
                nest_menu->append( "追加する(_A)", "article.SetGlobalAboneWord" );
                submenu->append_submenu( "NG ワードに追加 (対象: 全体)(_A)", nest_menu );

                section->append_submenu( "NGワード(_N)", submenu );
            }
            return false;

            // 選択範囲のレスをあぼーん
        case ITEM_ABONE_SELECTION:
            section->append( "選択範囲のレスをあぼ〜ん(_A)", "article.AboneSelectionRes" );
            return false;

            // 引用してレス
        case ITEM_QUOTE_SELECTION:
            section->append( "引用してレスする(_Q)", "article.QuoteSelectionRes" );
            return false;

            // リンクをブラウザで開く
        case ITEM_OPEN_BROWSER:
            section->append( "ブラウザで開く(_W)", "article.OpenBrowser" );
            return false;

            // ユーザコマンド
        case ITEM_USER_COMMAND:
            // TODO: GTK4 ユーザーコマンドは現段階では省略します。GTKMM4 版をマージ完了後に対応します。
            return false;

            // リンクのURLをコピー
        case ITEM_COPY_URL:
            section->append( "URLをコピー(_U)", "article.CopyURL" );
            return false;

            // スレのタイトルとURLをコピー
        case ITEM_COPY_TITLE_URL_THREAD:
            section->append( "スレのタイトルとURLをコピー(_L)", "article.CopyTitleURL" );
            return false;

            // コピー
        case ITEM_COPY:
            section->append( "コピー(_C)", "article.Copy" );
            return false;

            // 再読み込み
        case ITEM_RELOAD:
            section->append( "再読み込み(_R)", "article.Reload");
            return false;

            // dat 保存
        case ITEM_SAVE_DAT:
            section->append( "datを保存(_S)...", "article.SaveDat" );
            return false;

            // スレ情報の引き継ぎ
        case ITEM_COPY_THREAD_INFO:
            section->append( "スレ情報を引き継ぐ(_I)...", "article.CopyInfo" );
            return false;

            // お気に入り
        case ITEM_APPENDFAVORITE:
            section->append( "お気に入りに追加(_F)...", "article.AppendFavorite" );
            return false;

            // プロパティ
        case ITEM_PREF_THREAD:
            section->append( "スレのプロパティ(_P)...", "article.PreferenceArticle" );
            return false;

            // 選択範囲の画像を開く
        case ITEM_SELECTIMG:
            section->append( "選択範囲の画像を開く(_G)", "article.ShowSelectImage" );
            return false;

            // 選択範囲の画像を削除
        case ITEM_SELECTDELIMG:
            {
                auto submenu = Gio::Menu::create();
                submenu->append( "削除する(_D)", "article.DeleteSelectImage" );
                section->append_submenu( "選択範囲の画像を削除(_T)", submenu );
            }
            return false;

            // 選択範囲の画像をあぼーん
        case ITEM_SELECTABONEIMG:
            {
                auto submenu = Gio::Menu::create();
                submenu->append( "あぼ〜んする(_A)", "article.AboneSelectImage" );
                section->append_submenu( "選択範囲の画像をあぼ〜ん(_B)", submenu );
            }
            return false;

            // 区切り
        case ITEM_SEPARATOR:
            return true;

            // 削除
        case ITEM_DELETE:
            {
                auto submenu = Gio::Menu::create();
                submenu->append( "削除(_D)", "article.Delete" );
                submenu->append( "スレ情報を消さずにスレ再取得(_R)", "article.DeleteOpen" );
                section->append_submenu( "削除(_D)", submenu );
            }
            return false;
    }

    return false;
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
    if( is_popup_shown() ) popup_article = dynamic_cast<ArticleViewBase*>( m_popup_win->view() );
    if( popup_article && popup_article->is_mouse_on_view() ) {
        m_enable_menuslot = true;
        return;
    }
    hide_popup();

    // Action の状態を切り替える

    auto copy_url_act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "CopyURL" ) );
    auto open_browser_act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "OpenBrowser" ) );

    // url がセットされてない
    if( url.empty() ) {
        if( copy_url_act ) {
            copy_url_act->set_enabled( false );
        }
        if( open_browser_act ) {
            open_browser_act->set_enabled( false );
        }
        m_url_tmp.clear();
    }

    // url がセットされている
    else {

        if( copy_url_act ) {
            copy_url_act->set_enabled( true );
        }
        if( open_browser_act ) {
            open_browser_act->set_enabled( true );
        }

        // レス番号クリックの場合
        if( url.starts_with( PROTO_RES ) ) {
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_RES ) ).c_str() ), 0 );
        }

        // アンカークリックの場合
        else if( url.starts_with( PROTO_ANCHORE ) ) {
            m_url_tmp = DBTREE::url_readcgi( m_url_article, atoi( url.substr( strlen( PROTO_ANCHORE ) ).c_str() ), 0 );
        }

        else {
            m_url_tmp = url;
        }
    }

    // 検索ビューや書き込みログ表示などの場合
    const bool nourl = DBTREE::url_readcgi( m_url_article, 0, 0 ).empty();

    // Actionの設定を行うヘルパー関数
    auto set_action_enabled = [this]( const char* action_name, bool enabled ) {
        if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( action_name ) ) ) {
            act->set_enabled( enabled );
        }
    };

    set_action_enabled( "QuoteRes", ! nourl );

    // 範囲選択されてない

    // 検索関係

    // TODO: GTK4 ユーザーコマンドは現段階では省略します。GTKMM4 版をマージ完了後に対応します。

    // ブックマークがセットされていない

    // 書き込みしていない

    // 高参照レス抽出

    // 新着移動

    // 進む、戻る

    // 透明あぼーん
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "TranspAbone" ) ) ) {
        act->set_state( Glib::Variant<bool>::create( m_article->get_abone_transparent() ) );
    }

    // 透明/連鎖あぼーん
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "TranspChainAbone" ) ) ) {
        act->set_state( Glib::Variant<bool>::create(
                    m_article->get_abone_transparent() && m_article->get_abone_chain() ) );
    }

    // 画像
    if( ! url.empty() && DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ) {

        const bool has_cache = DBIMG::is_cached( url );

        // モザイク解除
        set_action_enabled( "Cancel_Mosaic", has_cache && DBIMG::get_mosaic( url ) );

        // モザイクで開く
        set_action_enabled( "Show_Mosaic", ! has_cache );

        // サイズの大きい画像を表示
        set_action_enabled( "ShowLargeImg", DBIMG::get_type_real( url ) == DBIMG::T_LARGE );

        // 保護のトグル切替え
        if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "ProtectImage" ) ) ) {
            if( DBIMG::is_cached( url ) ) {

                act->set_enabled( true );
                act->set_state( Glib::Variant<bool>::create( DBIMG::is_protected( url ) ) );
            }
            else act->set_enabled( false );
        }

        // 削除
        // TODO: GTK4 GMenu ではサブメニュー親の sensitive が Action に連動しないため、
        // 「削除」サブメニュー自体は無効化せず、子の DeleteImage だけ enabled を落とします。
        // サブメニュー親の無効化をどうするかは後続のフェーズで決めます。
        set_action_enabled( "DeleteImage", DBIMG::get_code( url ) != HTTP_INIT && ! DBIMG::is_protected( url ) );

        // 保存
        set_action_enabled( "SaveImage", has_cache );

        // プロパティ
        set_action_enabled( "PreferenceImage", has_cache );

        // あぼーん
        if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "AboneImage" ) ) ) {
            if( DBIMG::is_protected( url ) ) {
                act->set_enabled( false );
            }
            else {
                act->set_enabled( true );
                act->set_state( Glib::Variant<bool>::create( DBIMG::get_abone( url ) ) );
            }
        }

        // キャッシュをブラウザで開く
        set_action_enabled( "OpenCacheBrowser", has_cache );
    }

    // スレ情報の引き継ぎ

    m_enable_menuslot = true;
}


/**
 * @brief ポップアップメニューを取得する
 *
 * @see SKELETON::View::show_popupmenu()
 * @return 未実装のため nullptr
 */
Gtk::Menu* ARTICLE::ArticleViewBase::get_popupmenu( const std::string& url )
{
    // 表示
    Gtk::Menu* popupmenu = nullptr;

    // 削除サブメニュー
    if( url == "popup_menu_delete" ) {
        popupmenu = &m_popup_menu_delete;
    }

    // レス番号ポップアップメニュー
    else if( url.starts_with( PROTO_RES ) ) {
        popupmenu = &m_popup_menu_res;
    }

    //　アンカーポップアップメニュー
    else if( url.starts_with( PROTO_ANCHORE ) ) {
        popupmenu = &m_popup_menu_anc;
    }

    // IDポップアップメニュー
    else if( url.starts_with( PROTO_ID ) ) {
        popupmenu = &m_popup_menu_id;
    }

    // 名前ポップアップメニュー
    else if( url.starts_with( PROTO_NAME ) ) {
        popupmenu = &m_popup_menu_name;
    }

    // あぼーんポップアップメニュー
    else if( url.starts_with( PROTO_ABONE ) ) {
        popupmenu = &m_popup_menu_abone;
    }

    // 壊れていますポップアップメニュー
    else if( url.starts_with( PROTO_BROKEN ) ) {
        popupmenu = &m_popup_menu_broken;
    }

    // 画像ポップアップメニュー
    else if( DBIMG::get_type_ext( url ) != DBIMG::T_UNKNOWN ) {
        popupmenu = &m_popup_menu_img;
    }

    // 通常メニュー
    else {
        popupmenu = &m_popup_menu;
    }

    return popupmenu;
}
