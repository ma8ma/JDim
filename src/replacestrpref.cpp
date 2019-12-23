// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "replacestrmanager.h"
#include "replacestrpref.h"

#include "control/controlid.h"
#include "jdlib/miscgtk.h"
#include "jdlib/miscutil.h"
#include "skeleton/msgdiag.h"
#include "xml/document.h"

#include "command.h"

using namespace CORE;

ReplaceStrDiag::ReplaceStrDiag( Gtk::Window* parent, const int condition, const std::string& pattern, const std::string& replace )
    : SKELETON::PrefDiag( parent, "" )
    , m_button_copy( "この設定をクリップボードにコピー" )
    , m_check_active( "有効" )
    , m_check_icase( "大文字小文字" )
    , m_check_regex( "正規表現" )
    , m_check_wchar( "全角半角" )
    , m_check_norm( "互換文字" )
    , m_entry_pattern( true, "置換パターン：" )
    , m_entry_replace( true, "置換文字列：" )
{
    resize( 600, 1 );

    m_button_copy.signal_clicked().connect( [this]{ slot_copy(); } );
    m_check_regex.signal_clicked().connect( [this]{ slot_sens(); } );
    m_check_wchar.signal_clicked().connect( [this]{ slot_sens(); } );
    m_check_norm.signal_clicked().connect( [this]{ slot_sens(); } );

    ReplaceStrCondition cdt;
    cdt.raw = condition;

    m_check_active.set_active( cdt.flag.active );
    m_check_icase.set_active( cdt.flag.icase );
    m_check_regex.set_active( cdt.flag.regex );
    m_check_wchar.set_active( cdt.flag.wchar );
    m_check_wchar.set_sensitive( cdt.flag.regex && !cdt.flag.norm );
    m_check_norm.set_active( cdt.flag.norm );
    m_check_norm.set_sensitive( cdt.flag.regex && cdt.flag.wchar );

    m_check_active.set_tooltip_text( "この条件の置換を有効にする" );
    m_check_icase.set_tooltip_text( "大文字小文字を区別しない" );
    m_check_regex.set_tooltip_text( "正規表現を使用する" );
    m_check_wchar.set_tooltip_text( "英数字とカナの種類(俗にいう全角半角)を区別しない" );
    m_check_norm.set_tooltip_text( "Unicodeの互換文字を区別しない" );

    m_hbox_regex.pack_start( m_check_icase, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_regex, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_wchar, Gtk::PACK_SHRINK );
    m_hbox_regex.pack_start( m_check_norm, Gtk::PACK_SHRINK );

    m_entry_pattern.set_text( pattern );
    m_entry_replace.set_text( replace );

    m_hbox_active.pack_start( m_check_active );
    m_hbox_active.pack_start( m_button_copy, Gtk::PACK_SHRINK );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox_active, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_hbox_regex, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_entry_pattern, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_entry_replace, Gtk::PACK_SHRINK );

    set_title( "置換条件設定" );
    show_all_children();
}


//
// 条件フラグ取得
//
int ReplaceStrDiag::get_condition() const
{
    ReplaceStrCondition condition;

    condition.raw = 0;
    condition.flag.active = get_active();
    condition.flag.icase = get_icase();
    condition.flag.regex = get_regex();
    condition.flag.wchar = get_wchar();
    condition.flag.norm = get_norm();

    return condition.raw;
}


//
// クリップボードにコピー
//
void ReplaceStrDiag::slot_copy()
{
    XML::Document document;
    XML::Dom* node = ReplaceStr_Manager::dom_append( &document, get_condition(), get_pattern(), get_replace() );

    MISC::CopyClipboard( node->get_xml() );
}


//
// wcharチェックボックスのsensitive切り替え
//
void ReplaceStrDiag::slot_sens()
{
    m_check_wchar.set_sensitive( get_regex() && !get_norm() );
    m_check_norm.set_sensitive( get_regex() && get_wchar() );
}


///////////////////////////////////////////////

static constexpr const char* target_name[ REPLACEPREF_NUM_TARGET ] = {
    "スレタイトル", "名前", "メール", "日付/ID", "本文" };

