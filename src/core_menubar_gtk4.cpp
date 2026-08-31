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
    auto top_menu = Gtk::make_managed<Gtk::MenuItem>( "表示(_V)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    // サイドバー
    {
        auto sidebar_item = Gtk::make_managed<Gtk::MenuItem>( "サイドバー(_S)", true );
        auto sidebar_submenu = Gtk::make_managed<Gtk::Menu>();

        auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "板一覧(_B)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_BBSLISTVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_FAVORITEVIEW ) + "(_F)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_FAVORITEVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_HISTVIEW ) + "(_T)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTTHREADVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_HIST_BOARDVIEW ) + "(_B)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTBOARDVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_HIST_CLOSEVIEW ) + "(_M)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_HIST_CLOSEBOARDVIEW ) + "(_N)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEBOARDVIEW, false ) );
        sidebar_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( std::string( ITEM_NAME_HIST_CLOSEIMGVIEW ) + "(_I)", true );
        check_item->signal_activate().connect(
                sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEIMGVIEW, false ) );
        sidebar_submenu->append( *check_item );

        sidebar_item->set_submenu( *sidebar_submenu );
        submenu->append( *sidebar_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    auto item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧(_B)", true );
    item->signal_activate().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_board ), false ) );
    submenu->append( *item );

    item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー(_T)", true );
    item->signal_activate().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_article ), false ) );
    submenu->append( *item );

    item = Gtk::make_managed<Gtk::MenuItem>( "画像ビュー(_I)", true );
    item->signal_activate().connect( sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_image ), false ) );
    submenu->append( *item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    // pane 設定
    Gtk::RadioButtonGroup radiogroup;
    auto raction0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup, "２ペイン表示(_2)", true );
    auto raction1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup, "３ペイン表示(_3)", true );
    auto raction2 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup, "縦３ペイン表示(_V)", true );

    switch( SESSION::get_mode_pane() ){
        case SESSION::MODE_2PANE: raction0->set_active( true );break;
        case SESSION::MODE_3PANE: raction1->set_active( true );break;
        case SESSION::MODE_V3PANE: raction2->set_active( true );break;
    }

    raction0->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_2pane ) );
    raction1->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_3pane ) );
    raction2->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_v3pane ) );

    submenu->append( *raction0 );
    submenu->append( *raction1 );
    submenu->append( *raction2 );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    // フルスクリーン
    auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "全画面表示(_F)", true );
    check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_fullscreen ) );
    submenu->append( *check_item );

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto view_item = Gtk::make_managed<Gtk::MenuItem>( "詳細設定(_D)", true );
        auto view_submenu = Gtk::make_managed<Gtk::Menu>();

        {
            auto general_item = Gtk::make_managed<Gtk::MenuItem>( "一般(_G)", true );
            auto general_submenu = Gtk::make_managed<Gtk::Menu>();

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "メニューバー表示(_M)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::toggle_menubar ) );
            general_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "ステータスバー表示(_S)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::toggle_statbar ) );
            general_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "ボタンをフラット表示(_F)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::toggle_flat_button ) );
            general_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "ツールバーの背景を描画する(_T)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::toggle_draw_toolbarback ) );
            general_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "自分が書き込んだレスにマークをつける(_W)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::toggle_post_mark ) );
            general_submenu->append( *check_item );

            general_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

            {
                auto since_item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧の since 表示(_N)", true );
                auto since_submenu = Gtk::make_managed<Gtk::Menu>();

                // since
                Gtk::RadioButtonGroup radiogroup_since;
                auto raction_since0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_since, "年/月/日 時:分", true );
                auto raction_since1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_since, "月/日 時:分", true );
                auto raction_since2 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_since, "年/月/日(曜日) 時:分:秒", true );
                auto raction_since3 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_since, "年/月/日 時:分:秒", true );
                auto raction_since4 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_since, "～前", true );

                switch( SESSION::get_col_since_time() ){
                    case MISC::TIME_NORMAL: raction_since0->set_active( true );break;
                    case MISC::TIME_NO_YEAR: raction_since1->set_active( true );break;
                    case MISC::TIME_WEEK: raction_since2->set_active( true );break;
                    case MISC::TIME_SECOND: raction_since3->set_active( true );break;
                    case MISC::TIME_PASSED: raction_since4->set_active( true );break;
                }

                raction_since0->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_NORMAL ) );
                raction_since1->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_NO_YEAR ) );
                raction_since2->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_WEEK ) );
                raction_since3->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_SECOND ) );
                raction_since4->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_PASSED ) );

                since_submenu->append( *raction_since0 );
                since_submenu->append( *raction_since1 );
                since_submenu->append( *raction_since2 );
                since_submenu->append( *raction_since3 );
                since_submenu->append( *raction_since4 );

                since_item->set_submenu( *since_submenu );
                general_submenu->append( *since_item );
            }

            {
                auto write_item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧の最終書込表示(_N)", true );
                auto write_submenu = Gtk::make_managed<Gtk::Menu>();

                // 最終書き込み
                Gtk::RadioButtonGroup radiogroup_write;
                auto raction_write0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_write, "年/月/日 時:分", true );
                auto raction_write1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_write, "月/日 時:分", true );
                auto raction_write2 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_write, "年/月/日(曜日) 時:分:秒", true );
                auto raction_write3 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_write, "年/月/日 時:分:秒", true );
                auto raction_write4 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_write, "～前", true );

                switch( SESSION::get_col_write_time() ){
                    case MISC::TIME_NORMAL: raction_write0->set_active( true );break;
                    case MISC::TIME_NO_YEAR: raction_write1->set_active( true );break;
                    case MISC::TIME_WEEK: raction_write2->set_active( true );break;
                    case MISC::TIME_SECOND: raction_write3->set_active( true );break;
                    case MISC::TIME_PASSED: raction_write4->set_active( true );break;
                }

                raction_write0->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_NORMAL ) );
                raction_write1->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_NO_YEAR ) );
                raction_write2->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_WEEK ) );
                raction_write3->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_SECOND ) );
                raction_write4->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_PASSED ) );

                write_submenu->append( *raction_write0 );
                write_submenu->append( *raction_write1 );
                write_submenu->append( *raction_write2 );
                write_submenu->append( *raction_write3 );
                write_submenu->append( *raction_write4 );

                write_item->set_submenu( *write_submenu );
                general_submenu->append( *write_item );
            }

            {
                auto access_item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧の最終取得表示(_N)", true );
                auto access_submenu = Gtk::make_managed<Gtk::Menu>();

                // 最終アクセス
                Gtk::RadioButtonGroup radiogroup_access;
                auto raction_access0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_access, "年/月/日 時:分", true );
                auto raction_access1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_access, "月/日 時:分", true );
                auto raction_access2 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_access, "年/月/日(曜日) 時:分:秒", true );
                auto raction_access3 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_access, "年/月/日 時:分:秒", true );
                auto raction_access4 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_access, "～前", true );

                switch( SESSION::get_col_access_time() ){
                    case MISC::TIME_NORMAL: raction_access0->set_active( true );break;
                    case MISC::TIME_NO_YEAR: raction_access1->set_active( true );break;
                    case MISC::TIME_WEEK: raction_access2->set_active( true );break;
                    case MISC::TIME_SECOND: raction_access3->set_active( true );break;
                    case MISC::TIME_PASSED: raction_access4->set_active( true );break;
                }

                raction_access0->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_NORMAL ) );
                raction_access1->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_NO_YEAR ) );
                raction_access2->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_WEEK ) );
                raction_access3->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_SECOND ) );
                raction_access4->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_PASSED ) );

                access_submenu->append( *raction_access0 );
                access_submenu->append( *raction_access1 );
                access_submenu->append( *raction_access2 );
                access_submenu->append( *raction_access3 );
                access_submenu->append( *raction_access4 );

                access_item->set_submenu( *access_submenu );
                general_submenu->append( *access_item );
            }

            general_item->set_submenu(*general_submenu );
            view_submenu->append( *general_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto tab_item = Gtk::make_managed<Gtk::MenuItem>( "タブ表示(_B)", true );
            auto tab_submenu = Gtk::make_managed<Gtk::Menu>();

            // タブ表示
            auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレ一覧(_B)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_tabboard ) );
            tab_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレビュー(_A)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_tabarticle ) );
            tab_submenu->append( *check_item );

            tab_item->set_submenu( *tab_submenu );
            view_submenu->append( *tab_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto toolbar_item = Gtk::make_managed<Gtk::MenuItem>( "ツールバー表示(_T)", true );
            auto toolbar_submenu = Gtk::make_managed<Gtk::Menu>();

            // ツールバー表示

            // メインツールバー
            auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "メインツールバー表示(_M)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_toolbarmain ) );
            toolbar_submenu->append( *check_item );

            // 各ビューのツールバー
            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "サイドバーのツールバー表示(_S)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_toolbarbbslist ) );
            toolbar_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレ一覧のツールバー表示(_B)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_toolbarboard ) );
            toolbar_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレビューのツールバー表示(_A)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_toolbararticle ) );
            toolbar_submenu->append( *check_item );

            toolbar_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

            {
                auto toolbarpos_item = Gtk::make_managed<Gtk::MenuItem>( "メインツールバーの位置(_P)", true );
                auto toolbarpos_submenu = Gtk::make_managed<Gtk::Menu>();

                Gtk::RadioButtonGroup radiogroup_toolbar;
                auto raction_toolbarpos0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_toolbar, "メニューバーの下に表示する(_U)", true );
                auto raction_toolbarpos1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_toolbar, "サイドバーの右に表示する(_R)", true );

                switch( SESSION::get_toolbar_pos() ){
                    case SESSION::TOOLBAR_POS_NORMAL: raction_toolbarpos0->set_active( true );break;
                    case SESSION::TOOLBAR_POS_RIGHT: raction_toolbarpos1->set_active( true );break;
                }

                raction_toolbarpos0->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_POS_NORMAL ) );
                raction_toolbarpos1->signal_activate().connect(
                        sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_POS_RIGHT ) );

                toolbarpos_submenu->append( *raction_toolbarpos0 );
                toolbarpos_submenu->append( *raction_toolbarpos1 );

                toolbarpos_item->set_submenu( *toolbarpos_submenu );
                toolbar_submenu->append( *toolbarpos_item );
            }

            toolbar_item->set_submenu( *toolbar_submenu );
            view_submenu->append( *toolbar_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto toolbarconf_item = Gtk::make_managed<Gtk::MenuItem>( "ツールバー項目設定(_I)", true );
            auto toolbarconf_submenu = Gtk::make_managed<Gtk::Menu>();

            // ツールバー項目設定
            auto item = Gtk::make_managed<Gtk::MenuItem>( "メイン(_M)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_mainitem ) );
            toolbarconf_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "サイドバー(_S)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_sidebaritem ) );
            toolbarconf_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧(_B)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_boarditem ) );
            toolbarconf_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー(_A)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_articleitem ) );
            toolbarconf_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "ログ/スレタイ検索(_L)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_searchitem ) );
            toolbarconf_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "書き込みビュー(_W)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_msgitem ) );
            toolbarconf_submenu->append( *item );

            toolbarconf_item->set_submenu( *toolbarconf_submenu );
            view_submenu->append( *toolbarconf_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto listitem_item = Gtk::make_managed<Gtk::MenuItem>( "リスト項目設定(_L)", true );
            auto listitem_submenu = Gtk::make_managed<Gtk::Menu>();

            // リスト表示項目設定
            auto item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧(_T)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_boarditem_column ) );
            listitem_submenu->append( *item );

            listitem_item->set_submenu( *listitem_submenu );
            view_submenu->append( *listitem_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto menuitem_item = Gtk::make_managed<Gtk::MenuItem>( "コンテキストメニュー項目設定(_C)", true );
            auto menuitem_submenu = Gtk::make_managed<Gtk::Menu>();

            // コンテキストメニュー項目設定
            auto item = Gtk::make_managed<Gtk::MenuItem>( "スレ一覧(_B)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_boarditem_menu ) );
            menuitem_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー(_A)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_articleitem_menu ) );
            menuitem_submenu->append( *item );

            menuitem_item->set_submenu( *menuitem_submenu );
            view_submenu->append( *menuitem_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto font_color_item = Gtk::make_managed<Gtk::MenuItem>( "フォントと色(_F)", true );
            auto font_color_submenu = Gtk::make_managed<Gtk::Menu>();

            auto item = Gtk::make_managed<Gtk::MenuItem>( "スレビューフォント(_T)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_main ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "メール欄フォント(_U)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_mail ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "ポップアップフォント(_P)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧フォント(_B)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
            font_color_submenu->append( *item );

            font_color_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

            item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー文字色(_C)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー背景色(_A)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧文字色(_H)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_char_tree ) );
            font_color_submenu->append( *item );

            item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧背景色(_K)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );
            font_color_submenu->append( *item );

            font_color_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

            item = Gtk::make_managed<Gtk::MenuItem>( "詳細設定(_R)...", true );
            item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_fontcolor ) );
            font_color_submenu->append( *item );

            font_color_item->set_submenu( *font_color_submenu );
            view_submenu->append( *font_color_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto imgview_item = Gtk::make_managed<Gtk::MenuItem>( "画像表示設定(_G)", true );
            auto imgview_submenu = Gtk::make_managed<Gtk::Menu>();

            // 画像表示設定
            {
                auto img_item = Gtk::make_managed<Gtk::MenuItem>( "画像ビュー(_V)", true );
                auto img_submenu = Gtk::make_managed<Gtk::Menu>();

                Gtk::RadioButtonGroup radiogroup_img;
                auto raction_img0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_img, "ウィンドウ表示する(_W)", true );
                auto raction_img1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_img, "埋め込み表示する(_E)", true );
                auto raction_img2 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_img, "表示しない(_D)", true );

                if( CONFIG::get_use_image_view() ){
                    if( ! SESSION::get_embedded_img() ) raction_img0->set_active( true );
                    else raction_img1->set_active( true );
                }
                else {
                    raction_img2->set_active( true );
                }

                raction_img0->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_WINDOW ) );
                raction_img1->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_EMB ) );
                raction_img2->signal_activate().connect( sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_NO ) );

                img_submenu->append( *raction_img0 );
                img_submenu->append( *raction_img1 );
                img_submenu->append( *raction_img2 );

                img_item->set_submenu( *img_submenu );
                imgview_submenu->append( *img_item );
            }

            auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "画像にモザイクをかける(_M)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_use_mosaic ) );
            imgview_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "画像ポップアップを表示する(_P)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_use_imgpopup ) );
            imgview_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "インライン画像を表示する(_I)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_use_inlineimg ) );
            imgview_submenu->append( *check_item );

            check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "BEアイコン/エモティコンを表示する(_B)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_show_ssspicon ) );
            imgview_submenu->append( *check_item );

            imgview_item->set_submenu( *imgview_submenu );
            view_submenu->append( *imgview_item );
        }

        view_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        {
            auto msgview_item = Gtk::make_managed<Gtk::MenuItem>( "書き込み設定(_M)", true );
            auto msgview_submenu = Gtk::make_managed<Gtk::Menu>();

            // 書き込みビュー
            {
                auto msg_item = Gtk::make_managed<Gtk::MenuItem>( "書き込みビュー(_M)", true );
                auto msg_submenu = Gtk::make_managed<Gtk::Menu>();

                Gtk::RadioButtonGroup radiogroup_msg;
                auto raction_msg0 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_msg, "ウィンドウ表示する(_W)", true );
                auto raction_msg1 = Gtk::make_managed<Gtk::RadioMenuItem>( radiogroup_msg, "埋め込み表示する(_E)", true );

                if( ! SESSION::get_embedded_mes() ) raction_msg0->set_active( true );
                else raction_msg1->set_active( true );

                raction_msg0->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_winmsg ) );
                raction_msg1->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_embmsg ) );

                msg_submenu->append( *raction_msg0 );
                msg_submenu->append( *raction_msg1 );

                msg_item->set_submenu( *msg_submenu );
                msgview_submenu->append( *msg_item );
            }

            auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "テキストを折り返し表示する(_W)", true );
            check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_msg_wrap ) );
            msgview_submenu->append( *check_item );

            msgview_item->set_submenu( *msgview_submenu );
            view_submenu->append( *msgview_item );
        }

        view_item->set_submenu( *view_submenu );
        submenu->append( *view_item );
    }

    top_menu->set_submenu( *submenu );
    return top_menu;
}

