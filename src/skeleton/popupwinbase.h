// ライセンス: GPL2

//
// ポップアップウィンドウの基底クラス
//

#ifndef _POPUPWINBASE_H
#define _POPUPWINBASE_H

#include "gtkmmversion.h"

#include <gtkmm.h>

namespace SKELETON
{
    enum
    {
        POPUPWIN_DRAWFRAME = true,
        POPUPWIN_NOFRAME = false
    };

    //
    // Gtk::Windows は signal_configure_event()を発行しないようなので
    // 自前でconfigureイベントをフックしてシグナルを発行する
    //
    typedef sigc::signal< void, int, int > SIG_CONFIGURED_POPUP;

    class PopupWinBase : public Gtk::Window
    {
        SIG_CONFIGURED_POPUP m_sig_configured;
#if !GTKMM_CHECK_VERSION(2,10,0)
        Glib::RefPtr< Gdk::GC > m_gc;
#endif

        bool m_draw_frame;

      public:

        // draw_frame == true なら枠を描画する
        PopupWinBase( bool draw_frame ) : Gtk::Window( Gtk::WINDOW_POPUP ), m_draw_frame( draw_frame ){
            if( m_draw_frame ) set_border_width( 1 );
        }
        virtual ~PopupWinBase(){}

        SIG_CONFIGURED_POPUP sig_configured(){ return m_sig_configured; }

      protected:

        virtual void on_realize()
        {
            Gtk::Window::on_realize();

#if !GTKMM_CHECK_VERSION(2,10,0)
            Glib::RefPtr< Gdk::Window > window = get_window();
            m_gc = Gdk::GC::create( window );    
#endif
        }

#if GTKMM_CHECK_VERSION(3,0,0)
        virtual bool on_draw( const Cairo::RefPtr< Cairo::Context >& cr ) override
        {
            const bool ret = Gtk::Window::on_draw( cr );
            if( m_draw_frame ) {
                Gdk::Cairo::set_source_rgba( cr, Gdk::RGBA( "black" ) );
                cr->set_line_width( 1.0 );
                cr->rectangle( 0.0, 0.0, get_width(), get_height() );
                cr->stroke();
            }
            return ret;
        }
#else
        virtual bool on_expose_event( GdkEventExpose* event )
        {
            bool ret = Gtk::Window::on_expose_event( event );

            // 枠の描画
            if( m_draw_frame ){
#if GTKMM_CHECK_VERSION(2,10,0)
                // cairomm 1.12.0 がメモリリークを起こしたので
                // C API を使うことで問題を回避する
                cairo_t* cr = gdk_cairo_create( get_window()->gobj() );
                gdk_cairo_set_source_color( cr, Gdk::Color( "black" ).gobj() );
                cairo_set_line_width( cr, 1.0 );
                cairo_rectangle( cr, 0.0, 0.0, get_width(), get_height() );
                cairo_stroke( cr );
                cairo_destroy( cr );
#else
                m_gc->set_foreground( Gdk::Color( "black" ) );
                get_window()->draw_rectangle( m_gc, false, 0, 0, get_width()-1, get_height()-1 );
#endif
            }

            return ret;
        }
#endif // GTKMM_CHECK_VERSION(3,0,0)

        virtual bool on_configure_event( GdkEventConfigure* event )
        {
            bool ret = Gtk::Window::on_configure_event( event );
            m_sig_configured.emit( get_width(), get_height() );

            return ret;
        }
    };
}

#endif
