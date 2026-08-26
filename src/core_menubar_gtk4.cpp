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
    auto top_menu = Gtk::make_managed<Gtk::MenuItem>( "ツール(_T)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    auto item = Gtk::make_managed<Gtk::MenuItem>( "実況開始／停止(_L)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_live_start_stop ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    // FIXME: GTK4 スレタイ検索用のメニュー項目は変更可能だが現時点では固定する。
    item = Gtk::make_managed<Gtk::MenuItem>( "スレタイ検索 (ff5ch.syoboi.jp)(_T)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_search_title ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto cache_search_item = Gtk::make_managed<Gtk::MenuItem>( "キャッシュ内ログ検索(_C)", true );
        auto cache_search_submenu = Gtk::make_managed<Gtk::Menu>();

        item = Gtk::make_managed<Gtk::MenuItem>( "表示中の板のログを検索(_B)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_search_cache_board ) );
        cache_search_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "キャッシュ内の全ログを検索(_A)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_search_cache ) );
        cache_search_submenu->append( *item );

        cache_search_item->set_submenu( *cache_search_submenu );
        submenu->append( *cache_search_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto cache_list_item = Gtk::make_managed<Gtk::MenuItem>( "キャッシュ内ログ一覧(_H)", true );
        auto cache_list_submenu = Gtk::make_managed<Gtk::Menu>();

        item = Gtk::make_managed<Gtk::MenuItem>( "表示中の板のログをスレ一覧に表示(_B)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_cache_board ) );
        cache_list_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "キャッシュ内の全ログをスレ一覧に表示(_A)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_cache ) );
        cache_list_submenu->append( *item );

        cache_list_item->set_submenu( *cache_list_submenu );
        submenu->append( *cache_list_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto update_check_item = Gtk::make_managed<Gtk::MenuItem>( "サイドバーの更新チェック(_U)", true );
        auto update_check_submenu = Gtk::make_managed<Gtk::Menu>();

        item = Gtk::make_managed<Gtk::MenuItem>( "更新チェックのみ(_R)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_check_update_root ) );
        update_check_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "更新されたスレをタブで開く(_T)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_check_update_open_root ) );
        update_check_submenu->append( *item );

        update_check_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        item = Gtk::make_managed<Gtk::MenuItem>( "キャンセル(_C)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_cancel_check_update ) );
        update_check_submenu->append( *item );

        update_check_item->set_submenu( *update_check_submenu );
        submenu->append( *update_check_item );
    }

    item = Gtk::make_managed<Gtk::MenuItem>( "サイドバーをスレ一覧に表示(_B)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_sidebarboard ) );
    submenu->append( *item );

    item = Gtk::make_managed<Gtk::MenuItem>( "サイドバーの仮想板を作成(_V)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_create_vboard ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "お気に入りの編集(_E)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_edit_favorite ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "書き込みログの表示(_P)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_show_postlog ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    item = Gtk::make_managed<Gtk::MenuItem>( "表示中の板にdatをインポート(_I)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_import_dat ) );
    submenu->append( *item );

    top_menu->set_submenu( *submenu );
    return top_menu;
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