ReplaceStrPref::ReplaceStrPref( Gtk::Window* parent, const std::string& url )
    : SKELETON::PrefDiag( parent, url )
    , m_id_target( 0 )
    , m_label_target( "置換対象：" )
    , m_label_menu( target_name[ 0 ] )
    , m_menu_target( true, m_label_menu )
    , m_check_chref( "対象の置換前に文字参照をデコード" )
    , m_button_top( Gtk::Stock::GOTO_TOP )
    , m_button_up( Gtk::Stock::GO_UP )
    , m_button_down( Gtk::Stock::GO_DOWN )
    , m_button_bottom( Gtk::Stock::GOTO_BOTTOM )
    , m_button_delete( Gtk::Stock::DELETE )
    , m_button_add( Gtk::Stock::ADD )
{
    m_button_top.signal_clicked().connect( [this]{ slot_top(); } );
    m_button_up.signal_clicked().connect( [this]{ slot_up(); } );
    m_button_down.signal_clicked().connect( [this]{ slot_down(); } );
    m_button_bottom.signal_clicked().connect( [this]{ slot_bottom(); } );
    m_button_delete.signal_clicked().connect( [this]{ slot_delete(); } );
    m_button_add.signal_clicked().connect( [this]{ slot_add(); } );

    for( Glib::RefPtr< Gtk::ListStore >& store : m_store ) {
        store = Gtk::ListStore::create( m_columns );
    }
    m_current_store = m_store[ 0 ];
    m_treeview.set_model( m_current_store );
    m_treeview.set_size_request( 640, 400 );
    m_treeview.signal_row_activated().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_row_activated ) );
    m_treeview.sig_key_release().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_key_release ) );

    Gtk::TreeViewColumn* column[ REPLACEPREF_NUM_COLUMNS ];

    column[ 0 ] = Gtk::manage( new Gtk::TreeViewColumn( "有効", m_columns.m_col_active ) );
    column[ 0 ]->set_fixed_width( 35 );
    column[ 0 ]->set_alignment( Gtk::ALIGN_CENTER );
    column[ 1 ] = Gtk::manage( new Gtk::TreeViewColumn( "大小", m_columns.m_col_icase ) );
    column[ 1 ]->set_fixed_width( 35 );
    column[ 1 ]->set_alignment( Gtk::ALIGN_CENTER );
    column[ 2 ] = Gtk::manage( new Gtk::TreeViewColumn( "正規", m_columns.m_col_regex ) );
    column[ 2 ]->set_fixed_width( 35 );
    column[ 2 ]->set_alignment( Gtk::ALIGN_CENTER );
    column[ 3 ] = Gtk::manage( new Gtk::TreeViewColumn( "置換パターン", m_columns.m_col_pattern ) );
    column[ 3 ]->set_fixed_width( 220 );
    column[ 4 ] = Gtk::manage( new Gtk::TreeViewColumn( "置換文字列", m_columns.m_col_replace ) );
    column[ 4 ]->set_fixed_width( 200 );

    for( Gtk::TreeViewColumn* col : column ) {
        col->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
        col->set_resizable( true );
        m_treeview.append_column( *col );
    }

    m_scrollwin.add( m_treeview );
    m_scrollwin.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS );
    m_scrollwin.set_size_request( 640, 400 );

    m_vbuttonbox.pack_start( m_button_top, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_up, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_down, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_bottom, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_delete, Gtk::PACK_SHRINK );
    m_vbuttonbox.pack_start( m_button_add, Gtk::PACK_SHRINK );
    m_vbuttonbox.set_layout( Gtk::BUTTONBOX_START );
    m_vbuttonbox.set_spacing( 4 );

    m_hbox.pack_start( m_scrollwin, Gtk::PACK_EXPAND_WIDGET );
    m_hbox.pack_start( m_vbuttonbox, Gtk::PACK_SHRINK );

    std::vector< std::string > list_target;
    for( const char* target : target_name ) list_target.emplace_back( target );
    m_menu_target.append_menu( list_target );
    m_menu_target.set_enable_sig_clicked( false );
    m_menu_target.signal_selected().connect( sigc::mem_fun( *this, &ReplaceStrPref::slot_target_changed ) );

    m_hbox_target.set_border_width( 8 );
    m_hbox_target.set_spacing( 8 );
    m_hbox_target.pack_start( m_label_target, Gtk::PACK_SHRINK );
    m_hbox_target.pack_start( m_menu_target, Gtk::PACK_SHRINK );

    m_check_chref.set_tooltip_markup( "ダブルクォーテション<b>\"</b>, アンパサント"
            "<b>&amp;</b>, 少なり記号<b>&lt;</b>, 大なり記号<b>&gt;</b>を<b>除く</b>"
            "文字参照をデコードしてから置換を行います。" );

    get_vbox()->set_spacing( 8 );
    get_vbox()->pack_start( m_hbox_target, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_check_chref, Gtk::PACK_SHRINK );
    get_vbox()->pack_start( m_hbox );

    show_all_children();
    set_title( "文字列置換設定" );

    append_rows();
}


