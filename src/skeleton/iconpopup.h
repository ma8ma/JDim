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
            set_decorated( false );
            set_app_paintable( true );
            auto visual = get_screen()->get_rgba_visual();
            gtk_widget_set_visual( GTK_WIDGET( gobj() ), visual->gobj() );
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
