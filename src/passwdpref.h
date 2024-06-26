// ライセンス: GPL2

// パスワード設定ダイアログ

#ifndef _PASSWDPREF_H
#define _PASSWDPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include "login2ch.h"
#include "loginbe.h"

#include "jdlib/miscutil.h"

enum
{
    BOXSPACING = 8
};

namespace CORE
{
    // 2chログイン用
    class PasswdFrame2ch : public Gtk::VBox
    {
        SKELETON::LabelEntry m_label_sid_2ch;

      public:

        SKELETON::LabelEntry entry_id;
        SKELETON::LabelEntry entry_passwd;

        PasswdFrame2ch()
            : m_label_sid_2ch( false, "SID： ", "2chのログインは現在サポート中止しています。" )
            , entry_id( true, "ユーザID(_I)： " )
            , entry_passwd( true, "パスワード(_P)： " )
        {
            const int mrg = 8;

            set_border_width( BOXSPACING );

            entry_id.set_border_width( BOXSPACING );
            pack_start( entry_id );

            entry_passwd.set_border_width( BOXSPACING );
            entry_passwd.set_visibility( false );
            pack_start( entry_passwd, Gtk::PACK_SHRINK );

            m_label_sid_2ch.set_border_width( BOXSPACING );
            pack_start( m_label_sid_2ch );

            set_border_width( mrg );
        }
    };

    // BEログイン用
    class PasswdFrameBe : public Gtk::VBox
    {
        SKELETON::LabelEntry m_label_dmdm;
        SKELETON::LabelEntry m_label_mdmd;

      public:

        SKELETON::LabelEntry entry_id;
        SKELETON::LabelEntry entry_passwd;

      PasswdFrameBe()
          : m_label_dmdm( false, "DMDM： ", CORE::get_loginbe()->get_sessionid() ),
            m_label_mdmd( false, "MDMD： ", CORE::get_loginbe()->get_sessiondata() ),
            entry_id( true, "メールアドレス(_I)： " ), entry_passwd( true, "パスワード(_P)： " )
        {
            set_border_width( BOXSPACING );

            entry_id.set_border_width( BOXSPACING );
            pack_start( entry_id );

            entry_passwd.set_border_width( BOXSPACING );
            entry_passwd.set_visibility( false );
            pack_start( entry_passwd, Gtk::PACK_SHRINK );

            pack_start( m_label_dmdm );
            pack_start( m_label_mdmd );

            set_border_width( BOXSPACING );
        }
    };


    class PasswdPref : public SKELETON::PrefDiag
    {

        Gtk::Notebook m_notebook;

        PasswdFrame2ch m_frame_2ch;
        PasswdFrameBe m_frame_be;

        // OK押した
        void slot_ok_clicked() override
        {
            // 2ch
            CORE::get_login2ch()->set_username( MISC::utf8_trim( m_frame_2ch.entry_id.get_text().raw() ) );
            CORE::get_login2ch()->set_passwd( MISC::utf8_trim( m_frame_2ch.entry_passwd.get_text().raw() ) );

            // BE
            CORE::get_loginbe()->set_username( MISC::utf8_trim( m_frame_be.entry_id.get_text().raw() ) );
            CORE::get_loginbe()->set_passwd( MISC::utf8_trim( m_frame_be.entry_passwd.get_text().raw() ) );
        }

      public:

        PasswdPref( Gtk::Window* parent, const std::string& url )
        : SKELETON::PrefDiag( parent, url )
        , m_frame_2ch(), m_frame_be()
        {
            // 2chログイン用
            m_frame_2ch.entry_id.set_text( CORE::get_login2ch()->get_username() );
            m_frame_2ch.entry_passwd.set_text( CORE::get_login2ch()->get_passwd() );

            set_activate_entry( m_frame_2ch.entry_id );
            set_activate_entry( m_frame_2ch.entry_passwd );

            // beログイン用
            m_frame_be.entry_id.set_text( CORE::get_loginbe()->get_username() );
            m_frame_be.entry_passwd.set_text( CORE::get_loginbe()->get_passwd() );

            set_activate_entry( m_frame_be.entry_id );
            set_activate_entry( m_frame_be.entry_passwd );

            m_notebook.append_page( m_frame_2ch, "2ch" );
            m_notebook.append_page( m_frame_be, "BE" );
            get_content_area()->pack_start( m_notebook );

            set_title( "パスワード設定" );
            show_all_children();
        }

        ~PasswdPref() noexcept = default;
    };

}

#endif
