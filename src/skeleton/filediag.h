// ライセンス: GPL2

// ファイル選択ダイアログの基底クラス

#ifndef _FILEDIAG_H
#define _FILEDIAG_H

#include <glib/gi18n.h>
#include <gtkmm.h>

#include "command.h"
#include "session.h"

#include "jdlib/gtk_compat.h"

namespace SKELETON
{
    class FileDiag : public Gtk::FileChooserDialog
    {
        using FileChooserAction = JDLIB::compat::FileChooser::Action::type;
        FileChooserAction m_action;

      public:

        // ボタン追加 + saveボタンをデフォルトボタンにセット
        void add_buttons(){

            add_button( g_dgettext( GTK_DOMAIN, "_Cancel" ), JDLIB::compat::ResponseType::CANCEL );

            if( m_action == JDLIB::compat::FileChooser::Action::OPEN ) {
                add_button( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Open", 12 ), JDLIB::compat::ResponseType::ACCEPT );
            }
            else add_button( g_dpgettext( GTK_DOMAIN, "Stock label\x04_Save", 12 ), JDLIB::compat::ResponseType::ACCEPT );

            set_default_response( JDLIB::compat::ResponseType::ACCEPT );
        }

        FileDiag( Gtk::Window& parent, const Glib::ustring& title, FileChooserAction action = JDLIB::compat::FileChooser::Action::OPEN )
        : Gtk::FileChooserDialog( parent, title, action ), m_action( action )
        {
            add_buttons();
        }

        // parent がポインタの時は  nullptr かどうかで場合分け
        FileDiag( Gtk::Window* parent, const Glib::ustring& title, FileChooserAction action = JDLIB::compat::FileChooser::Action::OPEN )
        : Gtk::FileChooserDialog( title, action ), m_action( action )
        {
            add_buttons();

            if( parent ) set_transient_for( *parent );
            else set_transient_for( *CORE::get_mainwindow() );
        }

        ~FileDiag() noexcept override = default;

        virtual int run(){

            SESSION::set_dialog_shown( true );
            CORE::core_set_command( "dialog_shown" );

            int ret = Gtk::FileChooserDialog::run();

            SESSION::set_dialog_shown( false );
            CORE::core_set_command( "dialog_hidden" );

            return ret;
        }

    };
}

#endif
