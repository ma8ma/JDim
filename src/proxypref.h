// ライセンス: GPL2

// プロキシ設定ダイアログ

#ifndef _PROXYPREF_H
#define _PROXYPREF_H

#include "skeleton/prefdiag.h"
#include "skeleton/label_entry.h"

#include "config/globalconf.h"

#include "jdlib/miscutil.h"

namespace CORE
{
    constexpr const char kSendCookieTooltip[] = "JDimからプロキシへサイトのクッキーを送信します。\n"
                                                "プロキシのクッキー処理にあわせて設定してください。";

    class ProxyFrame : public Gtk::Frame
    {
        Gtk::VBox m_vbox;
        Gtk::HBox m_hbox;

      public:

        Gtk::CheckButton ckbt;
        Gtk::CheckButton send_cookie_check;
        SKELETON::LabelEntry entry_host;
        SKELETON::LabelEntry entry_port;

        ProxyFrame( const std::string& title, const Glib::ustring& ckbt_label, const Glib::ustring& send_label,
                    const Glib::ustring& host_label, const Glib::ustring& port_label )
            : ckbt( ckbt_label, true )
            , send_cookie_check( send_label, true )
            , entry_host( true, host_label)
            , entry_port( true, port_label )
        {
            send_cookie_check.set_tooltip_text( kSendCookieTooltip );

            m_hbox.set_spacing( 8 );
            m_hbox.pack_start( ckbt, Gtk::PACK_SHRINK );
            m_hbox.pack_start( send_cookie_check, Gtk::PACK_SHRINK );
            m_hbox.pack_start( entry_host );
            m_hbox.pack_start( entry_port, Gtk::PACK_SHRINK );

            m_hbox.set_border_width( 8 );
            m_vbox.set_spacing( 8 );
            m_vbox.pack_start( m_hbox, Gtk::PACK_SHRINK );

            set_label( title );
            set_border_width( 8 );
            add( m_vbox );
        }
    };

    class ProxyPref : public SKELETON::PrefDiag
    {
        Gtk::Label m_label;

        // 2ch読み込み用
        ProxyFrame m_frame_2ch;

        // 2ch書き込み用
        ProxyFrame m_frame_2ch_w;

        // 一般用
        ProxyFrame m_frame_data;

        // OK押した
        void slot_ok_clicked() override
        {
            // 2ch
            CONFIG::set_use_proxy_for2ch( m_frame_2ch.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for2ch( m_frame_2ch.send_cookie_check.get_active() );
            CONFIG::set_proxy_for2ch( MISC::utf8_trim( m_frame_2ch.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for2ch( atoi( m_frame_2ch.entry_port.get_text().c_str() ) );

            // 2ch書き込み用
            CONFIG::set_use_proxy_for2ch_w( m_frame_2ch_w.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for2ch_w( m_frame_2ch_w.send_cookie_check.get_active() );
            CONFIG::set_proxy_for2ch_w( MISC::utf8_trim( m_frame_2ch_w.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for2ch_w( atoi( m_frame_2ch_w.entry_port.get_text().c_str() ) );

            // 一般
            CONFIG::set_use_proxy_for_data( m_frame_data.ckbt.get_active() );
            CONFIG::set_send_cookie_to_proxy_for_data( m_frame_data.send_cookie_check.get_active() );
            CONFIG::set_proxy_for_data( MISC::utf8_trim( m_frame_data.entry_host.get_text().raw() ) );
            CONFIG::set_proxy_port_for_data( atoi( m_frame_data.entry_port.get_text().c_str() ) );
        }

      public:

        ProxyPref( Gtk::Window* parent, const std::string& url )
            : SKELETON::PrefDiag( parent, url )
            , m_label( "認証を行う場合はホスト名を「ユーザID:パスワード@ホスト名」としてください。" )
            , m_frame_2ch( "2ch読み込み用", "使用する(_U)", "クッキーを送る(_I)",
                           "ホスト名(_H)： ", "ポート番号(_P)： " )
            , m_frame_2ch_w( "2ch書き込み用", "使用する(_S)", "クッキーを送る(_J)",
                             "ホスト名(_N)： ", "ポート番号(_R)： " )
            , m_frame_data( "その他のサーバ用(板一覧、外部板、画像など)", "使用する(_E)", "クッキーを送る(_K)",
                            "ホスト名(_A)： ", "ポート番号(_T)： " )
        {
            std::string host;

            // 2ch用
            m_frame_2ch.ckbt.set_active( CONFIG::get_use_proxy_for2ch() );
            m_frame_2ch.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for2ch() );
            if( CONFIG::get_proxy_basicauth_for2ch().empty() ) host = CONFIG::get_proxy_for2ch();
            else host = CONFIG::get_proxy_basicauth_for2ch() + "@" + CONFIG::get_proxy_for2ch();
            m_frame_2ch.entry_host.set_text( host );
            m_frame_2ch.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for2ch() ) );

            set_activate_entry( m_frame_2ch.entry_host );
            set_activate_entry( m_frame_2ch.entry_port );

            // 2ch書き込み用
            m_frame_2ch_w.ckbt.set_active( CONFIG::get_use_proxy_for2ch_w() );
            m_frame_2ch_w.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for2ch_w() );
            if( CONFIG::get_proxy_basicauth_for2ch_w().empty() ) host = CONFIG::get_proxy_for2ch_w();
            else host = CONFIG::get_proxy_basicauth_for2ch_w() + "@" + CONFIG::get_proxy_for2ch_w();
            m_frame_2ch_w.entry_host.set_text( host );
            m_frame_2ch_w.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for2ch_w() ) );

            set_activate_entry( m_frame_2ch_w.entry_host );
            set_activate_entry( m_frame_2ch_w.entry_port );

            // 一般用
            m_frame_data.ckbt.set_active( CONFIG::get_use_proxy_for_data() );
            m_frame_data.send_cookie_check.set_active( CONFIG::get_send_cookie_to_proxy_for_data() );
            if( CONFIG::get_proxy_basicauth_for_data().empty() ) host = CONFIG::get_proxy_for_data();
            else host = CONFIG::get_proxy_basicauth_for_data() + "@" + CONFIG::get_proxy_for_data();
            m_frame_data.entry_host.set_text( host );
            m_frame_data.entry_port.set_text( std::to_string( CONFIG::get_proxy_port_for_data() ) );

            set_activate_entry( m_frame_data.entry_host );
            set_activate_entry( m_frame_data.entry_port );

            get_content_area()->set_spacing( 4 );
            get_content_area()->pack_start( m_label );
            get_content_area()->pack_start( m_frame_2ch );
            get_content_area()->pack_start( m_frame_2ch_w );
            get_content_area()->pack_start( m_frame_data );

            set_title( "プロキシ設定" );
            show_all_children();
        }

        ~ProxyPref() noexcept = default;
    };

}

#endif
