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
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "ファイル(_F)", true );
    menu->set_sensitive( false );
    return menu;
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
    auto menu = Gtk::make_managed<Gtk::MenuItem>( "ヘルプ(_H)", true );
    menu->set_sensitive( false );
    return menu;
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
