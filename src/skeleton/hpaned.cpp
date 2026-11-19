// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "hpaned.h"

#include "jdlib/gtk_compat.h"

using namespace SKELETON;


JDHPaned::JDHPaned( const int fixmode )
    : Gtk::Paned( JDLIB::compat::Orientation::HORIZONTAL )
    , m_pctrl( *this, fixmode )
{}


JDHPaned::~JDHPaned() noexcept = default;


void JDHPaned::on_realize()
{
    Gtk::Paned::on_realize();
    m_pctrl.update_position();
}


#ifndef USE_GTKMM4
// TODO: GTK4 [Mouse] Gtk::GestureClick::signal_pressed に移行する。
bool JDHPaned::on_button_press_event( GdkEventButton* event )
{
    m_pctrl.button_press_event( event );
    return Gtk::Paned::on_button_press_event( event );
}


// TODO: GTK4 [Mouse] Gtk::GestureClick::signal_released に移行する。
bool JDHPaned::on_button_release_event( GdkEventButton* event )
{
    m_pctrl.button_release_event( event );   
    return Gtk::Paned::on_button_release_event( event );
}


// TODO: GTK4 [Mouse] Gtk::EventControllerMotion::signal_motion に移行する。
bool JDHPaned::on_motion_notify_event( GdkEventMotion* event )
{
    m_pctrl.motion_notify_event( event );
    return Gtk::Paned::on_motion_notify_event( event );
}


// TODO: GTK4 [Mouse] Gtk::EventControllerMotion::signal_enter に移行する。
bool JDHPaned::on_enter_notify_event( GdkEventCrossing* event )
{
    m_pctrl.enter_notify_event( event );
    return Gtk::Paned::on_enter_notify_event( event );
}

// TODO: GTK4 [Mouse] Gtk::EventControllerMotion::signal_leave に移行する。
bool JDHPaned::on_leave_notify_event( GdkEventCrossing* event )
{
    m_pctrl.leave_notify_event( event );
    return Gtk::Paned::on_leave_notify_event( event );
}
#endif // USE_GTKMM4
