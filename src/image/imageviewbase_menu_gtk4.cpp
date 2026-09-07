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
    // TODO: メニューを構築する
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
    // TODO: maybe_unused は url を使うコードを実装したら取り除く
    if( !m_img ) return;

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // TODO: Action の状態を切り替える

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
    return nullptr;
}
