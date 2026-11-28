// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "tablabel.h"

#include "icons/iconmanager.h"

#include "config/globalconf.h"

#include "jdlib/gtk_compat.h"

#include "dndmanager.h"

using namespace SKELETON;

enum
{
    SPACING_LABEL = 3 // アイコンとラベルの間のスペース
};


TabLabel::TabLabel( const std::string& url )
    : m_url( url )
    , m_hbox{ JDLIB::compat::Orientation::HORIZONTAL, 0 }
    , m_id_icon( ICON::NUM_ICONS )
{
#ifdef _DEBUG
    std::cout << "TabLabel::TabLabel " << m_url << std::endl;
#endif

#ifdef USE_GTKMM4
    m_hbox.set_hexpand( true );
    m_hbox.set_vexpand( true );
    append( m_hbox );

    JDLIB::compat::box_append_shrink( m_hbox, m_label );
#else
    // 背景透過
    set_visible_window( false );

    add_events( Gdk::POINTER_MOTION_MASK );
    add_events( Gdk::LEAVE_NOTIFY_MASK );
    add_events( Gdk::SMOOTH_SCROLL_MASK ); // マウスホイールによるタブの切り替え

    add( m_hbox );
    JDLIB::compat::box_append_shrink( m_hbox, m_label );

    show_all_children();
#endif
}


// アイコンセット
void TabLabel::set_id_icon( const int id )
{
    if( ! CONFIG::get_show_tab_icon() ) return;
    if( m_id_icon == id ) return;
    if( id == ICON::NONE ) return;

    if( !m_image ){
        m_image = std::make_unique<Gtk::Image>();
        m_hbox.remove( m_label );
        m_hbox.set_spacing( SPACING_LABEL );
        JDLIB::compat::box_append_shrink( m_hbox, *m_image );
        JDLIB::compat::box_append_shrink( m_hbox, m_label );
        show_all_children();
    }

#ifdef _DEBUG
    std::cout << "TabLabel::set_icon " <<  m_fulltext << " id = " << id << std::endl;
#endif

    m_id_icon = id;

    m_image->set( ICON::get_pixbuf( id ) );
}


// タブの文字列の文字数がlngになるようにリサイズする
void TabLabel::resize_tab( const unsigned int lng )
{
    Glib::ustring ulabel( m_fulltext );
    const unsigned int lng_org = ulabel.length();
    ulabel.resize( lng );
    if( lng < lng_org ) ulabel += "...";
    m_label.set_text( ulabel );
}


//
// D&D設定
//
void TabLabel::set_dragable( bool dragable, int button )
{
    // TODO: GTK4 [DnD] set_dragable() は未実装。タブの並べ替えは動かない。Gtk::DragSource と Gtk::DropTarget に移す。
#ifndef USE_GTKMM4
    if( dragable ){

        std::vector< Gtk::TargetEntry > targets;
        targets.push_back( Gtk::TargetEntry( DNDTARGET_FAVORITE, Gtk::TARGET_SAME_APP, 0 ) );
        targets.push_back( Gtk::TargetEntry( DNDTARGET_TAB, Gtk::TARGET_SAME_APP, 0 ) );

        // ドラッグ開始側にする
        switch( button ){

            case 1: drag_source_set( targets, Gdk::BUTTON1_MASK ); break;
            case 2: drag_source_set( targets, Gdk::BUTTON2_MASK ); break;
            case 3: drag_source_set( targets, Gdk::BUTTON3_MASK ); break;

            default: return;
        }
    }
    else{
        drag_source_unset();
        drag_dest_unset();
    }
#endif
}


//
// 本体の横幅 - ラベルの横幅
//
int TabLabel::get_label_margin() const
{
    int label_margin;

#ifdef USE_GTKMM4
    label_margin = m_label.get_margin_start() + m_label.get_margin_end()
#else
    int x_pad, y_pad;
    m_label.get_padding( x_pad, y_pad );
    label_margin = x_pad*2
#endif
        + m_hbox.get_spacing() + m_hbox.get_border_width()*2
        + get_border_width()*2;

    if( CONFIG::get_show_tab_icon() && m_id_icon < ICON::NUM_ICONS ) label_margin += ICON::get_pixbuf( m_id_icon )->get_width();

#ifdef _DEBUG
    std::cout << "image_w = " << ( m_image ? m_image->get_allocation().get_width() : 0 )
              << " label_w = "  << m_label.get_allocation().get_width()
              << " alloc_w =  " << get_allocation().get_width()
              << " label_margine = " << label_margin
              << std::endl;
#endif

    return label_margin;
}


//
// マウスが動いた
//
// TODO: GTK4 [Mouse] Gtk::EventControllerMotion::signal_motion に移行する。接続したハンドラは呼ばれない。
#ifndef USE_GTKMM4
bool TabLabel::on_motion_notify_event( GdkEventMotion* event )
{
    const bool ret = Gtk::EventBox::on_motion_notify_event( event );

    m_sig_tab_motion_event.emit();

    return ret;
}
#endif


//
// マウスが出た
//
// TODO: GTK4 [Mouse] Gtk::EventControllerMotion::signal_leave に移行する。接続したハンドラは呼ばれない。
#ifndef USE_GTKMM4
bool TabLabel::on_leave_notify_event( GdkEventCrossing* event )
{
#ifdef _DEBUG
    std::cout << "TabLabel::leave\n";
#endif

    const bool ret = Gtk::EventBox::on_leave_notify_event( event );

    m_sig_tab_leave_event.emit();

    return ret;
}
#endif


//
// ドラッグ開始
//
// TODO: GTK4 [DnD] Gtk::DragSource::signal_drag_begin に移行する。接続したハンドラは呼ばれない。
#ifndef USE_GTKMM4
void TabLabel::on_drag_begin( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_begin " << m_fulltext << std::endl;
#endif

    m_sig_tab_drag_begin.emit();

    Gtk::EventBox::on_drag_begin( context );
}
#endif


//
// D&Dで受信側がデータ送信を要求してきた
//
// TODO: GTK4 [DnD] Gtk::DragSource::signal_prepare に移行する。接続したハンドラは呼ばれない。
#ifndef USE_GTKMM4
void TabLabel::on_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                 Gtk::SelectionData& selection_data, guint info, guint time )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_data_get target = " << selection_data.get_target()
              << " " << m_fulltext << std::endl;
#endif

    Gtk::EventBox::on_drag_data_get( context, selection_data, info, time );

    m_sig_tab_drag_data_get( selection_data );
}
#endif


//
// ドラッグ終了
//
// TODO: GTK4 [DnD] Gtk::DragSource::signal_drag_end に移行する。接続したハンドラは呼ばれない。
#ifndef USE_GTKMM4
void TabLabel::on_drag_end( const Glib::RefPtr< Gdk::DragContext >& context )
{
#ifdef _DEBUG
    std::cout << "TabLabel::on_drag_end " << m_fulltext << std::endl;
#endif

    Gtk::EventBox::on_drag_end( context );

    m_sig_tab_drag_end.emit();
}
#endif
