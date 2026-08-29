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
    auto top_menu = Gtk::make_managed<Gtk::MenuItem>( "履歴(_S)", true );
    auto submenu = Gtk::make_managed<Gtk::Menu>();

    auto item = Gtk::make_managed<Gtk::MenuItem>( "前へ戻る(_P)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_prevview ) );
    submenu->append( *item );

    item = Gtk::make_managed<Gtk::MenuItem>( "次へ進む(_N)", true );
    item->signal_activate().connect( sigc::mem_fun( *this, &Core::slot_nextview ) );
    submenu->append( *item );

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

    // FIXME: GTK4 動的な履歴メニューは Gtk::ActionGroup に依存しているため
    // フェーズ1では未実装とする。
}