Gtk::MenuItem* CORE::Core::create_history_menu()
{
    auto top_menu = Gtk::make_managed<Gtk::MenuItem>( "履歴(_S)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    auto item = Gtk::make_managed<Gtk::MenuItem>( "前へ戻る(_P)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_prevview ) );
    submenu->append( *item );
    m_action_group.try_emplace( "PrevView", item );

    item = Gtk::make_managed<Gtk::MenuItem>( "次へ進む(_N)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_nextview ) );
    submenu->append( *item );
    m_action_group.try_emplace( "NextView", item );

    top_menu->set_submenu( *submenu );
    return top_menu;
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
    auto top_menu = Gtk::make_managed<Gtk::MenuItem>( "設定(_C)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    {
        auto property_item = Gtk::make_managed<Gtk::MenuItem>( "プロパティ(_P)", true );
        auto property_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "板一覧のプロパティ(_L)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_bbslist_pref ) );
        property_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "表示中の板のプロパティ(_B)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_board_pref ) );
        property_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "表示中のスレのプロパティ(_T)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_article_pref ) );
        property_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "表示中の画像のプロパティ(_I)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_image_pref ) );
        property_submenu->append( *item );

        property_item->set_submenu( *property_submenu );
        submenu->append( *property_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto general_item = Gtk::make_managed<Gtk::MenuItem>( "一般(_G)", true );
        auto general_submenu = Gtk::make_managed<Gtk::Menu>();

        // FIXME: GTK4 CheckMenuItem の active 状態の切り替えは Core::slot_activate_menubar() の中で設定を参照して行う。
        // 現状では CheckMenuItem とアプリケーションの状態は同期していない。
        auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "前回開いていた各ビューを起動時に復元する(_R)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_restore_views ) );
        general_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "非アクティブ時に書き込みビューを折りたたむ(_C)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_fold_message ) );
        general_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "サイドバー／スレ一覧の選択を表示中のビューと同期する(_S)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_select_item_sync ) );
        general_submenu->append( *check_item );

        general_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "書き込みログを保存する(_A)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_save_post_log ) );
        general_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "書き込み履歴(鉛筆マーク)を保存する(_P)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_save_post_history ) );
        general_submenu->append( *check_item );

        general_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "まちBBSでID表示を使用する(_I)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_show_machi_id ) );
        general_submenu->append( *check_item );

        general_item->set_submenu( *general_submenu );
        submenu->append( *general_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto mouse_item = Gtk::make_managed<Gtk::MenuItem>( "マウス／キーボード(_M)", true );
        auto mouse_submenu = Gtk::make_managed<Gtk::Menu>();

        auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレ一覧／スレビューを開く時に常に新しいタブで開く(_T)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_tabbutton ) );
        mouse_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "{X11} スレビューでアンカーをクリックして多重ポップアップモードに移行する(_W)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_popupwarpmode ) );
        mouse_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレビューでカーソルを移動して多重ポップアップモードに移行する(_M)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_shortmargin_popup ) );
        mouse_submenu->append( *check_item );

        mouse_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "書き込みビューのショートカットキーをEmacs風にする(_E)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_emacsmode ) );
        mouse_submenu->append( *check_item );

        mouse_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        auto item = Gtk::make_managed<Gtk::MenuItem>( "ショートカットキー詳細設定(_R)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_key ) );
        mouse_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "マウスジェスチャ詳細設定(_G)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_mouse ) );
        mouse_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "マウスボタン詳細設定(_B)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_button ) );
        mouse_submenu->append( *item );

        mouse_item->set_submenu( *mouse_submenu );
        submenu->append( *mouse_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto font_color_item = Gtk::make_managed<Gtk::MenuItem>( "フォントと色(_F)", true );
        auto font_color_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "スレビューフォント(_T)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_main ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "メール欄フォント(_U)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_mail ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "ポップアップフォント(_P)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧フォント(_B)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
        font_color_submenu->append( *item );

        font_color_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー文字色(_C)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "スレビュー背景色(_A)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧文字色(_H)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_char_tree ) );
        font_color_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "板／スレ一覧背景色(_K)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );
        font_color_submenu->append( *item );

        font_color_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        item = Gtk::make_managed<Gtk::MenuItem>( "詳細設定(_R)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_fontcolor ) );
        font_color_submenu->append( *item );

        font_color_item->set_submenu( *font_color_submenu );
        submenu->append( *font_color_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto network_item = Gtk::make_managed<Gtk::MenuItem>( "ネットワーク(_N)", true );
        auto network_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "プロキシ(_X)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_proxy ) );
        network_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "Webブラウザ(_W)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_browser ) );
        network_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "パスワード(_P)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_passwd ) );
        network_submenu->append( *item );

        network_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "IPv6使用(_I)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_ipv6 ) );
        network_submenu->append( *check_item );

        network_item->set_submenu( *network_submenu );
        submenu->append( *network_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto abone_item = Gtk::make_managed<Gtk::MenuItem>( "あぼ〜ん(_A)", true );
        auto abone_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "全体あぼ〜ん設定(対象: スレビュー)(_V)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_abone ) );
        abone_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "全体スレあぼ〜ん設定(対象: スレ一覧)(_L)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_abone_thread ) );
        abone_submenu->append( *item );

        abone_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        auto check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "スレビューで透明／連鎖あぼ〜んをデフォルト設定にする(_T)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_abone_transp_chain ) );
        abone_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "NG正規表現で大小と全半角文字の違いを無視する(_W)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_abone_icase_wchar ) );
        abone_submenu->append( *check_item );

        check_item = Gtk::make_managed<Gtk::CheckMenuItem>( "(実験的な機能) あぼーんしたレスに判定理由を表示する(_R)", true );
        check_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_toggle_show_abone_reason ) );
        abone_submenu->append( *check_item );

        abone_item->set_submenu( *abone_submenu );
        submenu->append( *abone_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto privacy_item = Gtk::make_managed<Gtk::MenuItem>( "プライバシー(_R)", true );
        auto privacy_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "各履歴等の消去(_I)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_clear_privacy ) );
        privacy_submenu->append( *item );

        privacy_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        item = Gtk::make_managed<Gtk::MenuItem>( "書き込みログの消去(_P)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_clear_post_log ) );
        privacy_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "書き込み履歴(鉛筆マーク)の消去(_H)", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_clear_post_history ) );
        privacy_submenu->append( *item );

        privacy_submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

        item = Gtk::make_managed<Gtk::MenuItem>( "画像キャッシュの消去(_D)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_delete_all_images ) );
        privacy_submenu->append( *item );

        privacy_item->set_submenu( *privacy_submenu );
        submenu->append( *privacy_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    {
        auto etc_item = Gtk::make_managed<Gtk::MenuItem>( "その他(_O)", true );
        auto etc_submenu = Gtk::make_managed<Gtk::Menu>();

        auto item = Gtk::make_managed<Gtk::MenuItem>( "実況設定(_L)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_setup_live ) );
        etc_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "ユーザコマンドの編集(_U)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_usrcmd_pref ) );
        etc_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "リンクフィルタの編集(_F)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_filter_pref ) );
        etc_submenu->append( *item );

        item = Gtk::make_managed<Gtk::MenuItem>( "置換文字列の編集(_R)...", true );
        item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_replace_pref ) );
        etc_submenu->append( *item );

        etc_item->set_submenu( *etc_submenu );
        submenu->append( *etc_item );
    }

    submenu->append( *Gtk::make_managed<Gtk::SeparatorMenuItem>() );

    auto item = Gtk::make_managed<Gtk::MenuItem>( "about:config 高度な設定(_C)...", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_aboutconfig ) );
    submenu->append( *item );

    top_menu->set_submenu( *submenu );
    return top_menu;
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

    // 履歴メニューに前へ戻る、次へ進むの sensitive を切り替えるシグナルハンドラを接続する。
    const auto items = m_menubar->get_children();
    auto item = dynamic_cast< Gtk::MenuItem* >( *std::next( items.begin(), 2 ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_activate_historymenu ) );

    // FIXME: GTK4 動的な履歴メニューは Gtk::ActionGroup に依存しているため
    // フェーズ1では未実装とする。
}


/**
 * @brief メニューバーがアクティブになったときに呼ばれるスロット
 */
void CORE::Core::slot_activate_menubar()
{
    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    // TODO: Gtk::MenuItem, Gtk::CheckMenuItem を使って set_active, set_sensitive を切り替える

    m_enable_menuslot = true;
}


/**
 * @brief 履歴メニューがアクティブになった
 */
void CORE::Core::slot_activate_historymenu()
{
    m_enable_menuslot = false;

    std::string view_url;
    switch( SESSION::focused_admin() ){

        case SESSION::FOCUS_BOARD: view_url = BOARD::get_admin()->get_current_url(); break;
        case SESSION::FOCUS_ARTICLE: view_url = ARTICLE::get_admin()->get_current_url(); break;
    }

    bool enable_prev = false;
    bool enable_next = false;
    if( ! view_url.empty() ){

        enable_prev = HISTORY::get_history_manager()->can_back_viewhistory( view_url, 1 );
        enable_next = HISTORY::get_history_manager()->can_forward_viewhistory( view_url, 1 );
    }

#ifdef _DEBUG
    std::cout << "Core::slot_activate_historymenu\n"
              << "view_url = " << view_url
              << " prev = " << enable_prev << " next = " << enable_next
              << std::endl;
#endif

    if( auto it = m_action_group.find( "PrevView" ); it != m_action_group.end() ) {
        it->second->set_sensitive( enable_prev );
    }
    if( auto it = m_action_group.find( "NextView" ); it != m_action_group.end() ) {
        it->second->set_sensitive( enable_next );
    }

    m_enable_menuslot = true;
}
