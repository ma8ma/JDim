// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "core.h"
#include "maintoolbar.h"
#include "command.h"
#include "winmain.h"
#include "session.h"
#include "global.h"
#include "type.h"
#include "dndmanager.h"
#include "usrcmdmanager.h"
#include "linkfiltermanager.h"
#include "replacestrmanager.h"
#include "compmanager.h"
#include "searchmanager.h"
#include "aamanager.h"
#include "dispatchmanager.h"
#include "cssmanager.h"
#include "updatemanager.h"
#include "login2ch.h"
#include "loginbe.h"
#include "loginacorn.h"
#include "environment.h"
#include "setupwizard.h"
#include "cache.h"
#include "sharedbuffer.h"

#include "control/controlutil.h"
#include "control/controlid.h"

#include "history/historymanager.h"

#include "skeleton/msgdiag.h"

#include "config/globalconf.h"
#include "config/defaultconf.h"

#include "jdlib/cookiemanager.h"
#include "jdlib/miscutil.h"
#include "jdlib/miscgtk.h"
#include "jdlib/misctime.h"
#include "jdlib/loader.h"
#include "jdlib/timeout.h"

#include "dbtree/interface.h"
#include "dbimg/imginterface.h"

#include "bbslist/bbslistadmin.h"
#include "board/boardadmin.h"
#include "article/articleadmin.h"
#include "image/imageadmin.h"
#include "message/messageadmin.h"

#include "message/logmanager.h"

#include "sound/soundmanager.h"

#include <algorithm>


Gtk::MenuItem* CORE::Core::create_file_menu()
{
    auto top_item = Gtk::make_managed<Gtk::MenuItem>( "ファイル(_F)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    auto item = Gtk::make_managed<Gtk::MenuItem>( "URLを開く(_U)...", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_openurl ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    // FIXME: GTK4 CheckMenuItem の active 状態の切り替えは Core::slot_activate_menubar() の中で設定を参照して行う。
    // 現状では CheckMenuItem とアプリケーションの状態は同期していない。
    auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "2chにログイン(_L)", true );
    check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_login2ch ) );
    submenu->append( *check_item );

    check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "BEにログイン(_B)", true );
    check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_loginbe ) );
    submenu->append( *check_item );

    check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "どんぐりシステムにGmail警備員●でログイン(_G)", true );
    check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_loginacorn ) );
    submenu->append( *check_item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "セッション保存(_S)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::save_session ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "板一覧再読込(_R)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_reload_list ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "オフライン作業(_W)", true );
    check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_online ) );
    submenu->append( *check_item );

    // TODO: GTK4 CONTROL::JDExit に紐づけされている Gtk::AccelKey を取得してショートカットキーの登録を行う。
    item = Gtk::make_managed<Gtk::MenuItem>( "終了(_Q)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_quit ) );
    submenu->append( *item );

    top_item->set_submenu( *submenu );
    return top_item;
}

Gtk::MenuItem* CORE::Core::create_view_menu()
{
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "表示(_V)", true );
    menu->set_sensitive( false );
    return menu;
}

Gtk::MenuItem* CORE::Core::create_history_menu()
{
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "履歴(_S)", true );
    menu->set_sensitive( false );
    return menu;
}

Gtk::MenuItem* CORE::Core::create_tool_menu()
{
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "ツール(_T)", true );
    menu->set_sensitive( false );
    return menu;
}

Gtk::MenuItem* CORE::Core::create_setting_menu()
{
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "設定(_C)", true );
    menu->set_sensitive( false );
    return menu;
}

Gtk::MenuItem* CORE::Core::create_help_menu()
{
    auto top_item = Gtk::make_managed<Gtk::MenuItem>( "ヘルプ(_H)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    auto item = Gtk::make_managed<Gtk::MenuItem>( "オンラインマニュアル(_M)...", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_manual ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "JD サポート掲示板(_B)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_bbs ) );
    submenu->append( *item );

    item = Gtk::make_managed<Gtk::MenuItem>( "2chスレ過去ログ(_L)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_old2ch ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "JDimについて(_A)...", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_about ) );
    submenu->append( *item );

    top_item->set_submenu( *submenu );
    return top_item;
}


/**
 * @brief メインメニューの設定
 */
void CORE::Core::setup_menubar()
{
    m_menubar = Gtk::make_managed<Gtk::MenuBar>();

    m_menubar->append( *create_file_menu() );
    m_menubar->append( *create_view_menu() );
    m_menubar->append( *create_history_menu() );
    m_menubar->append( *create_tool_menu() );
    m_menubar->append( *create_setting_menu() );
    m_menubar->append( *create_help_menu() );
}
