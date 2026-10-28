// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "msgdiag.h"

#include "command.h"
#include "session.h"
#include "dispatchmanager.h"
#include "global.h"

#include "jdlib/gtk_compat.h"

#include <glib/gi18n.h>


using namespace SKELETON;


MsgDiag::MsgDiag( Gtk::Window& parent,
                  const Glib::ustring& message,
                  bool use_markup,
                  Gtk::MessageType type,
                  Gtk::ButtonsType buttons,
                  bool modal )
    : Gtk::MessageDialog( parent, message, use_markup, type, buttons, modal )
{}

// parent がポインタの時は  nullptr かどうかで場合分け
MsgDiag::MsgDiag( Gtk::Window* parent,
                  const Glib::ustring& message,
                  bool use_markup,
                  Gtk::MessageType type,
                  Gtk::ButtonsType buttons,
                  bool modal )
    : Gtk::MessageDialog( message, use_markup, type, buttons, modal )
{
    if( parent ) set_transient_for( *parent );
    else set_transient_for( *CORE::get_mainwindow() );

    // tab でラベルにフォーカスが移らないようにする
    const std::vector< Gtk::Widget* > area = get_message_area()->get_children();
    Gtk::Label* const primary_label = dynamic_cast< Gtk::Label* >( area.front() );
    if( primary_label ) {
        primary_label->set_can_focus( false );
    }
}


MsgDiag::~MsgDiag() noexcept = default;


void MsgDiag::add_default_button( const Glib::ustring& label, const int id )
{
    Gtk::Button* button = Gtk::manage( new Gtk::Button( label, true ) );
    add_default_button( button, id );
}


void MsgDiag::add_default_button( Gtk::Widget* button, const int id )
{
    if( ! button ) return;

    add_action_widget( *button, id );
    button->show();

    button->set_can_default( true );

    button->grab_default();
    button->grab_focus();
}


int MsgDiag::run()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::run start\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    // タイマーセット
    sigc::slot< bool > slot_tmout = sigc::bind( sigc::mem_fun(*this, &MsgDiag::slot_timeout), 0 );
    m_conn_timer = JDLIB::Timeout::connect( slot_tmout, TIMER_TIMEOUT );

    int ret = Gtk::MessageDialog::run();

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

#ifdef _DEBUG
    std::cout << "MsgDiag::run fin\n";
#endif

    return ret;
}


void MsgDiag::show()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::show\n";
#endif

    SESSION::set_dialog_shown( true );
    CORE::core_set_command( "dialog_shown" );

    Gtk::MessageDialog::show();
}


void MsgDiag::hide()
{
#ifdef _DEBUG
    std::cout << "MsgDiag::hide\n";
#endif

    SESSION::set_dialog_shown( false );
    CORE::core_set_command( "dialog_hidden" );

    Gtk::MessageDialog::hide();
}


// タイマーのslot関数
bool MsgDiag::slot_timeout( int timer_number )
{
    // メインループが停止していて dispatcher が働かないため
    // タイマーから強制的に実行させる
    CORE::get_dispmanager()->slot_dispatch();

    return true;
}



/////////////////////////////////////


MsgCheckDiag::MsgCheckDiag( Gtk::Window* parent,
                            const Glib::ustring& message,
                            const Glib::ustring& message_check,
                            Gtk::MessageType type, Gtk::ButtonsType buttons, const int default_response )
    : SKELETON::MsgDiag( parent, message, false, type, JDLIB::compat::ButtonsType::NONE, false )
    , m_chkbutton( message_check, true )
{
    const int mrg = 16;
                
    Gtk::Box* hbox = Gtk::make_managed<Gtk::Box>( JDLIB::compat::Orientation::HORIZONTAL, 0 );
    JDLIB::compat::box_append_expand( *hbox, m_chkbutton, mrg );
    JDLIB::compat::box_append_shrink( *get_content_area(), *hbox );

    if( buttons == JDLIB::compat::ButtonsType::OK ){

        add_default_button( g_dgettext( GTK_DOMAIN, "_OK" ), JDLIB::compat::ResponseType::OK );
    }
    else if( buttons == JDLIB::compat::ButtonsType::OK_CANCEL ){
        add_button( g_dgettext( GTK_DOMAIN, "_No" ), JDLIB::compat::ResponseType::CANCEL );

        add_default_button( g_dgettext( GTK_DOMAIN, "_OK" ), JDLIB::compat::ResponseType::OK );
    }
    else if( buttons == JDLIB::compat::ButtonsType::YES_NO ){

        if( default_response == JDLIB::compat::ResponseType::NO ){

            add_default_button( g_dgettext( GTK_DOMAIN, "_No" ), JDLIB::compat::ResponseType::NO );

            add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), JDLIB::compat::ResponseType::YES );
        }
        else{

            add_button( g_dgettext( GTK_DOMAIN, "_No" ), JDLIB::compat::ResponseType::NO );

            add_default_button( g_dgettext( GTK_DOMAIN, "_Yes" ), JDLIB::compat::ResponseType::YES );
        }
    }

    show_all_children();
}


MsgCheckDiag::~MsgCheckDiag() noexcept = default;


/////////////////////////////////////



MsgOverwriteDiag::MsgOverwriteDiag( Gtk::Window* parent )
    : SKELETON::MsgDiag( parent, "ファイルが存在します。ファイル名を変更して保存しますか？", 
                         false, JDLIB::compat::MessageType::QUESTION, JDLIB::compat::ButtonsType::NONE )
{
    add_button( g_dgettext( GTK_DOMAIN, "_No" ), JDLIB::compat::ResponseType::NO );
    add_button( g_dgettext( GTK_DOMAIN, "_Yes" ), JDLIB::compat::ResponseType::YES );
    add_button( "上書き", OVERWRITE_YES );
    add_button( "すべていいえ", OVERWRITE_NO_ALL );
    add_button( "すべて上書き", OVERWRITE_YES_ALL );
}


MsgOverwriteDiag::~MsgOverwriteDiag() noexcept = default;