void ReplaceStrPref::append_rows()
{
    ReplaceStr_Manager* mgr = CORE::get_replacestr_manager();

    for( int i = 0; i < REPLACEPREF_NUM_TARGET; ++i ){

        m_chref[ i ] = mgr->get_chref( i );

        for( const auto& item : mgr->get_list( i ) ) {
            append_row( m_store[ i ], item->condition, item->pattern, item->replace );
        }
    }

    m_check_chref.set_active( m_chref[ 0 ] );
    select_row( get_top_row() );
}


void ReplaceStrPref::append_row( Glib::RefPtr< Gtk::ListStore > store, const int condition, const std::string& pattern, const std::string& replace )
{
    Gtk::TreeModel::Row row = *( store->append() );

    if( row ){
        const auto* cdt = reinterpret_cast< const ReplaceStrCondition * >( &condition );

        row[ m_columns.m_col_active ] = cdt->flag.active;
        row[ m_columns.m_col_icase ] = cdt->flag.icase;
        row[ m_columns.m_col_regex ] = cdt->flag.regex;
        row[ m_columns.m_col_wchar ] = cdt->flag.wchar;
        row[ m_columns.m_col_norm ] = cdt->flag.norm;
        row[ m_columns.m_col_pattern ] = pattern;
        row[ m_columns.m_col_replace ] = replace;

        select_row( row );
    }
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_selected_row() const
{
    std::vector< Gtk::TreeModel::Path > paths = m_treeview.get_selection()->get_selected_rows();

    if( ! paths.size() ) return Gtk::TreeModel::const_iterator();
    return *( m_current_store->get_iter( paths.front() ) );
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_top_row() const
{
    Gtk::TreeModel::Children children = m_current_store->children();

    if( children.empty() ) return Gtk::TreeModel::const_iterator();
    return children.begin();
}


Gtk::TreeModel::const_iterator ReplaceStrPref::get_bottom_row() const
{
    Gtk::TreeModel::Children children = m_current_store->children();

    if( children.empty() ) return Gtk::TreeModel::const_iterator();
    return std::prev( children.end() );
}


void ReplaceStrPref::select_row( const Gtk::TreeModel::const_iterator& it )
{
    if( !it ) return;

    const Gtk::TreePath path( it );
    m_treeview.get_selection()->select( path );
}


//
// OK ボタンを押した
//
void ReplaceStrPref::slot_ok_clicked()
{
#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_ok_clicked" << std::endl;
#endif

    ReplaceStr_Manager* mgr = CORE::get_replacestr_manager();

    m_chref[ m_id_target ] = m_check_chref.get_active();

    for( int i = 0; i < REPLACEPREF_NUM_TARGET; ++i ){

        mgr->list_clear( i );

        mgr->set_chref( i, m_chref[ i ] );

        for( const Gtk::TreeModel::Row& row : m_store[ i ]->children() ) {

            const bool active = row[ m_columns.m_col_active ];
            const bool icase = row[ m_columns.m_col_icase ];
            const bool regex = row[ m_columns.m_col_regex ];
            const bool wchar = row[ m_columns.m_col_wchar ];
            const bool norm = row[ m_columns.m_col_norm ];

            const ReplaceStrCondition cdt = { { active, icase, regex, wchar, norm } };
            const std::string& pattern = row[ m_columns.m_col_pattern ];
            const std::string& replace = row[ m_columns.m_col_replace ];

            mgr->list_append( i, cdt.raw, pattern, replace );
#ifdef _DEBUG
            std::cout << "target=" << i << " condition=" << cdt.raw
                << " pattern=" << pattern << " replace=" << replace << std::endl;
#endif
        }
    }

    CORE::get_replacestr_manager()->save_xml();

    //DBTREE::update_abone_thread();
    CORE::core_set_command( "relayout_all_board" );
    CORE::core_set_command( "relayout_all_article", "", "completely" );
}


void ReplaceStrPref::slot_row_activated( const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column )
{
#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_row_activated path = " << path.to_string() << std::endl;
#endif

    Gtk::TreeModel::Row row = *( m_current_store->get_iter( path ) );
    if( ! row ) return;

    const bool active = row[ m_columns.m_col_active ];
    const bool icase = row[ m_columns.m_col_icase ];
    const bool regex = row[ m_columns.m_col_regex ];
    const bool wchar = row[ m_columns.m_col_wchar ];
    const bool norm = row[ m_columns.m_col_norm ];
    const ReplaceStrCondition condition = { { active, icase, regex, wchar, norm } };

    ReplaceStrDiag dlg( this, condition.raw, row[ m_columns.m_col_pattern ], row[ m_columns.m_col_replace ] );
    if( dlg.run() == Gtk::RESPONSE_OK ){
        row[ m_columns.m_col_active ] = dlg.get_active();
        row[ m_columns.m_col_icase ] = dlg.get_icase();
        row[ m_columns.m_col_regex ] = dlg.get_regex();
        row[ m_columns.m_col_wchar ] = dlg.get_wchar();
        row[ m_columns.m_col_norm ] = dlg.get_norm();
        row[ m_columns.m_col_pattern ] = dlg.get_pattern();
        row[ m_columns.m_col_replace ] = dlg.get_replace();

        if( dlg.get_regex() ){
            JDLIB::RegexPattern ptn;
            constexpr bool newline = true;
            constexpr bool migemo = true;

            if( ! ptn.set( dlg.get_pattern(), dlg.get_icase(), newline, migemo, dlg.get_wchar(), dlg.get_norm() ) ){
                const std::string msg = ptn.errstr() + "\n\n" + dlg.get_pattern();

                SKELETON::MsgDiag mdlg( nullptr, msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK );
                mdlg.run();
            }
        }
    }
}


bool ReplaceStrPref::slot_key_release( GdkEventKey* event )
{
    const int id = m_control.key_press( event );

#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_key_release id = " << id << std::endl;
#endif

    if( id == CONTROL::Delete ) slot_delete();

    return true;
}


//
// 一番上へ移動
//
void ReplaceStrPref::slot_top()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_top = get_top_row();

    if( it && it != it_top ) m_current_store->move( it, it_top );
}


//
// 上へ移動
//
void ReplaceStrPref::slot_up()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_top = get_top_row();

    if( it && it != it_top ){

        Gtk::TreePath path_dst( it );
        if( path_dst.prev() ){

            Gtk::TreeModel::iterator it_dst = m_current_store->get_iter( path_dst );
            m_current_store->iter_swap( it, it_dst );
        }
    }
}


