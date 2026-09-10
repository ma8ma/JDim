// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageviewbase.h"
#include "imageareabase.h"

#include "skeleton/msgdiag.h"

#include "dbtree/interface.h"

#include "dbimg/imginterface.h"
#include "dbimg/img.h"

#include "jdlib/miscgtk.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "config/globalconf.h"

#include "cache.h"
#include "command.h"
#include "sharedbuffer.h"
#include "global.h"
#include "type.h"
#include "prefdiagfactory.h"
#include "usrcmdmanager.h"
#include "httpcode.h"
#include "session.h"

#include <sstream>


/**
 * @brief ImageViewBase のコンテキストメニューを構築する。
 *
 * ImageViewBase::setup_common() から呼び出され、コンテキストメニューの生成と
 * アクションの登録を行います。
 */
void IMAGE::ImageViewBase::setup_popupmenu()
{
    // TODO: Action を構築する
    m_action_group = Gio::SimpleActionGroup::create();

    // popup_menu_icon
    m_action_group->add_action( "Move_Menu" );
    m_action_group->add_action( "MoveHead", sigc::mem_fun( *this, &ImageViewBase::slot_move_head ) );
    m_action_group->add_action( "MoveTail", sigc::mem_fun( *this, &ImageViewBase::slot_move_tail ) );
    m_action_group->add_action_bool( "LockTab", sigc::mem_fun( *this, &ImageViewBase::slot_lock ), false );
    m_action_group->add_action( "AppendFavorite", sigc::mem_fun( *this, &ImageViewBase::slot_favorite ) );
    m_action_group->add_action( "Close_Menu" );
    m_action_group->add_action( "CloseOther", sigc::mem_fun( *this, &ImageViewBase::slot_close_other_views ) );
    m_action_group->add_action( "CloseLeft", sigc::mem_fun( *this, &ImageViewBase::slot_close_left_views ) );
    m_action_group->add_action( "CloseRight", sigc::mem_fun( *this, &ImageViewBase::slot_close_right_views ) );
    m_action_group->add_action( "CloseError404", sigc::mem_fun( *this, &ImageViewBase::slot_close_error_views ) );
    m_action_group->add_action( "CloseError503", sigc::mem_fun( *this, &ImageViewBase::slot_close_notimeout_error_views ) );
    m_action_group->add_action( "CloseErrorAll", sigc::mem_fun( *this, &ImageViewBase::slot_close_all_error_views ) );
    m_action_group->add_action( "CloseNoError", sigc::mem_fun( *this, &ImageViewBase::slot_close_noerror_views ) );
    m_action_group->add_action( "CloseAll", sigc::mem_fun( *this, &ImageViewBase::slot_close_all_views ) );
    m_action_group->add_action( "SaveAll", sigc::mem_fun( *this, &ImageViewBase::slot_save_all ) );
    m_action_group->add_action( "OpenBrowser", sigc::mem_fun( *this, &ImageViewBase::slot_open_browser ) );
    m_action_group->add_action( "OpenCacheBrowser", sigc::mem_fun( *this, &ImageViewBase::slot_open_cache_browser ) );
    m_action_group->add_action( "OpenRef", sigc::mem_fun( *this, &ImageViewBase::slot_open_ref ) );
    m_action_group->add_action( "LoadStop", sigc::mem_fun( *this, &ImageViewBase::stop ) );
    m_action_group->add_action( "Reload", sigc::mem_fun( *this, &ImageViewBase::slot_reload_force ) );

    // popup_menu_popup
    m_action_group->add_action( "CancelMosaic", sigc::mem_fun( *this, &ImageViewBase::slot_cancel_mosaic ) );
    m_action_group->add_action( "Quit", sigc::mem_fun( *this, &ImageViewBase::close_view ) );
    m_action_group->add_action( "CopyURL", sigc::mem_fun( *this, &ImageViewBase::slot_copy_url ) );
    m_action_group->add_action( "Save", sigc::mem_fun( *this, &ImageViewBase::slot_save ) );
    m_action_group->add_action_bool( "ProtectImage", sigc::mem_fun( *this, &ImageViewBase::slot_toggle_protectimage ), false );
    m_action_group->add_action( "DeleteMenu" );
    m_action_group->add_action( "DeleteImage", sigc::mem_fun( *this, &ImageViewBase::delete_view ) );
    m_action_group->add_action( "AboneImage", sigc::mem_fun( *this, &ImageViewBase::slot_abone_img ) );
    m_action_group->add_action( "Preference", sigc::mem_fun( *this, &ImageViewBase::show_preference ) );

    insert_action_group( "image", m_action_group );

    // TODO: メニューを構築する

    // UI 定義: src/ui/imageview_menu.ui
    // リソース URI: /com/github/jdimproved/JDim/imageview_menu.ui (src/ui/jdim-ui.gresource.xml)
    auto builder = Gtk::Builder::create_from_resource( "/com/github/jdimproved/JDim/imageview_menu.ui" );

    auto menumodel = Glib::RefPtr<Gio::MenuModel>::cast_dynamic( builder->get_object( "popup_menu_icon" ) );
    assert( menumodel );
    m_popup_menu_icon.bind_model( menumodel, true );
    m_popup_menu_icon.attach_to_widget( *this );

    menumodel = Glib::RefPtr<Gio::MenuModel>::cast_dynamic( builder->get_object( "popup_menu_popup" ) );
    assert( menumodel );
    m_popup_menu_popup.bind_model( menumodel, true );
    m_popup_menu_popup.attach_to_widget( *this );
}


