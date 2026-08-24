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


/**
 * @brief メインメニューの設定
 */
void CORE::Core::setup_menubar()
{
    m_action_group = Gtk::ActionGroup::create();

    // File
    m_action_group->add( Gtk::Action::create( "Menu_File", "ファイル(_F)" ) );
    m_action_group->add( Gtk::Action::create( "OpenURL", "OpenURL"), sigc::mem_fun( *this, &Core::slot_openurl ) );
    m_action_group->add( Gtk::ToggleAction::create( "Online", "オフライン作業(_W)", std::string(), ! SESSION::is_online() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_online ) );
    m_action_group->add( Gtk::ToggleAction::create( "Login2ch", "2chにログイン(_L)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_login2ch ) );
    m_action_group->add( Gtk::ToggleAction::create( "LoginBe", "BEにログイン(_B)", std::string(), false ),
                        sigc::mem_fun( *this, &Core::slot_toggle_loginbe ) );
    m_action_group->add( Gtk::ToggleAction::create( "LoginAcorn", "どんぐりシステムにGmail警備員●でログイン(_G)", {}, false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_loginacorn ) );
    m_action_group->add( Gtk::Action::create( "ReloadList", "板一覧再読込(_R)"), sigc::mem_fun( *this, &Core::slot_reload_list ) );

    m_action_group->add( Gtk::Action::create( "SaveSession", "セッション保存(_S)"), sigc::mem_fun( *this, &Core::save_session ) );

    Gtk::AccelKey jdexitKey = CONTROL::get_accelkey( CONTROL::JDExit );
    if( jdexitKey.is_null() ){
        m_action_group->add( Gtk::Action::create( "Quit", "終了(_Q)" ),
                             sigc::mem_fun(*this, &Core::slot_quit ) );
    }else{
        m_action_group->add( Gtk::Action::create( "Quit", "終了(_Q)" ),
                             jdexitKey,
                             sigc::mem_fun(*this, &Core::slot_quit ) );
    }


    //////////////////////////////////////////////////////

    // 表示
    m_action_group->add( Gtk::Action::create( "Menu_View", "表示(_V)" ) );

    m_action_group->add( Gtk::Action::create( "Show_Board", "スレ一覧(_B)" ),
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_board ), false ) );
    m_action_group->add( Gtk::Action::create( "Show_Thread", "スレビュー(_T)" ),
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_article ), false ) );
    m_action_group->add( Gtk::Action::create( "Show_Image", "画像ビュー(_I)" ),
                         sigc::bind< bool >( sigc::mem_fun(*this, &Core::switch_image ), false ) );

    // サイドバー
    m_action_group->add( Gtk::Action::create( "Sidebar_Menu", "サイドバー(_S)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_BBS", "板一覧(_B)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_BBSLISTVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_FAVORITE", std::string( ITEM_NAME_FAVORITEVIEW ) + "(_F)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_FAVORITEVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_HISTTHREAD", std::string( ITEM_NAME_HISTVIEW ) + "(_T)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTTHREADVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_HISTBOARD", std::string( ITEM_NAME_HIST_BOARDVIEW ) + "(_B)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTBOARDVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_HISTCLOSE", std::string( ITEM_NAME_HIST_CLOSEVIEW ) + "(_M)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_HISTCLOSEBOARD", std::string( ITEM_NAME_HIST_CLOSEBOARDVIEW ) + "(_N)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEBOARDVIEW, false ) );
    m_action_group->add( Gtk::ToggleAction::create( "Show_HISTCLOSEIMG", std::string( ITEM_NAME_HIST_CLOSEIMGVIEW ) + "(_I)", std::string(), SESSION::show_sidebar() ),
                         sigc::bind< std::string, bool >( sigc::mem_fun(*this, &Core::switch_sidebar ), URL_HISTCLOSEIMGVIEW, false ) );

    m_action_group->add( Gtk::Action::create( "View_Menu", "詳細設定(_D)" ) );

    // 一般
    m_action_group->add( Gtk::ToggleAction::create( "ShowMenuBar", "ShowMenuBar", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_menubar ) );
    m_action_group->add( Gtk::ToggleAction::create( "ShowStatBar", "ステータスバー表示(_S)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_statbar ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToggleFlatButton", "ボタンをフラット表示(_F)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_flat_button ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToggleDrawToolbarback", "ツールバーの背景を描画する(_T)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::toggle_draw_toolbarback ) );
    m_action_group->add( Gtk::ToggleAction::create( "TogglePostMark", "自分が書き込んだレスにマークをつける(_W)",
                                                    std::string(), CONFIG::get_show_post_mark() ),
                         sigc::mem_fun( *this, &Core::toggle_post_mark ) );

    // since
    Gtk::RadioButtonGroup radiogroup_since;
    m_action_group->add( Gtk::Action::create( "Since_Menu", "スレ一覧の since 表示(_N)" ) );
    Glib::RefPtr< Gtk::RadioAction > raction_since0 = Gtk::RadioAction::create( radiogroup_since, "Since_Normal", "年/月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_since1 = Gtk::RadioAction::create( radiogroup_since, "Since_NoYear", "月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_since2 = Gtk::RadioAction::create( radiogroup_since, "Since_Week", "年/月/日(曜日) 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_since3 = Gtk::RadioAction::create( radiogroup_since, "Since_Second", "年/月/日 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_since4 = Gtk::RadioAction::create( radiogroup_since, "Since_Passed", "～前" );

    switch( SESSION::get_col_since_time() ){
        case MISC::TIME_NORMAL: raction_since0->set_active( true );break;
        case MISC::TIME_NO_YEAR: raction_since1->set_active( true );break;
        case MISC::TIME_WEEK: raction_since2->set_active( true );break;
        case MISC::TIME_SECOND: raction_since3->set_active( true );break;
        case MISC::TIME_PASSED: raction_since4->set_active( true );break;
    }

    m_action_group->add( raction_since0,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_NORMAL ) );
    m_action_group->add( raction_since1,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_NO_YEAR ) );
    m_action_group->add( raction_since2,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_WEEK ) );
    m_action_group->add( raction_since3,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_SECOND ) );
    m_action_group->add( raction_since4,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_since ), MISC::TIME_PASSED ) );

    // 最終書き込み
    Gtk::RadioButtonGroup radiogroup_write;
    m_action_group->add( Gtk::Action::create( "Write_Menu", "スレ一覧の最終書込表示(_N)" ) );
    Glib::RefPtr< Gtk::RadioAction > raction_write0 = Gtk::RadioAction::create( radiogroup_write, "Write_Normal", "年/月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_write1 = Gtk::RadioAction::create( radiogroup_write, "Write_NoYear", "月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_write2 = Gtk::RadioAction::create( radiogroup_write, "Write_Week", "年/月/日(曜日) 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_write3 = Gtk::RadioAction::create( radiogroup_write, "Write_Second", "年/月/日 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_write4 = Gtk::RadioAction::create( radiogroup_write, "Write_Passed", "～前" );

    switch( SESSION::get_col_write_time() ){
        case MISC::TIME_NORMAL: raction_write0->set_active( true );break;
        case MISC::TIME_NO_YEAR: raction_write1->set_active( true );break;
        case MISC::TIME_WEEK: raction_write2->set_active( true );break;
        case MISC::TIME_SECOND: raction_write3->set_active( true );break;
        case MISC::TIME_PASSED: raction_write4->set_active( true );break;
    }

    m_action_group->add( raction_write0,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_NORMAL ) );
    m_action_group->add( raction_write1,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_NO_YEAR ) );
    m_action_group->add( raction_write2,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_WEEK ) );
    m_action_group->add( raction_write3,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_SECOND ) );
    m_action_group->add( raction_write4,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_write ), MISC::TIME_PASSED ) );

    // 最終アクセス
    Gtk::RadioButtonGroup radiogroup_access;
    m_action_group->add( Gtk::Action::create( "Access_Menu", "スレ一覧の最終取得表示(_N)" ) );
    Glib::RefPtr< Gtk::RadioAction > raction_access0 = Gtk::RadioAction::create( radiogroup_access, "Access_Normal", "年/月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_access1 = Gtk::RadioAction::create( radiogroup_access, "Access_NoYear", "月/日 時:分" );
    Glib::RefPtr< Gtk::RadioAction > raction_access2 = Gtk::RadioAction::create( radiogroup_access, "Access_Week", "年/月/日(曜日) 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_access3 = Gtk::RadioAction::create( radiogroup_access, "Access_Second", "年/月/日 時:分:秒" );
    Glib::RefPtr< Gtk::RadioAction > raction_access4 = Gtk::RadioAction::create( radiogroup_access, "Access_Passed", "～前" );

    switch( SESSION::get_col_access_time() ){
        case MISC::TIME_NORMAL: raction_access0->set_active( true );break;
        case MISC::TIME_NO_YEAR: raction_access1->set_active( true );break;
        case MISC::TIME_WEEK: raction_access2->set_active( true );break;
        case MISC::TIME_SECOND: raction_access3->set_active( true );break;
        case MISC::TIME_PASSED: raction_access4->set_active( true );break;
    }

    m_action_group->add( raction_access0,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_NORMAL ) );
    m_action_group->add( raction_access1,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_NO_YEAR ) );
    m_action_group->add( raction_access2,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_WEEK ) );
    m_action_group->add( raction_access3,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_SECOND ) );
    m_action_group->add( raction_access4,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_access ), MISC::TIME_PASSED ) );

    // ツールバー表示
    m_action_group->add( Gtk::Action::create( "Toolbar_Menu", "ツールバー表示(_T)" ) );

    // メインツールバー
    m_action_group->add( Gtk::ToggleAction::create( "ShowToolBarMain", "ShowToolBarMain", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbarmain ) );

    Gtk::RadioButtonGroup radiogroup_toolbar;
    m_action_group->add( Gtk::Action::create( "Toolbar_Main_Menu", "メインツールバーの位置(_P)" ) );
    Glib::RefPtr< Gtk::RadioAction > raction_toolbarpos0 = Gtk::RadioAction::create( radiogroup_toolbar, "ToolbarPos0", "メニューバーの下に表示する(_U)" );
    Glib::RefPtr< Gtk::RadioAction > raction_toolbarpos1 = Gtk::RadioAction::create( radiogroup_toolbar, "ToolbarPos1", "サイドバーの右に表示する(_R)" );

    switch( SESSION::get_toolbar_pos() ){
        case SESSION::TOOLBAR_POS_NORMAL: raction_toolbarpos0->set_active( true );break;
        case SESSION::TOOLBAR_POS_RIGHT: raction_toolbarpos1->set_active( true );break;
    }

    m_action_group->add( raction_toolbarpos0,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_POS_NORMAL ) );
    m_action_group->add( raction_toolbarpos1,
                         sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_toolbarpos ), SESSION::TOOLBAR_POS_RIGHT ) );

    // その他のツールバー
    m_action_group->add( Gtk::ToggleAction::create( "ShowToolBarBbslist", "サイドバーのツールバー表示(_S)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbarbbslist ) );
    m_action_group->add( Gtk::ToggleAction::create( "ShowToolBarBoard", "スレ一覧のツールバー表示(_B)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbarboard ) );
    m_action_group->add( Gtk::ToggleAction::create( "ShowToolBarArticle", "スレビューのツールバー表示(_A)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_toolbararticle ) );

    // タブ表示
    m_action_group->add( Gtk::Action::create( "Tab_Menu", "タブ表示(_B)" ) );
    m_action_group->add( Gtk::ToggleAction::create( "TabBoard", "スレ一覧(_B)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabboard ) );
    m_action_group->add( Gtk::ToggleAction::create( "TabArticle", "スレビュー(_A)", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabarticle ) );

    // pane 設定
    Gtk::RadioButtonGroup radiogroup;
    Glib::RefPtr< Gtk::RadioAction > raction0 = Gtk::RadioAction::create( radiogroup, "2Pane", "２ペイン表示(_2)" );
    Glib::RefPtr< Gtk::RadioAction > raction1 = Gtk::RadioAction::create( radiogroup, "3Pane", "３ペイン表示(_3)" );
    Glib::RefPtr< Gtk::RadioAction > raction2 = Gtk::RadioAction::create( radiogroup, "v3Pane", "縦３ペイン表示(_V)" );

    switch( SESSION::get_mode_pane() ){
        case SESSION::MODE_2PANE: raction0->set_active( true );break;
        case SESSION::MODE_3PANE: raction1->set_active( true );break;
        case SESSION::MODE_V3PANE: raction2->set_active( true );break;
    }

    m_action_group->add( raction0, sigc::mem_fun( *this, &Core::slot_toggle_2pane ) );
    m_action_group->add( raction1, sigc::mem_fun( *this, &Core::slot_toggle_3pane ) );
    m_action_group->add( raction2, sigc::mem_fun( *this, &Core::slot_toggle_v3pane ) );

    // フルスクリーン
    m_action_group->add( Gtk::ToggleAction::create( "FullScreen", "FullScreen", std::string(), false ),
                         sigc::mem_fun( *this, &Core::slot_toggle_fullscreen ) );


    // 書き込みビュー
    m_action_group->add( Gtk::Action::create( "MessageView_Menu", "書き込み設定(_M)" ) );
    m_action_group->add( Gtk::Action::create( "ShowMsgView_Menu", "書き込みビュー(_M)" ) );

    Gtk::RadioButtonGroup radiogroup_msg;
    Glib::RefPtr< Gtk::RadioAction > raction_msg0 = Gtk::RadioAction::create( radiogroup_msg, "UseWinMsg", "ウィンドウ表示する(_W)" );
    Glib::RefPtr< Gtk::RadioAction > raction_msg1 = Gtk::RadioAction::create( radiogroup_msg, "UseEmbMsg", "埋め込み表示する(_E)" );

    if( ! SESSION::get_embedded_mes() ) raction_msg0->set_active( true );
    else raction_msg1->set_active( true );

    m_action_group->add( raction_msg0, sigc::mem_fun( *this, &Core::slot_toggle_winmsg ) );
    m_action_group->add( raction_msg1, sigc::mem_fun( *this, &Core::slot_toggle_embmsg ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleWrap", "テキストを折り返し表示する(_W)", std::string(), CONFIG::get_message_wrap() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_msg_wrap ) );

    // 画像表示設定
    m_action_group->add( Gtk::Action::create( "ImageView_Menu", "画像表示設定(_G)" ) );
    m_action_group->add( Gtk::Action::create( "ShowImageView_Menu", "画像ビュー(_V)" ) );

    Gtk::RadioButtonGroup radiogroup_img;
    Glib::RefPtr< Gtk::RadioAction > raction_img0 = Gtk::RadioAction::create( radiogroup_img, "UseWinImg", "ウィンドウ表示する(_W)" );
    Glib::RefPtr< Gtk::RadioAction > raction_img1 = Gtk::RadioAction::create( radiogroup_img, "UseEmbImg", "埋め込み表示する(_E)" );
    Glib::RefPtr< Gtk::RadioAction > raction_img2 = Gtk::RadioAction::create( radiogroup_img, "NoUseImg", "表示しない(_D)" );

    if( CONFIG::get_use_image_view() ){
        if( ! SESSION::get_embedded_img() ) raction_img0->set_active( true );
        else raction_img1->set_active( true );
    }
    else {
        raction_img2->set_active( true );
    }

    m_action_group->add( raction_img0, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_WINDOW ) );
    m_action_group->add( raction_img1, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_EMB ) );
    m_action_group->add( raction_img2, sigc::bind< int >( sigc::mem_fun( *this, &Core::slot_toggle_imgview ), IMGVIEW_NO ) );

    m_action_group->add( Gtk::ToggleAction::create( "UseMosaic", "画像にモザイクをかける(_M)", std::string(), CONFIG::get_use_mosaic() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_mosaic ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseImgPopup", "画像ポップアップを表示する(_P)", std::string(), CONFIG::get_use_image_popup() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_imgpopup ) );
    m_action_group->add( Gtk::ToggleAction::create( "UseInlineImg", "インライン画像を表示する(_I)", std::string(), CONFIG::get_use_inline_image() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_use_inlineimg ) );
    m_action_group->add( Gtk::ToggleAction::create( "ShowSsspIcon", "BEアイコン/エモティコンを表示する(_B)", std::string(), CONFIG::get_show_ssspicon() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_show_ssspicon ) );

    // リスト表示項目設定
    m_action_group->add( Gtk::Action::create( "ListItem_Menu", "リスト項目設定(_L)" ) );
    m_action_group->add( Gtk::Action::create( "SetupBoardItemColumn", "スレ一覧(_T)..." ), sigc::mem_fun( *this, &Core::slot_setup_boarditem_column ) );

    // ツールバー項目設定
    m_action_group->add( Gtk::Action::create( "Item_Menu", "ツールバー項目設定(_I)" ) );
    m_action_group->add( Gtk::Action::create( "SetupMainItem", "メイン(_M)..." ), sigc::mem_fun( *this, &Core::slot_setup_mainitem ) );
    m_action_group->add( Gtk::Action::create( "SetupSidebarItem", "サイドバー(_S)..." ), sigc::mem_fun( *this, &Core::slot_setup_sidebaritem ) );
    m_action_group->add( Gtk::Action::create( "SetupBoardItem", "スレ一覧(_B)..." ), sigc::mem_fun( *this, &Core::slot_setup_boarditem ) );
    m_action_group->add( Gtk::Action::create( "SetupArticleItem", "スレビュー(_A)..." ), sigc::mem_fun( *this, &Core::slot_setup_articleitem ) );
    m_action_group->add( Gtk::Action::create( "SetupSearchItem", "ログ/スレタイ検索(_L)..." ), sigc::mem_fun( *this, &Core::slot_setup_searchitem ) );
    m_action_group->add( Gtk::Action::create( "SetupMsgItem", "書き込みビュー(_W)..." ), sigc::mem_fun( *this, &Core::slot_setup_msgitem ) );


    // コンテキストメニュー項目設定
    m_action_group->add( Gtk::Action::create( "MenuItem_Menu", "コンテキストメニュー項目設定(_C)" ) );
    m_action_group->add( Gtk::Action::create( "SetupBoardItemMenu", "スレ一覧(_B)..." ), sigc::mem_fun( *this, &Core::slot_setup_boarditem_menu ) );
    m_action_group->add( Gtk::Action::create( "SetupArticleItemMenu", "スレビュー(_A)..." ), sigc::mem_fun( *this, &Core::slot_setup_articleitem_menu ) );


    //////////////////////////////////////////////////////

    // 履歴
    m_action_group->add( Gtk::Action::create( "Menu_History", "履歴(_S)" ) );

    // 戻る、進む
    m_action_group->add( Gtk::Action::create( "PrevView", "PrevView"), sigc::mem_fun( *this, &Core::slot_prevview ) );
    m_action_group->add( Gtk::Action::create( "NextView", "NextView"), sigc::mem_fun( *this, &Core::slot_nextview ) );

    //////////////////////////////////////////////////////

    // 設定
    m_action_group->add( Gtk::Action::create( "Menu_Config", "設定(_C)" ) );

    m_action_group->add( Gtk::Action::create( "Property_Menu", "プロパティ(_P)" ) );
    m_action_group->add( Gtk::Action::create( "BbslistPref", "板一覧のプロパティ(_L)..." ), sigc::mem_fun( *this, &Core::slot_bbslist_pref ) );
    m_action_group->add( Gtk::Action::create( "BoardPref", "表示中の板のプロパティ(_B)..." ), sigc::mem_fun( *this, &Core::slot_board_pref ) );
    m_action_group->add( Gtk::Action::create( "ArticlePref", "表示中のスレのプロパティ(_T)..." ), sigc::mem_fun( *this, &Core::slot_article_pref ) );
    m_action_group->add( Gtk::Action::create( "ImagePref", "表示中の画像のプロパティ(_I)..." ), sigc::mem_fun( *this, &Core::slot_image_pref ) );

    // 一般
    m_action_group->add( Gtk::Action::create( "General_Menu", "一般(_G)" ) );

    m_action_group->add( Gtk::ToggleAction::create( "RestoreViews", "前回開いていた各ビューを起動時に復元する(_R)", std::string(),
                                                    ( CONFIG::get_restore_board()
                                                      && CONFIG::get_restore_article()
                                                      && CONFIG::get_restore_image() ) ),
                         sigc::mem_fun( *this, &Core::slot_toggle_restore_views ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleFoldMessage", "非アクティブ時に書き込みビューを折りたたむ(_C)", std::string(),
                                                    CONFIG::get_fold_message() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_fold_message ) );

    m_action_group->add( Gtk::ToggleAction::create( "SelectItemSync", "サイドバー／スレ一覧の選択を表示中のビューと同期する(_S)", std::string(),
                                                    ( CONFIG::get_select_item_sync() != 0 ) ),
                         sigc::mem_fun( *this, &Core::slot_toggle_select_item_sync ) );

    m_action_group->add( Gtk::ToggleAction::create( "SavePostLog", "書き込みログを保存する(_A)", std::string(), CONFIG::get_save_post_log() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_save_post_log ) );
    m_action_group->add( Gtk::ToggleAction::create( "SavePostHist", "書き込み履歴(鉛筆マーク)を保存する(_P)", std::string(), CONFIG::get_save_post_history() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_save_post_history ) );


    m_action_group->add( Gtk::ToggleAction::create( "ShowMachiID", "まちBBSでID表示を使用する(_I)",
                                                    std::string(), CONFIG::get_show_machi_id() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_show_machi_id ) );


    // マウス／キーボード
    m_action_group->add( Gtk::Action::create( "Mouse_Menu", "マウス／キーボード(_M)" ) );

    const bool toggled = CONTROL::is_toggled_tab_button() && CONTROL::is_toggled_tab_key();
    m_action_group->add( Gtk::ToggleAction::create( "ToggleTab", "スレ一覧／スレビューを開く時に常に新しいタブで開く(_T)", std::string(), toggled ),
                         sigc::mem_fun( *this, &Core::slot_toggle_tabbutton ) );

    m_action_group->add( Gtk::ToggleAction::create( "TogglePopupWarp", "{X11} スレビューでアンカーをクリックして多重ポップアップモードに移行する(_W)", std::string(),
                                                    CONTROL::is_popup_warpmode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_popupwarpmode ) );

    m_action_group->add( Gtk::ToggleAction::create( "ShortMarginPopup", "スレビューでカーソルを移動して多重ポップアップモードに移行する(_M)", std::string(),
                                                    ( CONFIG::get_margin_popup() != CONFIG::CONF_MARGIN_POPUP ) ),
                         sigc::mem_fun( *this, &Core::slot_shortmargin_popup ) );

    m_action_group->add( Gtk::ToggleAction::create( "ToggleEmacsMode", "書き込みビューのショートカットキーをEmacs風にする(_E)", std::string(),
                                                    CONTROL::is_emacs_mode() ),
                         sigc::mem_fun( *this, &Core::slot_toggle_emacsmode ) );

    m_action_group->add( Gtk::Action::create( "MousePref", "マウスジェスチャ詳細設定(_G)..." ), sigc::mem_fun( *this, &Core::slot_setup_mouse ) );
    m_action_group->add( Gtk::Action::create( "KeyPref", "ショートカットキー詳細設定(_R)..." ), sigc::mem_fun( *this, &Core::slot_setup_key ) );
    m_action_group->add( Gtk::Action::create( "ButtonPref", "マウスボタン詳細設定(_B)..." ), sigc::mem_fun( *this, &Core::slot_setup_button ) );

    // フォントと色
    m_action_group->add( Gtk::Action::create( "FontColor_Menu", "フォントと色(_F)" ) );

    m_action_group->add( Gtk::Action::create( "FontMain", "スレビューフォント(_T)..." ), sigc::mem_fun( *this, &Core::slot_changefont_main ) );
    m_action_group->add( Gtk::Action::create( "FontMail", "メール欄フォント(_U)..." ), sigc::mem_fun( *this, &Core::slot_changefont_mail ) );
    m_action_group->add( Gtk::Action::create( "FontPopup", "ポップアップフォント(_P)..." ), sigc::mem_fun( *this, &Core::slot_changefont_popup ) );
    m_action_group->add( Gtk::Action::create( "FontTree", "板／スレ一覧フォント(_B)..." ), sigc::mem_fun( *this, &Core::slot_changefont_tree ) );
    m_action_group->add( Gtk::Action::create( "ColorChar", "スレビュー文字色(_C)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_char ) );
    m_action_group->add( Gtk::Action::create( "ColorBack", "スレビュー背景色(_A)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_back ) );
    m_action_group->add( Gtk::Action::create( "ColorCharTree", "板／スレ一覧文字色(_H)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_char_tree ) );
    m_action_group->add( Gtk::Action::create( "ColorBackTree", "板／スレ一覧背景色(_K)..." ), sigc::mem_fun( *this, &Core::slot_changecolor_back_tree ) );
    m_action_group->add( Gtk::Action::create( "FontColorPref", "詳細設定(_R)..." ), sigc::mem_fun( *this, &Core::slot_setup_fontcolor ) );

    // ネットワーク
    m_action_group->add( Gtk::Action::create( "Net_Menu", "ネットワーク(_N)" ) );
    m_action_group->add( Gtk::Action::create( "SetupProxy", "プロキシ(_X)..." ), sigc::mem_fun( *this, &Core::slot_setup_proxy ) );
    m_action_group->add( Gtk::Action::create( "SetupBrowser", "Webブラウザ(_W)..." ), sigc::mem_fun( *this, &Core::slot_setup_browser ) );
    m_action_group->add( Gtk::Action::create( "SetupPasswd", "パスワード(_P)..." ), sigc::mem_fun( *this, &Core::slot_setup_passwd ) );
    m_action_group->add( Gtk::ToggleAction::create( "ToggleIPv6", "IPv6使用(_I)", std::string(),
                                                    CONFIG::get_use_ipv6() ), sigc::mem_fun( *this, &Core::slot_toggle_ipv6 ) );

    // あぼーん
    m_action_group->add( Gtk::Action::create( "Abone_Menu", "あぼ〜ん(_A)" ) );
    m_action_group->add( Gtk::Action::create( "SetupAbone", "全体あぼ〜ん設定(対象: スレビュー)(_V)..." ), sigc::mem_fun( *this, &Core::slot_setup_abone ) );
    m_action_group->add( Gtk::Action::create( "SetupAboneThread", "全体スレあぼ〜ん設定(対象: スレ一覧)(_L)..." ),
                         sigc::mem_fun( *this, &Core::slot_setup_abone_thread ) );

    m_action_group->add( Gtk::ToggleAction::create( "TranspChainAbone", "スレビューで透明／連鎖あぼ〜んをデフォルト設定にする(_T)", std::string(),
                                                    ( CONFIG::get_abone_transparent() && CONFIG::get_abone_chain() ) ),
                                                    sigc::mem_fun( *this, &Core::slot_toggle_abone_transp_chain ) );

    m_action_group->add( Gtk::ToggleAction::create( "IcaseWcharAbone", "NG正規表現で大小と全半角文字の違いを無視する(_W)", std::string(),
                                                    ( CONFIG::get_abone_icase() && CONFIG::get_abone_wchar() ) ),
                                                    sigc::mem_fun( *this, &Core::slot_toggle_abone_icase_wchar ) );

    m_action_group->add( Gtk::ToggleAction::create( "ShowAboneReason", "(実験的な機能) あぼーんしたレスに判定理由を表示する(_R)",
                                                    Glib::ustring{}, CONFIG::get_show_abone_reason() ),
                                                    sigc::mem_fun( *this, &Core::slot_toggle_show_abone_reason ) );

    // その他
    m_action_group->add( Gtk::Action::create( "Etc_Menu", "その他(_O)" ) );
    m_action_group->add( Gtk::Action::create( "LivePref", "実況設定(_L)..." ), sigc::mem_fun( *this, &Core::slot_setup_live ) );
    m_action_group->add( Gtk::Action::create( "UsrCmdPref", "ユーザコマンドの編集(_U)..." ), sigc::mem_fun( *this, &Core::slot_usrcmd_pref ) );
    m_action_group->add( Gtk::Action::create( "FilterPref", "リンクフィルタの編集(_F)..." ), sigc::mem_fun( *this, &Core::slot_filter_pref ) );
    m_action_group->add( Gtk::Action::create( "ReplacePref", "置換文字列の編集(_R)..." ), sigc::mem_fun( *this, &Core::slot_replace_pref ) );
    m_action_group->add( Gtk::Action::create( "AboutConfig", "about:config 高度な設定(_C)..." ), sigc::mem_fun( *this, &Core::slot_aboutconfig ) );


    // プライバシー
    m_action_group->add( Gtk::Action::create( "Privacy_Menu", "プライバシー(_R)" ) );
    m_action_group->add( Gtk::Action::create( "ClearAllPrivacy", "各履歴等の消去(_I)..." ), sigc::mem_fun( *this, &Core::slot_clear_privacy ) );
    m_action_group->add( Gtk::Action::create( "ClearPostLog", "書き込みログの消去(_P)" ), sigc::mem_fun( *this, &Core::slot_clear_post_log ) );
    m_action_group->add( Gtk::Action::create( "ClearPostHist", "書き込み履歴(鉛筆マーク)の消去(_H)" ), sigc::mem_fun( *this, &Core::slot_clear_post_history ) );
    m_action_group->add( Gtk::Action::create( "DeleteImages", "画像キャッシュの消去(_D)..." ), sigc::mem_fun( *this, &Core::slot_delete_all_images ) );


    //////////////////////////////////////////////////////

    // ツール
    m_action_group->add( Gtk::Action::create( "Menu_Tool", "ツール(_T)" ) );

    m_action_group->add( Gtk::Action::create( "LiveStartStop", "LiveStartStop"), sigc::mem_fun( *this, &Core::slot_live_start_stop ) );

    m_action_group->add( Gtk::Action::create( "SearchCache_Menu", "キャッシュ内ログ検索(_C)" ) );
    m_action_group->add( Gtk::Action::create( "SearchCacheBoard", "表示中の板のログを検索(_B)" ), sigc::mem_fun( *this, &Core::slot_search_cache_board ) );
    m_action_group->add( Gtk::Action::create( "SearchCache", "キャッシュ内の全ログを検索(_A)" ), sigc::mem_fun( *this, &Core::slot_search_cache ) );

    m_action_group->add( Gtk::Action::create( "ShowCache_Menu", "キャッシュ内ログ一覧(_H)" ) );
    m_action_group->add( Gtk::Action::create( "ShowCacheBoard", "表示中の板のログをスレ一覧に表示(_B)" ), sigc::mem_fun( *this, &Core::slot_show_cache_board ) );
    m_action_group->add( Gtk::Action::create( "ShowCache", "キャッシュ内の全ログをスレ一覧に表示(_A)" ), sigc::mem_fun( *this, &Core::slot_show_cache ) );

    m_action_group->add( Gtk::Action::create( "SearchTitle", "SearchTitle" ), sigc::mem_fun( *this, &Core::slot_search_title ) );

    m_action_group->add( Gtk::Action::create( "CheckUpdate_Menu", "サイドバーの更新チェック(_U)" ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateRoot", "更新チェックのみ(_R)" ), sigc::mem_fun( *this, &Core::slot_check_update_root ) );
    m_action_group->add( Gtk::Action::create( "CheckUpdateOpenRoot", "更新されたスレをタブで開く(_T)" ),
                         sigc::mem_fun( *this, &Core::slot_check_update_open_root ) );
    m_action_group->add( Gtk::Action::create( "CancelCheckUpdate", "キャンセル(_C)" ),
                         sigc::mem_fun( *this, &Core::slot_cancel_check_update ) );

    m_action_group->add( Gtk::Action::create( "EditFavorite", "お気に入りの編集(_E)"), sigc::mem_fun( *this, &Core::slot_edit_favorite ) );

    m_action_group->add( Gtk::Action::create( "ShowPostlog", "書き込みログの表示(_P)" ), sigc::mem_fun( *this, &Core::slot_show_postlog ) );

    m_action_group->add( Gtk::Action::create( "ImportDat", "表示中の板にdatをインポート(_I)" ), sigc::mem_fun( *this, &Core::slot_import_dat ) );

    m_action_group->add( Gtk::Action::create( "ShowSidebarBoard", "サイドバーをスレ一覧に表示(_B)" ), sigc::mem_fun( *this, &Core::slot_show_sidebarboard ) );

    m_action_group->add( Gtk::Action::create( "CreateVBoard", "サイドバーの仮想板を作成(_V)" ), sigc::mem_fun( *this, &Core::slot_create_vboard ) );


    //////////////////////////////////////////////////////

    // help
    m_action_group->add( Gtk::Action::create( "Menu_Help", "ヘルプ(_H)" ) );
    m_action_group->add( Gtk::Action::create( "Bbs", "JD サポート掲示板(_B)" ), sigc::mem_fun( *this, &Core::slot_show_bbs ) );
    m_action_group->add( Gtk::Action::create( "OldLog", "2chスレ過去ログ(_L)" ), sigc::mem_fun( *this, &Core::slot_show_old2ch ) );
    Gtk::AccelKey jdhelpKey = CONTROL::get_accelkey( CONTROL::JDHelp );
    if( jdhelpKey.is_null() ){
        m_action_group->add( Gtk::Action::create( "Manual", "オンラインマニュアル(_M)..." ),
                             sigc::mem_fun( *this, &Core::slot_show_manual ) );
    }else{
        m_action_group->add( Gtk::Action::create( "Manual", "オンラインマニュアル(_M)..." ),
                             jdhelpKey,
                             sigc::mem_fun( *this, &Core::slot_show_manual ) );
    }
    m_action_group->add( Gtk::Action::create( "About", "JDimについて(_A)..." ), sigc::mem_fun( *this, &Core::slot_show_about ) );


    m_ui_manager = Gtk::UIManager::create();
    m_ui_manager->insert_action_group( m_action_group );

    // アクセラレータの追加
    m_win_main.add_accel_group( m_ui_manager->get_accel_group() );

    Glib::ustring menu_font =
        "<menu action='FontColor_Menu'>"
            "<menuitem action='FontMain'/>"
            "<menuitem action='FontMail'/>"
            "<menuitem action='FontPopup'/>"
            "<menuitem action='FontTree'/>"
            "<separator/>"
            "<menuitem action='ColorChar'/>"
            "<menuitem action='ColorBack'/>"
            "<menuitem action='ColorCharTree'/>"
            "<menuitem action='ColorBackTree'/>"
            "<separator/>"
            "<menuitem action='FontColorPref'/>"
        "</menu>";

    Glib::ustring str_ui =
        "<ui>"
        "<menubar name='menu_bar'>"

    // ファイル
        "<menu action='Menu_File'>"
            "<menuitem action='OpenURL'/>"
            "<separator/>"
            "<menuitem action='Login2ch'/>"
            "<menuitem action='LoginBe'/>"
            "<menuitem action='LoginAcorn'/>"
            "<separator/>"
            "<menuitem action='SaveSession'/>"
            "<separator/>"
            "<menuitem action='ReloadList'/>"
            "<separator/>"
            "<menuitem action='Online'/>"
            "<menuitem action='Quit'/>"
        "</menu>"

    // 表示
        "<menu action='Menu_View'>"

            "<menu action='Sidebar_Menu'>"
                "<menuitem action='Show_BBS'/>"
                "<menuitem action='Show_FAVORITE'/>"
                "<menuitem action='Show_HISTTHREAD'/>"
                "<menuitem action='Show_HISTBOARD'/>"
                "<menuitem action='Show_HISTCLOSE'/>"
                "<menuitem action='Show_HISTCLOSEBOARD'/>"
                "<menuitem action='Show_HISTCLOSEIMG'/>"
            "</menu>"
            "<separator/>"

            "<menuitem action='Show_Board'/>"
            "<menuitem action='Show_Thread'/>"
            "<menuitem action='Show_Image'/>"
            "<separator/>"

            "<menuitem action='2Pane'/>"
            "<menuitem action='3Pane'/>"
            "<menuitem action='v3Pane'/>"
            "<separator/>"

            "<menuitem action='FullScreen'/>"
            "<separator/>"

    // 詳細設定
            "<menu action='View_Menu'>"

                "<menu action='General_Menu'>"
                    "<menuitem action='ShowMenuBar'/>"
                    "<menuitem action='ShowStatBar'/>"
                    "<menuitem action='ToggleFlatButton'/>"
                    "<menuitem action='ToggleDrawToolbarback'/>"
                    "<menuitem action='TogglePostMark'/>"
                    "<separator/>"
                    "<menu action='Since_Menu'>"
                        "<menuitem action='Since_Normal'/>"
                        "<menuitem action='Since_NoYear'/>"
                        "<menuitem action='Since_Week'/>"
                        "<menuitem action='Since_Second'/>"
                        "<menuitem action='Since_Passed'/>"
                    "</menu>"
                    "<menu action='Write_Menu'>"
                        "<menuitem action='Write_Normal'/>"
                        "<menuitem action='Write_NoYear'/>"
                        "<menuitem action='Write_Week'/>"
                        "<menuitem action='Write_Second'/>"
                        "<menuitem action='Write_Passed'/>"
                    "</menu>"
                    "<menu action='Access_Menu'>"
                        "<menuitem action='Access_Normal'/>"
                        "<menuitem action='Access_NoYear'/>"
                        "<menuitem action='Access_Week'/>"
                        "<menuitem action='Access_Second'/>"
                        "<menuitem action='Access_Passed'/>"
                    "</menu>"
                "</menu>"
                "<separator/>"

                "<menu action='Tab_Menu'>"
                    "<menuitem action='TabBoard'/>"
                    "<menuitem action='TabArticle'/>"
                "</menu>"
                "<separator/>"

                "<menu action='Toolbar_Menu'>"
                    "<menuitem action='ShowToolBarMain'/>"
                    "<menuitem action='ShowToolBarBbslist'/>"
                    "<menuitem action='ShowToolBarBoard'/>"
                    "<menuitem action='ShowToolBarArticle'/>"
                    "<separator/>"
                    "<menu action='Toolbar_Main_Menu'>"
                        "<menuitem action='ToolbarPos0'/>"
                        "<menuitem action='ToolbarPos1'/>"
                    "</menu>"
                "</menu>"
                "<separator/>"

                "<menu action='Item_Menu'>"
                    "<menuitem action='SetupMainItem'/>"
                    "<menuitem action='SetupSidebarItem'/>"
                    "<menuitem action='SetupBoardItem'/>"
                    "<menuitem action='SetupArticleItem'/>"
                    "<menuitem action='SetupSearchItem'/>"
                    "<menuitem action='SetupMsgItem'/>"
                "</menu>"
                "<separator/>"

                "<menu action='ListItem_Menu'>"
                    "<menuitem action='SetupBoardItemColumn'/>"
                "</menu>"
                "<separator/>"

                "<menu action='MenuItem_Menu'>"
                    "<menuitem action='SetupBoardItemMenu'/>"
                    "<menuitem action='SetupArticleItemMenu'/>"
                "</menu>"
                "<separator/>";

    str_ui += menu_font;
    str_ui +=
                "<separator/>"

                "<menu action='ImageView_Menu'>"
                    "<menu action='ShowImageView_Menu'>"
                        "<menuitem action='UseWinImg'/>"
                        "<menuitem action='UseEmbImg'/>"
                        "<menuitem action='NoUseImg'/>"
                    "</menu>"
                    "<menuitem action='UseMosaic'/>"
                    "<menuitem action='UseImgPopup'/>"
                    "<menuitem action='UseInlineImg'/>"
                    "<menuitem action='ShowSsspIcon'/>"
                "</menu>"
                "<separator/>"

                "<menu action='MessageView_Menu'>"
                    "<menu action='ShowMsgView_Menu'>"
                        "<menuitem action='UseWinMsg'/>"
                        "<menuitem action='UseEmbMsg'/>"
                    "</menu>"
                    "<menuitem action='ToggleWrap'/>"
                "</menu>"

            "</menu>"

        "</menu>"

    // 履歴
        "<menu action='Menu_History'>"
            "<menuitem action='PrevView'/>"
            "<menuitem action='NextView'/>"
        "</menu>"

    // ツール
        "<menu action='Menu_Tool'>"

            "<menuitem action='LiveStartStop'/>"
            "<separator/>"
            "<menuitem action='SearchTitle'/>"
            "<separator/>"

            "<menu action='SearchCache_Menu'>"
                "<menuitem action='SearchCacheBoard'/>"
                "<menuitem action='SearchCache'/>"
            "</menu>"
            "<separator/>"

            "<menu action='ShowCache_Menu'>"
                "<menuitem action='ShowCacheBoard'/>"
                "<menuitem action='ShowCache'/>"
            "</menu>"
            "<separator/>"

            "<menu action='CheckUpdate_Menu'>"
                "<menuitem action='CheckUpdateRoot'/>"
                "<menuitem action='CheckUpdateOpenRoot'/>"
                "<separator/>"
                "<menuitem action='CancelCheckUpdate'/>"
            "</menu>"
            "<menuitem action='ShowSidebarBoard'/>"
            "<menuitem action='CreateVBoard'/>"
            "<separator/>"

            "<menuitem action='EditFavorite'/>"
            "<separator/>"
            "<menuitem action='ShowPostlog'/>"
            "<separator/>"
            "<menuitem action='ImportDat'/>"

        "</menu>"

    // 設定
        "<menu action='Menu_Config'>"

            "<menu action='Property_Menu'>"
                "<menuitem action='BbslistPref'/>"
                "<menuitem action='BoardPref'/>"
                "<menuitem action='ArticlePref'/>"
                "<menuitem action='ImagePref'/>"
            "</menu>"
            "<separator/>"

            "<menu action='General_Menu'>"
                "<menuitem action='RestoreViews'/>"
                "<menuitem action='ToggleFoldMessage'/>"
                "<menuitem action='SelectItemSync'/>"
                "<separator/>"
                "<menuitem action='SavePostLog'/>"
                "<menuitem action='SavePostHist'/>"
                "<separator/>"
                "<menuitem action='ShowMachiID'/>"
            "</menu>"
            "<separator/>"

            "<menu action='Mouse_Menu'>"
                "<menuitem action='ToggleTab'/>"
                "<menuitem action='TogglePopupWarp'/>"
                "<menuitem action='ShortMarginPopup'/>"
                "<separator/>"
                "<menuitem action='ToggleEmacsMode'/>"
                "<separator/>"
                "<menuitem action='KeyPref'/>"
                "<menuitem action='MousePref'/>"
                "<menuitem action='ButtonPref'/>"
            "</menu>"
            "<separator/>";

    str_ui += menu_font;
    str_ui +=
            "<separator/>"

            "<menu action='Net_Menu'>"
                "<menuitem action='SetupProxy'/>"
                "<menuitem action='SetupBrowser'/>"
                "<menuitem action='SetupPasswd'/>"
                "<separator/>"
                "<menuitem action='ToggleIPv6'/>"
            "</menu>"
            "<separator/>"

            "<menu action='Abone_Menu'>"
                "<menuitem action='SetupAbone'/>"
                "<menuitem action='SetupAboneThread'/>"
                "<separator/>"
                "<menuitem action='TranspChainAbone'/>"
                "<menuitem action='IcaseWcharAbone'/>"
                "<menuitem action='ShowAboneReason'/>"
            "</menu>"
            "<separator/>"

    // プライバシー
            "<menu action='Privacy_Menu'>"
                "<menuitem action='ClearAllPrivacy'/>"
                "<separator/>"
                "<menuitem action='ClearPostLog'/>"
                "<menuitem action='ClearPostHist'/>"
                "<separator/>"
                "<menuitem action='DeleteImages'/>"
            "</menu>"
            "<separator/>"

    // その他
            "<menu action='Etc_Menu'>"
                "<menuitem action='LivePref'/>"
                "<menuitem action='UsrCmdPref'/>"
                "<menuitem action='FilterPref'/>"
                "<menuitem action='ReplacePref'/>"
            "</menu>"
            "<separator/>"
            "<menuitem action='AboutConfig'/>"

        "</menu>"

    // ヘルプ
        "<menu action='Menu_Help'>"
            "<menuitem action='Manual'/>"
            "<separator/>"
            "<menuitem action='Bbs'/>"
            "<menuitem action='OldLog'/>"
            "<separator/>"
            "<menuitem action='About'/>"
        "</menu>"

        "</menubar>"
        "</ui>";

    m_ui_manager->add_ui_from_string( str_ui );
    m_menubar = dynamic_cast< Gtk::MenuBar* >( m_ui_manager->get_widget("/menu_bar") );
    assert( m_menubar );
    m_menubar->set_size_request( 0 );

    // 履歴メニュー追加
    const auto items = m_menubar->get_children();
    auto item = dynamic_cast< Gtk::MenuItem* >( *std::next( items.begin(), 2 ) );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_activate_historymenu ) );

    Gtk::Menu* submenu = item->get_submenu();

    submenu->append( *Gtk::manage( new Gtk::SeparatorMenuItem() ) );

    // スレ履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_thread() );

    // 板履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_board() );

    // 最近閉じたスレ履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_close() );

    // 最近閉じた板履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_closeboard() );

    // 最近閉じた画像履歴
    submenu->append( *HISTORY::get_history_manager()->get_menu_closeimg() );

    submenu->show_all_children();

    // メニューにショートカットキーやマウスジェスチャを表示
    for( auto&& widget : items ) {
        auto menu_item = dynamic_cast< Gtk::MenuItem* >( widget );
        CONTROL::set_menu_motion( menu_item->get_submenu() );
        menu_item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_activate_menubar ) );
    }
}
