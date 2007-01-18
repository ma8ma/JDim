// ライセンス: GPL2

// ツールバーのクラス

#ifndef _BBSLIST_TOOLBAR_H
#define _BBSLIST_TOOLBAR_H

#include <gtkmm.h>

#include "skeleton/imgbutton.h"
#include "skeleton/entry.h"

#include "controlutil.h"
#include "controlid.h"


enum
{
    COMBO_BBSLIST = 0,
    COMBO_FAVORITE = 1
};



namespace BBSLIST
{
    class BBSListtToolBar : public Gtk::VBox
    {
        friend class BBSListViewBase;
        friend class BBSListViewMain;
        friend class FavoriteListView;
        friend class SelectListView;

        // ラベルバー
        Gtk::HBox m_hbox_label;
        Gtk::ComboBoxText m_combo;
        SKELETON::ImgButton m_button_close;

        // 検索バー
        Gtk::HBox m_hbox_search;
        SKELETON::JDEntry m_entry_search;
        SKELETON::ImgButton m_button_up_search;
        SKELETON::ImgButton m_button_down_search;

        Gtk::Tooltips m_tooltip;

        void set_combo( int page ){ m_combo.set_active( page ); }
        int  get_combo(){ return m_combo.get_active_row_number(); }

        BBSListtToolBar() :
        m_button_close( Gtk::Stock::CLOSE ),
        m_button_up_search( Gtk::Stock::GO_UP ),
        m_button_down_search( Gtk::Stock::GO_DOWN )
        {
            // ラベルバー
            m_combo.append_text( "板一覧" );
            m_combo.append_text( "お気に入り" );

            m_tooltip.set_tip( m_button_close, CONTROL::get_label_motion( CONTROL::Quit ) );
            m_hbox_label.pack_start( m_combo, Gtk::PACK_EXPAND_WIDGET, 2 );
            m_hbox_label.pack_start( m_button_close, Gtk::PACK_SHRINK );

            // 検索バー
            m_tooltip.set_tip( m_button_up_search, CONTROL::get_label_motion( CONTROL::SearchPrev ) );
            m_tooltip.set_tip( m_button_down_search, CONTROL::get_label_motion( CONTROL::SearchNext ) );
            m_hbox_search.pack_start( m_entry_search );
            m_hbox_search.pack_end( m_button_up_search, Gtk::PACK_SHRINK );
            m_hbox_search.pack_end( m_button_down_search, Gtk::PACK_SHRINK );


            pack_start( m_hbox_label, Gtk::PACK_SHRINK );
            pack_start( m_hbox_search, Gtk::PACK_SHRINK );
        }

        void remove_label()
        {
            remove( m_hbox_label );
        }
    };
}


#endif
