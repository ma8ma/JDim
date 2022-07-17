// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "imageadmin.h"
#include "imageviewicon.h"
#include "imageareaicon.h"

#include "dbimg/img.h"

#include "jdlib/miscutil.h"

#include "control/controlid.h"

#include "type.h"
#include "dndmanager.h"
#include "sharedbuffer.h"
#include "cache.h"
#include "command.h"
#include "global.h"


using namespace IMAGE;


ImageViewIcon::ImageViewIcon( const std::string& url )
    : ImageViewBase( url )
    , m_provider{ Gtk::CssProvider::create() }
    , m_prev_n_tabs{ -1 }
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::ImageViewIcon : " << get_url() << std::endl;
#endif

    // コントロールモード設定
    get_control().add_mode( CONTROL::MODE_IMAGEICON );

    // $XDG_CONFIG_HOME/gtk-3.0/gtk.css から設定できるようにCSSセレクタを追加する (テスト用)
    set_name( "jdim-imageview-icon" );

    // 選択されてる画像アイコンの背景色を赤にするための設定
    pack_start( get_event() );
    get_event().set_border_width( 1 );
    try {
        m_provider->load_from_data( ".selected { background-color: red; }" );
    }
    catch( Gtk::CssProviderError& ) {
        std::terminate();
    }
    get_style_context()->add_provider( m_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );

    setup_common();

    // D&D可能にする
    std::vector< Gtk::TargetEntry > targets;
    targets.push_back( Gtk::TargetEntry( DNDTARGET_IMAGETAB, Gtk::TARGET_SAME_APP, 0 ) );
    get_event().drag_dest_set( targets );

    targets.push_back( Gtk::TargetEntry( DNDTARGET_FAVORITE, Gtk::TARGET_SAME_APP, 0 ) );
    get_event().drag_source_set( targets, Gdk::BUTTON1_MASK );

    get_event().signal_drag_begin().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_begin ) );
    get_event().signal_drag_data_get().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_data_get ) );
    get_event().signal_drag_data_received().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_data_received ) );
    get_event().signal_drag_end().connect( sigc::mem_fun( *this, &ImageViewIcon::slot_drag_end ) );
}


ImageViewIcon::~ImageViewIcon()
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::~ImageViewIcon : " << get_url() << std::endl;
#endif

    // スレッドを止めるために明示的にクリアする
    remove_imagearea();
}



//
// クロック入力
//
void ImageViewIcon::clock_in()
{
    View::clock_in();

    // 待ち状態
    if( is_wait() && ! get_img()->is_wait() ){

        set_wait( false );
        set_loading( get_img()->is_loading() );
        show_view();
    }

    // ロード終了
    else if( get_imagearea() && is_loading() && ! get_img()->is_loading() ){

        set_loading( false );
        show_view();
    }
}



//
// フォーカスイン
//
void ImageViewIcon::focus_view()
{
    get_style_context()->add_class( "selected" );
    get_event().grab_focus();
}


//
// フォーカスアウト
//
void ImageViewIcon::focus_out()
{
    SKELETON::View::focus_out();
    get_style_context()->remove_class( "selected" );
}



//
// 表示
//
void ImageViewIcon::show_view()
{
    if( is_loading() ) return;
    if( is_wait() ) return;

#ifdef _DEBUG
    std::cout << "ImageViewIcon::show_view url = " << get_url() << std::endl;
#endif    

    // 待ち状態
    if( get_img()->is_wait() ) set_wait( true );

    // 読み込み中        
    else if( get_img()->is_loading() ) set_loading( true );

    // 画像が既に表示しているなら再描画
    if( get_imagearea() ) get_imagearea()->show_image();

    // 画像貼り付け
    // ロード中の時はclock_in()経由でもう一度 show_view()が呼び出される
    else{

        set_imagearea( Gtk::manage( new ImageAreaIcon( get_url() ) ) );
        get_imagearea()->show_image();
        show_all_children();
    }
}



//
// アイコンが切り替わった
//
void ImageViewIcon::switch_icon()
{
    get_style_context()->add_class( "selected" );
}




//
// ポップアップメニュー取得
//
// SKELETON::View::show_popupmenu() を参照すること
//
Gtk::Menu* ImageViewIcon::get_popupmenu( const std::string& url )
{
    // 初回の呼び出し時にメニューモデルをバインドする
    if( ! m_popup_menu.get_attach_widget() ) {
        m_menumodel = Glib::RefPtr<Gio::Menu>::cast_dynamic( m_builder->get_object( "popup_menu_icon" ) );
        m_popup_menu.bind_model( m_menumodel, true );
        m_popup_menu.attach_to_widget( *this );

        // タブ切り替えサブメニュー
        auto submenu = Glib::RefPtr<Gio::MenuModel>::cast_dynamic( m_builder->get_object( "move_submenu" ) );
        m_item_sub = Gio::MenuItem::create( ITEM_NAME_GO, submenu );

        m_menumodel->prepend_item( m_item_sub );
    }

    // メニュー項目ラベルの更新は重いためタブ数が変わらないときは更新しない
    const int n_tabs = IMAGE::get_admin()->get_tab_nums();
    if( n_tabs != m_prev_n_tabs ) {
        assert( m_menumodel );
        // Gio::Menuを使うとメニューに追加された項目のラベルは変更できない
        // 動的なメニュー項目を実現するには一旦メニューから項目を取り除く必要がある
        m_menumodel->remove( 0 );
        auto label = Glib::ustring::compose( ITEM_NAME_GO " [ タブ数 %1 ](_M)", n_tabs );
        m_item_sub->set_label( label );
        m_menumodel->prepend_item( m_item_sub );

        m_prev_n_tabs = n_tabs;
    }

    return &m_popup_menu;
}



//
// D&D開始
//
void ImageViewIcon::slot_drag_begin( const Glib::RefPtr<Gdk::DragContext>& context )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_begin url = " << get_url() << std::endl;
#endif

    CORE::DND_Begin();
}



//
// D&Dで受信側がデータ送信を要求してきた
//
void ImageViewIcon::slot_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                        Gtk::SelectionData& selection_data, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "ImageViewIcon::on_drag_data_get target = " << selection_data.get_target()
              << " url = " << get_url() << std::endl;;
#endif

    set_image_to_buffer();
    selection_data.set( selection_data.get_target(), get_url() );
}


//
// 他の画像アイコンからドロップされた
//
void ImageViewIcon::slot_drag_data_received( const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                             const Gtk::SelectionData& selection_data, guint info, guint time )
{
    const std::string url_from = selection_data.get_data_as_string();

#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_data_received target = " << selection_data.get_target()
              << " url = " << get_url() << " url_from = " << url_from << std::endl;    
#endif

    IMAGE::get_admin()->set_command( "reorder", get_url(), url_from, get_url() );
}


//
// D&D終了
//
void ImageViewIcon::slot_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG    
    std::cout << "ImageViewIcon::slot_drag_end url = " << get_url() << std::endl;
#endif

    CORE::DND_End();
}



