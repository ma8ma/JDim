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
    m_action_group->add_action( "OpenBrowserRes",  // レスをクリックした時のメニュー用
                                sigc::mem_fun( *this, &ArticleViewBase::slot_open_browser ) );
    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &ArticleViewBase::slot_copy_current_url ) );
    m_action_group->add_action( "WriteRes",sigc::mem_fun( *this, &ArticleViewBase::slot_write_res ) );
    m_action_group->add_action( "QuoteRes",sigc::mem_fun( *this, &ArticleViewBase::slot_quote_res ) );
    m_action_group->add_action( "CopyRes", sigc::bind<bool>( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), false ) );
    m_action_group->add_action( "CopyResRef", sigc::bind<bool>( sigc::mem_fun( *this, &ArticleViewBase::slot_copy_res ), true ) );
    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &ArticleViewBase::exec_delete ) );
    m_action_group->add_action( "DeleteOpen", sigc::mem_fun( *this, &ArticleViewBase::delete_open_view ) );

    // 検索

    // 抽出系
    m_action_group->add_action( "Drawout_Menu" );
    m_action_group->add_action( "DrawoutRes", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_res ) );
    m_action_group->add_action( "DrawoutRefer", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_refer ) );
    m_action_group->add_action( "DrawoutAround", sigc::mem_fun( *this, &ArticleViewBase::slot_drawout_around ) );

    // あぼーん系
    m_action_group->add_action( "AboneRes", sigc::mem_fun( *this, &ArticleViewBase::slot_abone_res ) );

    // 移動系
    m_action_group->add_action( "Jump", sigc::mem_fun( *this, &ArticleViewBase::slot_jump ) );

    // 画像系

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

    // IDをクリックしたときのメニュー

    // 名前をクリックしたときのメニュー

    // あぼーんをクリックしたときのメニュー

    // 画像メニュー

    // 通常メニュー

    // ポップアップメニューにショートカットキーやマウスジェスチャを表示
}


/**
 * @brief 通常の右クリックメニューを作成する
 */
std::string ARTICLE::ArticleViewBase::create_context_menu() const
{
    // TODO: GTK4 メニュー項目のカスタマイズは現段階では省略します。GTKMM4 版をマージ完了後に対応します。
    return std::string{};
}


/**
 * @brief メニュー項目に対応するXMLを取得する
 *
 * @param item メニュー項目
 * @return メニュー項目のXML, 未実装のため nullptr
 */
const char* ARTICLE::ArticleViewBase::get_menu_item( [[maybe_unused]] const int item ) const
{
    // TODO: GTK4 メニュー項目のカスタマイズは現段階では省略します。GTKMM4 版をマージ完了後に対応します。
    return nullptr;
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

    // url がセットされてない
    if( url.empty() ) {
        if( copy_url_act ) {
            copy_url_act->set_enabled( false );
        }
        m_url_tmp.clear();
    }

    // url がセットされている
    else {

        if( copy_url_act ) {
            copy_url_act->set_enabled( true );
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

    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "QuoteRes" ) ) ) {
        act->set_enabled( ! nourl );
    }

    // 範囲選択されてない

    // 検索関係

    // TODO: GTK4 ユーザーコマンドは現段階では省略します。GTKMM4 版をマージ完了後に対応します。

    // ブックマークがセットされていない

    // 書き込みしていない

    // 高参照レス抽出

    // 新着移動

    // 進む、戻る

    // 透明あぼーん

    // 透明/連鎖あぼーん

    // 画像

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

    // IDポップアップメニュー

    // 名前ポップアップメニュー

    // あぼーんポップアップメニュー

    // 壊れていますポップアップメニュー
    else if( url.starts_with( PROTO_BROKEN ) ) {
        popupmenu = &m_popup_menu_broken;
    }

    // 画像ポップアップメニュー

    // 通常メニュー

    return popupmenu;
}
