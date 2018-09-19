// ライセンス: GPL2

#ifndef _ICONPOPUP_H
#define _ICONPOPUP_H

#include <gtkmm.h>

#include "icons/iconmanager.h"

namespace SKELETON
{
    class IconPopup : public Gtk::Window
    {
        Glib::RefPtr< Gdk::Pixbuf > m_pixbuf;
        Gtk::Image m_img;

      public:

      IconPopup( const int icon_id ) : Gtk::Window( Gtk::WINDOW_POPUP ){

            m_pixbuf = ICON::get_icon( icon_id );
            m_img.set( m_pixbuf );

#if GTKMM_CHECK_VERSION(3,0,0)
            cairo_t* cr = gdk_cairo_create( m_img.get_window()->gobj() );
            cairo_region_t* region =
                gdk_cairo_region_create_from_surface( cairo_get_target( cr ) );

            gtk_widget_shape_combine_region( GTK_WIDGET( gobj() ), region );

            cairo_region_destroy( region );
            cairo_destroy( cr );
#else
            Glib::RefPtr< Gdk::Pixmap > pixmap;
            Glib::RefPtr< Gdk::Bitmap > bitmap;

            m_pixbuf->render_pixmap_and_mask( pixmap, bitmap, 255 );
            shape_combine_mask( bitmap, 0, 0 );
#endif // GTKMM_CHECK_VERSION(3,0,0)

            add( m_img );
            show_all_children();
        }

        const int get_img_width(){ return m_pixbuf->get_width(); }
        const int get_img_height(){ return m_pixbuf->get_height(); }
    };

}

#endif