//
// 下へ移動
//
void ReplaceStrPref::slot_down()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_bottom = get_bottom_row();

    if( it && it != it_bottom ){

        Gtk::TreePath path_dst( it );
        path_dst.next();
        Gtk::TreeModel::iterator it_dst = m_current_store->get_iter( path_dst );
        if( it_dst ) m_current_store->iter_swap( it, it_dst );
    }
}



//
// 一番下へ移動
//
void ReplaceStrPref::slot_bottom()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    Gtk::TreeModel::iterator it_bottom = get_bottom_row();

    if( it && it != it_bottom ){
        m_current_store->move( it, m_current_store->children().end() );
    }
}


//
// 削除ボタン
//
void ReplaceStrPref::slot_delete()
{
    Gtk::TreeModel::iterator it = get_selected_row();
    if( ! it ) return;

    SKELETON::MsgDiag mdiag( nullptr, "削除しますか？", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO );
    if( mdiag.run() != Gtk::RESPONSE_YES ) return;

    Gtk::TreePath path_next( it );
    path_next.next();
    Gtk::TreeModel::iterator it_next = m_current_store->get_iter( path_next );

    m_current_store->erase( it );

    if( it_next ) select_row( it_next );
    else{
        Gtk::TreeModel::iterator it_bottom = get_bottom_row();
        if( it_bottom ) select_row( it_bottom );
    }
}


//
// 追加ボタン
//
void ReplaceStrPref::slot_add()
{
    const ReplaceStrCondition condition = { { true, false, false, false, false } };
    ReplaceStrDiag dlg( this, condition.raw, "", "" );
    if( dlg.run() == Gtk::RESPONSE_OK )
        append_row( m_current_store, dlg.get_condition(),
                    dlg.get_pattern(), dlg.get_replace() );
}


//
// 置換対象変更
//
void ReplaceStrPref::slot_target_changed( const int id )
{
#ifdef _DEBUG
    std::cout << "ReplaceStrPref::slot_target_changed target=" << id << std::endl;
#endif

    m_chref[ m_id_target ] = m_check_chref.get_active();
    m_check_chref.set_active( m_chref[ id ] );
    m_label_menu.set_text( target_name[ id ] );
    m_current_store = m_store[ id ];
    m_treeview.set_model( m_current_store );
    select_row( get_top_row() );
    m_id_target = id;
}
