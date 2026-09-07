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


#define SIZE_MENU { 25, 50, 75, 100, 150, 200, 400 }


/**
 * @brief ImageViewBase のコンテキストメニューを構築する。
 *
 * ImageViewBase::setup_common() から呼び出され、コンテキストメニューの生成と
 * アクションの登録を行います。
 */
void IMAGE::ImageViewBase::setup_popupmenu()
{
    // ポップアップメニューの設定
    // アクショングループを作ってUIマネージャに登録
    action_group() = Gtk::ActionGroup::create();
    action_group()->add( Gtk::Action::create( "CancelMosaic", "CancelMosaic"),
                         sigc::mem_fun( *this, &ImageViewBase::slot_cancel_mosaic ) );
    action_group()->add( Gtk::Action::create( "ShowLargeImg", "サイズが大きい画像を表示(_G)"),
                         sigc::mem_fun( *this, &ImageViewBase::slot_show_large_img ) );
    action_group()->add( Gtk::Action::create( "LoadStop", "StopLoading" ), sigc::mem_fun( *this, &ImageViewBase::stop ) );
    action_group()->add( Gtk::Action::create( "Reload", "強制再読み込み(_E)"), sigc::mem_fun( *this, &ImageViewBase::slot_reload_force ) );
    action_group()->add( Gtk::Action::create( "AppendFavorite", "AppendFavorite"), sigc::mem_fun( *this, &ImageViewBase::slot_favorite ) );

    action_group()->add( Gtk::Action::create( "ZoomFitImage", "ZoomFitImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_fit_win ) );
    action_group()->add( Gtk::Action::create( "ZoomInImage", "ZoomInImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_zoom_in ) );
    action_group()->add( Gtk::Action::create( "ZoomOutImage", "ZoomOutImage" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_zoom_out ) );
    action_group()->add( Gtk::Action::create( "OrgSizeImage", "OrgSizeImage" ),
                         sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_resize_image ), 100 ) );

    action_group()->add( Gtk::Action::create( "Size_Menu", "サイズ変更(_R)" ) );

    // サイズ
    unsigned int size[] = SIZE_MENU;
    for( unsigned int i = 0; i < sizeof( size )/sizeof( unsigned int ) ; ++i ){
        int tmp_size = size[ i ];
        std::string str_size = std::to_string( tmp_size );
        //ショートカットは、1から始まる
        std::string str_shortcut = "(_" + std::to_string( i+1 ) + ")";
        Glib::RefPtr< Gtk::Action > action = Gtk::Action::create( "Size" + str_size, str_size + "%" + str_shortcut );
        action_group()->add( action, sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_resize_image ), tmp_size ) );
    }

    action_group()->add( Gtk::Action::create( "Move_Menu", ITEM_NAME_GO "(_M)" ) );
    action_group()->add( Gtk::Action::create( "MoveHead", "先頭に移動(_H)" ), sigc::mem_fun( *this, &ImageViewBase::slot_move_head ) );
    action_group()->add( Gtk::Action::create( "MoveTail", "最後に移動(_T)" ), sigc::mem_fun( *this, &ImageViewBase::slot_move_tail ) );

    action_group()->add( Gtk::Action::create( "Quit", "Quit" ), sigc::mem_fun( *this, &ImageViewBase::close_view ) );

    action_group()->add( Gtk::Action::create( "Close_Menu", "複数の画像を閉じる(_L)" ) );
    action_group()->add( Gtk::Action::create( "CloseOther", "他の画像(_O)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_other_views ) );
    action_group()->add( Gtk::Action::create( "CloseLeft", "左←の画像(_L)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_left_views ) );
    action_group()->add( Gtk::Action::create( "CloseRight", "右→の画像(_R)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_right_views ) );
    action_group()->add( Gtk::Action::create( "CloseError404", "エラー画像(404,403のみ)(_E)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseError503", "エラー画像(timeout,503以外)(_T)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_notimeout_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseErrorAll", "エラー画像(読込み中含め全て)(_W)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_all_error_views ) );
    action_group()->add( Gtk::Action::create( "CloseNoError", "エラー以外の画像(_N)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_noerror_views ) );
    action_group()->add( Gtk::Action::create( "CloseAll", "全ての画像(_A)" ), sigc::mem_fun( *this, &ImageViewBase::slot_close_all_views ) );

    action_group()->add( Gtk::ToggleAction::create( "LockTab", "タブをロックする(_K)", std::string(), false ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_lock ) );

    action_group()->add( Gtk::Action::create( "OpenBrowser", ITEM_NAME_OPEN_BROWSER "(_W)" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_open_browser ) );
    action_group()->add( Gtk::Action::create( "OpenCacheBrowser", ITEM_NAME_OPEN_CACHE_BROWSER "(_X)" ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_open_cache_browser ) );
    action_group()->add( Gtk::Action::create( "OpenRef", "参照元のレスを開く(_O)"), sigc::mem_fun( *this, &ImageViewBase::slot_open_ref ) );
    action_group()->add( Gtk::Action::create( "CopyURL", ITEM_NAME_COPY_URL "(_U)" ), sigc::mem_fun( *this, &ImageViewBase::slot_copy_url ) );
    action_group()->add( Gtk::Action::create( "Save", "Save"), sigc::mem_fun( *this, &ImageViewBase::slot_save ) );
    action_group()->add( Gtk::Action::create( "SaveAll", "全ての画像を保存(_A)..."), sigc::mem_fun( *this, &ImageViewBase::slot_save_all ) );

    action_group()->add( Gtk::Action::create( "DeleteMenu", "Delete" ) );
    action_group()->add( Gtk::Action::create( "DeleteImage", "削除する(_D)"), sigc::mem_fun( *this, &ImageViewBase::delete_view ) );
    action_group()->add( Gtk::ToggleAction::create( "ProtectImage", "キャッシュを保護する(_H)", std::string(), false ),
                         sigc::mem_fun( *this, &ImageViewBase::slot_toggle_protectimage ) );

    action_group()->add( Gtk::Action::create( "AboneImage", "画像をあぼ〜んする(_B)"), sigc::mem_fun( *this, &ImageViewBase::slot_abone_img ) );

    action_group()->add( Gtk::Action::create( "PreferenceImage", "PreferenceImage"), sigc::mem_fun( *this, &ImageViewBase::show_preference ) );
    action_group()->add( Gtk::Action::create( "Preference", "プロパティ(_P)..."), sigc::mem_fun( *this, &ImageViewBase::show_preference ) );

    const std::string usrcmd = CORE::get_usrcmd_manager()->create_usrcmd_menu( action_group() );
    const int usrcmd_size = CORE::get_usrcmd_manager()->get_size();
    for( int i = 0; i < usrcmd_size; ++i ){
        Glib::RefPtr< Gtk::Action > act = CORE::get_usrcmd_manager()->get_action( action_group(), i );
        if( act ) act->signal_activate().connect(
            sigc::bind< int >( sigc::mem_fun( *this, &ImageViewBase::slot_usrcmd ), i ) );
    }

    ui_manager() = Gtk::UIManager::create();
    ui_manager()->insert_action_group( action_group() );

    // 画像ビューのメニュー
    const std::string menu =

    "<popup name='popup_menu'>"

    "<menuitem action='CancelMosaic'/>"
    "<menuitem action='ShowLargeImg'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='Size_Menu'>"
    "<menuitem action='Size25'/>"
    "<menuitem action='Size50'/>"
    "<menuitem action='Size75'/>"
    "<menuitem action='Size100'/>"
    "<menuitem action='Size150'/>"
    "<menuitem action='Size200'/>"
    "<menuitem action='Size400'/>"
    "</menu>"
    "<menuitem action='OrgSizeImage'/>"
    "<menuitem action='ZoomFitImage'/>"
    "<menuitem action='ZoomInImage'/>"
    "<menuitem action='ZoomOutImage'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<menuitem action='OpenCacheBrowser'/>"
    "<menuitem action='OpenRef'/>"
    + usrcmd
    + std::string(
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='AppendFavorite'/>"
    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='LoadStop'/>"
    "<menuitem action='Reload'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"

    "<menuitem action='PreferenceImage'/>"

    "</popup>"
    );

    // アイコンのメニュー
    const std::string menu_icon =

    "<popup name='popup_menu_icon'>"

    "<menu action='Move_Menu'>"
    "<menuitem action='MoveHead'/>"
    "<menuitem action='MoveTail'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='LockTab'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menu action='Close_Menu'>"
    "<menuitem action='CloseAll'/>"
    "<menuitem action='CloseOther'/>"
    "<menuitem action='CloseLeft'/>"
    "<menuitem action='CloseRight'/>"
    "<separator/>"
    "<menuitem action='CloseError404'/>"
    "<menuitem action='CloseError503'/>"
    "<menuitem action='CloseErrorAll'/>"
    "<separator/>"
    "<menuitem action='CloseNoError'/>"
    "</menu>"
    "<separator/>"

    "<menuitem action='SaveAll'/>"
    "<separator/>"

    "<menuitem action='OpenBrowser'/>"
    "<menuitem action='OpenCacheBrowser'/>"
    "<menuitem action='OpenRef'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='AppendFavorite'/>"
    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='LoadStop'/>"
    "<menuitem action='Reload'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"
    "<separator/>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"
    "<menuitem action='Preference'/>"

    "</popup>";

    // 画像ポップアップのメニュー
    const std::string menu_popup =

    "<popup name='popup_menu_popup'>"

    "<menuitem action='CancelMosaic'/>"
    "<separator/>"

    "<menuitem action='Quit'/>"
    "<separator/>"

    "<menuitem action='CopyURL'/>"
    "<separator/>"

    "<menuitem action='Save'/>"
    "<separator/>"

    "<menuitem action='ProtectImage'/>"
    "<menu action='DeleteMenu'>"
    "<menuitem action='DeleteImage'/>"
    "</menu>"

    "<separator/>"
    "<menuitem action='AboneImage'/>"

    "<separator/>"

    "<menuitem action='Preference'/>"

    "</popup>"
    ;


    ui_manager()->add_ui_from_string(
        "<ui>"
        + menu
        + menu_icon
        + menu_popup
        + "</ui>"
        );

    // ポップアップメニューにキーアクセレータやマウスジェスチャを表示
    Gtk::Menu* popupmenu = get_popupmenu_impl( "/popup_menu" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = get_popupmenu_impl( "/popup_menu_icon" );
    CONTROL::set_menu_motion( popupmenu );

    popupmenu = get_popupmenu_impl( "/popup_menu_popup" );
    CONTROL::set_menu_motion( popupmenu );
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
void IMAGE::ImageViewBase::activate_act_before_popupmenu( const std::string& url )
{
    if( !m_img ) return;

    // toggle　アクションを activeにするとスロット関数が呼ばれるので処理しないようにする
    m_enable_menuslot = false;

    Glib::RefPtr< Gtk::Action > act;

    bool current_protect = m_img->is_protected();

    // ロック
    act = action_group()->get_action( "LockTab" );
    if( act ){

        auto tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
        if( is_locked() ) tact->set_active( true );
        else tact->set_active( false );
    }

    // 閉じる
    act = action_group()->get_action( "Quit" );
    if( act ){
        if( is_locked() ) act->set_sensitive( false );
        else act->set_sensitive( true );
    }

    // モザイク
    act = action_group()->get_action( "CancelMosaic" );
    if( act ){
        if( m_img->is_cached() && m_img->get_mosaic() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // サイズの大きい画像を表示
    act = action_group()->get_action( "ShowLargeImg" );
    if( act ){
        if( m_img->get_type() == DBIMG::T_LARGE ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // サイズ系メニュー、お気に入り、保存
    constexpr const char* sizemenus[] =
    {
        "Size_Menu",
        "OrgSizeImage",
        "ZoomFitImage",
        "ZoomInImage",
        "ZoomOutImage",
        "AppendFavorite",
        "Save"
    };
    for( const char* menu : sizemenus ) {
        act = action_group()->get_action( menu );
        if( act ){
            if( m_img->is_cached() ) act->set_sensitive( true );
            else act->set_sensitive( false );
        }
    }

    // キャッシュをブラウザで開く
    act = action_group()->get_action( "OpenCacheBrowser" );
    if( act ){
        if( m_img->is_cached() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 参照元スレ
    act = action_group()->get_action( "OpenRef" );
    if( act ){
        if( ! m_img->get_refurl().empty() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // 保護
    act = action_group()->get_action( "ProtectImage" );
    if( act ){

        if( m_img->is_cached() ){

            act->set_sensitive( true );

            auto tact = Glib::RefPtr< Gtk::ToggleAction >::cast_dynamic( act );
            if( tact ){
                if( current_protect ) tact->set_active( true );
                else tact->set_active( false );
            }
        }
        else act->set_sensitive( false );
    }

    // 削除
    act = action_group()->get_action( "DeleteMenu" );
    if( act ){
        if(  m_img->get_code() != HTTP_INIT && ! m_img->is_protected() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // ロード停止
    act = action_group()->get_action( "LoadStop" );
    if( act ){
        if( m_img->is_loading() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // あぼーん
    act = action_group()->get_action( "AboneImage" );
    if( act ){
        if( ! m_img->is_protected() ) act->set_sensitive( true );
        else act->set_sensitive( false );
    }

    // ユーザコマンド
    // 選択不可かどうか判断して visible か sensitive にする
    const std::string url_article = DBTREE::url_dat( m_img->get_refurl() );
    CORE::get_usrcmd_manager()->toggle_sensitive( action_group(), url_article, get_url(), "" );

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
    return dynamic_cast<Gtk::Menu*>( ui_manager()->get_widget( menu_name ) );
}
