// ライセンス: GPL2

#ifndef _TABLABEL_H
#define _TABLABEL_H

#include "gtkmmversion.h"

#include <gtkmm.h>
#include <string>

namespace SKELETON
{
#if GTKMM_CHECK_VERSION(2,10,0)
    typedef sigc::signal< bool, GdkEventButton*, Gtk::Widget* >
        SIG_TAB_BUTTON_PRESS_EVENT;
    typedef sigc::signal< bool, GdkEventButton*, Gtk::Widget* >
        SIG_TAB_BUTTON_RELEASE_EVENT;
#else
    // マウス
    typedef sigc::signal< void > SIG_TAB_MOTION_EVENT;
    typedef sigc::signal< void > SIG_TAB_LEAVE_EVENT;

    // D&D
    typedef sigc::signal< void > SIG_TAB_DRAG_BEGIN;
    typedef sigc::signal< void, Gtk::SelectionData& > SIG_TAB_DRAG_DATA_GET;
    typedef sigc::signal< void > SIG_TAB_DRAG_END;
#endif // GTKMM_CHECK_VERSION(2,10,0)

    class TabLabel : public Gtk::EventBox
    {
#if GTKMM_CHECK_VERSION(2,10,0)
        SIG_TAB_BUTTON_PRESS_EVENT m_sig_tab_button_press_event;
        SIG_TAB_BUTTON_RELEASE_EVENT m_sig_tab_button_release_event;
#else

        SIG_TAB_MOTION_EVENT m_sig_tab_motion_event;
        SIG_TAB_LEAVE_EVENT m_sig_tab_leave_event;

        SIG_TAB_DRAG_BEGIN m_sig_tab_drag_begin;
        SIG_TAB_DRAG_DATA_GET m_sig_tab_drag_data_get;
        SIG_TAB_DRAG_END m_sig_tab_drag_end;
#endif // GTKMM_CHECK_VERSION(2,10,0)

#if !GTKMM_CHECK_VERSION(2,10,0)
        int m_x;
        int m_y;
        int m_width;
        int m_height;
#endif // !GTKMM_CHECK_VERSION(2,10,0)

        std::string m_url;
        Gtk::HBox m_hbox;
        int m_id_icon;

        Gtk::Label m_label;

        // アイコン画像
        Gtk::Image* m_image;

        // ラベルに表示する文字列の全体
        std::string m_fulltext;

      public:

        TabLabel( const std::string& url );
        virtual ~TabLabel();

#if GTKMM_CHECK_VERSION(2,10,0)
        SIG_TAB_BUTTON_PRESS_EVENT sig_tab_button_press_event()
        {
            return m_sig_tab_button_press_event;
        }
        SIG_TAB_BUTTON_RELEASE_EVENT sig_tab_button_release_event()
        {
            return m_sig_tab_button_release_event;
        }
#else
        SIG_TAB_MOTION_EVENT sig_tab_motion_event(){ return  m_sig_tab_motion_event; }
        SIG_TAB_LEAVE_EVENT sig_tab_leave_event(){ return m_sig_tab_leave_event; }

        SIG_TAB_DRAG_BEGIN sig_tab_drag_begin() { return m_sig_tab_drag_begin; }
        SIG_TAB_DRAG_DATA_GET sig_tab_drag_data_get() { return m_sig_tab_drag_data_get; }
        SIG_TAB_DRAG_END sig_tab_drag_end() { return m_sig_tab_drag_end; }
#endif // GTKMM_CHECK_VERSION(2,10,0)

#if !GTKMM_CHECK_VERSION(2,10,0)
        const int get_tab_x() const { return m_x; }
        const int get_tab_y() const { return m_y; }
        const int get_tab_width() const { return m_width; }
        const int get_tab_height() const { return m_height; }

        void set_tab_x( const int x ){ m_x = x; }
        void set_tab_y( const int y ){ m_y = y; }
        void set_tab_width( const int width ){ m_width = width; }
        void set_tab_height( const int height ){ m_height = height; }
#endif // !GTKMM_CHECK_VERSION(2,10,0)

        Pango::FontDescription get_label_font_description(){
            return m_label.get_pango_context()->get_font_description(); }

        const std::string& get_url(){ return m_url; }

#if !GTKMM_CHECK_VERSION(2,10,0)
        void set_dragable( bool dragable, int button );
#endif

        // 本体の横幅 - ラベルの横幅
        const int get_label_margin();

        // カットしていない全体の文字列
        const std::string& get_fulltext() const { return m_fulltext; }
#if GTKMM_CHECK_VERSION(2,10,0)
        void set_fulltext( const std::string& label );
#else
        void set_fulltext( const std::string& label ){ m_fulltext = label; }
#endif

        // アイコンセット
        void set_id_icon( const int id );
        const int get_id_icon() const { return m_id_icon; }

        // タブの文字列の文字数をlngにセット
        void resize_tab( const unsigned int lng );

      private:

#if GTKMM_CHECK_VERSION(2,10,0)
        virtual bool on_button_press_event( GdkEventButton* event );
        virtual bool on_button_release_event( GdkEventButton* event );
#else
        virtual bool on_motion_notify_event( GdkEventMotion* event );
        virtual bool on_leave_notify_event( GdkEventCrossing* event );

        virtual void on_drag_begin( const Glib::RefPtr< Gdk::DragContext>& context );
        virtual void on_drag_data_get( const Glib::RefPtr<Gdk::DragContext>& context,
                                       Gtk::SelectionData& selection_data, guint info, guint time );
        virtual void on_drag_end( const Glib::RefPtr< Gdk::DragContext>& context );
#endif // GTKMM_CHECK_VERSION(2,10,0)
    }; 
}

#endif