/**
 * @brief ポップアップメニューを表示する直前にメニュー項目（アクション）のアクティブ状態を更新する。
 *
 * ポップアップメニューを表示する直前に呼び出され、
 * 現在の画像状態に応じて ToggleAction や Action の状態を切り替える。
 *
 * @note SKELETON::View::show_popupmenu() から呼び出されます。
 * @param[in] url 対象の画像URL
 */
void IMAGE::ImageViewBase::activate_act_before_popupmenu( [[maybe_unused]] const std::string& url )
{
    if( !m_img ) return;

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    const bool current_protect = m_img->is_protected();

    // ロック
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "LockTab" ) ) ) {
        act->set_state( Glib::Variant<bool>::create( is_locked() ) );
    }

    // 閉じる
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "Quit" ) ) ) {
        act->set_enabled( ! is_locked() );
    }

    // モザイク
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "CancelMosaic" ) ) ) {
        act->set_enabled( m_img->is_cached() && m_img->get_mosaic() );
    }

    // TODO: Action の状態を切り替える
    // サイズの大きい画像を表示
    // サイズ系メニュー、お気に入り、保存

    // キャッシュをブラウザで開く
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "OpenCacheBrowser" ) ) ) {
        act->set_enabled( m_img->is_cached() );
    }

    // 参照元スレ
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "OpenRef" ) ) ) {
        act->set_enabled( ! m_img->get_refurl().empty() );
    }

    // 保護
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "ProtectImage" ) ) ) {

        if( m_img->is_cached() ){

            act->set_enabled( true );
            act->set_state( Glib::Variant<bool>::create( current_protect ) );
        }
        else act->set_enabled( false );
    }

    // 削除
    // TODO: GTK4 GMenu ではサブメニュー親の sensitive が Action に連動しないため、
    // 「削除」サブメニュー自体は無効化せず、子の DeleteImage だけ enabled を落とします。
    // サブメニュー親の無効化をどうするかは後続のフェーズで決めます。
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "DeleteImage" ) ) ) {
        act->set_enabled(  m_img->get_code() != HTTP_INIT && ! m_img->is_protected() );
    }

    // ロード停止
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "LoadStop" ) ) ) {
        act->set_enabled( m_img->is_loading() );
    }

    // あぼーん
    if( auto act = Glib::RefPtr<Gio::SimpleAction>::cast_dynamic( m_action_group->lookup_action( "AboneImage" ) ) ) {
        act->set_enabled( ! m_img->is_protected() );
    }

    // TODO: Action の状態を切り替える
    // ユーザコマンド
    // 選択不可かどうか判断して visible か sensitive にする

    m_enable_menuslot = true;
}


/**
 * @brief メニューのパス文字列から Gtk::Menu* を取得する。
 *
 * @param[in] menu_name ポップアップメニューの識別パス (例: "/popup_menu")
 * @return menu_name に関連付けされたコンテキストメニュー。見つからない場合は nullptr を返す。
 */
Gtk::Menu* IMAGE::ImageViewBase::get_popupmenu_impl( const Glib::ustring& menu_name )
{
    // TODO: 構築したメニューを返す
    if( menu_name == "/popup_menu_icon" ) {
        return &m_popup_menu_icon;
    }
    if( menu_name == "/popup_menu_popup" ) {
        return &m_popup_menu_popup;
    }
    return nullptr;
}
