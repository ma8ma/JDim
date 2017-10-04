// ライセンス: GPL2

// 文字列置換設定ダイアログ

#ifndef _REPLACESTRPREF_H
#define _REPLACESTRPREF_H

#include "skeleton/label_entry.h"
#include "skeleton/menubutton.h"
#include "skeleton/prefdiag.h"
#include "skeleton/treeviewbase.h"

#include "control/control.h"

namespace CORE
{
    class ReplaceStrDiag : public SKELETON::PrefDiag
    {
        Gtk::HBox m_hbox_active;
        Gtk::HBox m_hbox_regex;
        Gtk::Button m_button_copy;
        Gtk::CheckButton m_check_active;
        Gtk::CheckButton m_check_icase;
        Gtk::CheckButton m_check_regex;
        Gtk::CheckButton m_check_wchar;
        Gtk::CheckButton m_check_norm;
        SKELETON::LabelEntry m_entry_pattern;
        SKELETON::LabelEntry m_entry_replace;

      public:

        ReplaceStrDiag( Gtk::Window* parent, const int condition, const std::string& pattern, const std::string& replace );

        bool get_active() const { return m_check_active.get_active(); }
        bool get_icase() const { return m_check_icase.get_active(); }
        bool get_regex() const { return m_check_regex.get_active(); }
        bool get_wchar() const { return m_check_wchar.get_active(); }
        bool get_norm() const { return m_check_norm.get_active(); }
        int get_condition() const;
        Glib::ustring get_pattern() const { return m_entry_pattern.get_text(); }
        Glib::ustring get_replace() const { return m_entry_replace.get_text(); }

      private:
        void slot_copy();
        void slot_sens();
    };


    class ReplaceRecord : public Gtk::TreeModel::ColumnRecord
    {
      public:

        Gtk::TreeModelColumn< bool >  m_col_active;
        Gtk::TreeModelColumn< bool >  m_col_icase;
        Gtk::TreeModelColumn< bool >  m_col_regex;
        Gtk::TreeModelColumn< bool >  m_col_wchar;
        Gtk::TreeModelColumn< bool >  m_col_norm;
        Gtk::TreeModelColumn< std::string >  m_col_pattern;
        Gtk::TreeModelColumn< std::string >  m_col_replace;

        ReplaceRecord()
        {
            add( m_col_active );
            add( m_col_icase );
            add( m_col_regex );
            add( m_col_wchar );
            add( m_col_norm );
            add( m_col_pattern );
            add( m_col_replace );
        }
    };

    constexpr int REPLACEPREF_NUM_COLUMNS = 5;  // カラム数
    constexpr int REPLACEPREF_NUM_TARGET = 5;  // タイトル、名前、メール、ID、本文


    class ReplaceStrPref : public SKELETON::PrefDiag
    {
        int m_id_target;
        Gtk::HBox m_hbox_target;
        Gtk::Label m_label_target;
        Gtk::Label m_label_menu;
        SKELETON::MenuButton m_menu_target;

        Gtk::CheckButton m_check_chref;
        bool m_chref[ REPLACEPREF_NUM_TARGET ];

        SKELETON::JDTreeViewBase m_treeview;
        CONTROL::Control m_control;
        Glib::RefPtr< Gtk::ListStore > m_store[ REPLACEPREF_NUM_TARGET ];
        Glib::RefPtr< Gtk::ListStore > m_current_store;
        ReplaceRecord m_columns;
        Gtk::ScrolledWindow m_scrollwin;

        Gtk::Button m_button_top;
        Gtk::Button m_button_up;
        Gtk::Button m_button_down;
        Gtk::Button m_button_bottom;
        Gtk::Button m_button_delete;
        Gtk::Button m_button_add;
        Gtk::VButtonBox m_vbuttonbox;

        Gtk::HBox m_hbox;

      public:

        ReplaceStrPref( Gtk::Window* parent, const std::string& url );

      private:

        void append_rows();
        void append_row( Glib::RefPtr< Gtk::ListStore > store, const int condition,
                        const std::string& pattern, const std::string& replace );

        Gtk::TreeModel::const_iterator get_selected_row() const;
        Gtk::TreeModel::const_iterator get_top_row() const;
        Gtk::TreeModel::const_iterator get_bottom_row() const;

        void select_row( const Gtk::TreeModel::const_iterator& it );

        // OK押した
        virtual void slot_ok_clicked();

        void slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column );
        bool slot_key_release( GdkEventKey* event );

        void slot_top();
        void slot_up();
        void slot_down();
        void slot_bottom();
        void slot_delete();
        void slot_add();

        void slot_target_changed( const int id );
    };
}

#endif
