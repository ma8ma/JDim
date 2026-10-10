// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "toolbarsimple.h"
#include "articleadmin.h"

#include "compmanager.h"

#include "jdlib/gtk_compat.h"

using namespace ARTICLE;


ArticleToolBarSimple::ArticleToolBarSimple() :
    SKELETON::ToolBar( ARTICLE::get_admin() )
{
    // 検索バー    
    JDLIB::compat::box_append_expand( *get_searchbar(), *get_tool_search( CORE::COMP_SEARCH_ARTICLE ) );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_down_search() );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_up_search() );
    JDLIB::compat::box_append_shrink( *get_searchbar(), *get_button_close_searchbar() );

    ArticleToolBarSimple::pack_buttons();
}


ArticleToolBarSimple::~ArticleToolBarSimple() noexcept = default;


//
// ボタンのパッキング
//
void ArticleToolBarSimple::pack_buttons()
{
    get_label()->set_margin_start( 8 );
    JDLIB::compat::box_append_expand( get_buttonbar(), *get_label() );
    JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_open_searchbar() );
    JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_reload() );
    JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_stop() );
    JDLIB::compat::box_append_shrink( get_buttonbar(), *get_button_close() );

    set_relief();
    show_all_children();
}
