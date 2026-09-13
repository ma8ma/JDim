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

    m_action_group->add_action( "Delete", sigc::mem_fun( *this, &ArticleViewBase::exec_delete ) );
    m_action_group->add_action( "DeleteOpen", sigc::mem_fun( *this, &ArticleViewBase::delete_open_view ) );

    // 検索

    // 抽出系

    // あぼーん系

    // 移動系

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

    // レス番号をクリックしたときのメニュー

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
void ARTICLE::ArticleViewBase::activate_act_before_popupmenu( [[maybe_unused]] const std::string& url )
{
    // TODO: maybe_unused は url を使うコードを実装したら取り除く
#ifdef _DEBUG
    std::cout << "ArticleViewBase::activate_act_before_popupmenu url = " << url << std::endl;
#endif
    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // TODO: Action の状態を切り替える

    // 子ポップアップが表示されていて、かつポインタがその上だったら表示しない

    // url がセットされてない

    // url がセットされている

    // 検索ビューや書き込みログ表示などの場合

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

    //　アンカーポップアップメニュー

    // IDポップアップメニュー

    // 名前ポップアップメニュー

    // あぼーんポップアップメニュー

    // 壊れていますポップアップメニュー

    // 画像ポップアップメニュー

    // 通常メニュー

    return popupmenu;
}
