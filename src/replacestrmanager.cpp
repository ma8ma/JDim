// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "replacestrmanager.h"
#include "cache.h"
#include "type.h"
#include "command.h"

#include "jdlib/miscmsg.h"
#include "jdlib/miscutil.h"

#include "xml/document.h"
#include "xml/tools.h"

constexpr const char* ROOT_NODE_NAME_REPLACESTR = "replacestrlist";

static constexpr const char* target_str[] = { "subject", "name", "mail", "date", "msg" };


static CORE::ReplaceStr_Manager* instance_replacestr_manager = nullptr;

CORE::ReplaceStr_Manager* CORE::get_replacestr_manager()
{
    if( ! instance_replacestr_manager ) instance_replacestr_manager = new ReplaceStr_Manager();
    assert( instance_replacestr_manager );

    return instance_replacestr_manager;
}


void CORE::delete_replacestr_manager()
{
    if( instance_replacestr_manager ) delete instance_replacestr_manager;
    instance_replacestr_manager = nullptr;
}

///////////////////////////////////////////////

using namespace CORE;

ReplaceStr_Manager::ReplaceStr_Manager()
{
    for( bool& chref : m_chref ) chref = false;

    std::string xml;
    if( CACHE::load_rawdata( CACHE::path_replacestr(), xml ) ) xml2list( xml );
    else{
        // デフォルトの設定を行う
        m_chref[ 0 ] = true;
        ReplaceStrCondition condition = { { false, false, false, false, false } };
        list_append( 0, condition.raw, "ダブルクリックすると編集出来ます",
                     "(この項目は削除して構いません)" );
        condition.flag.regex = true;
        list_append( 0, condition.raw, "^\\[(無断)?転載禁止\\]",
                     "<span weight=\"heavy\" color=\"darkgreen\">...</span>" );
        list_append( 0, condition.raw,
                     "(\\[無?断?転載禁止\\]©([25]ch\\.net|bbspink\\.com)|\\[無?断?転載禁止\\]"
                     "|©[25]ch\\.net|©bbspink\\.com)([[:blank:]]+\\[[[:digit:]]+\\])?$",
                     "<span weight=\"heavy\" color=\"darkgreen\">...</span>\\3" );
    }
}


ReplaceStr_Manager::~ReplaceStr_Manager() noexcept = default;


//
// リストをクリア
//
void ReplaceStr_Manager::list_clear( const int id )
{
    m_list[ id ].clear();
}


//
// アイテムをリストに追加
//
void ReplaceStr_Manager::list_append( const int id, const int condition, const std::string& pattern, const std::string& replace )
{
    if( id >= REPLACETARGET_MAX ) return;

    auto item = std::unique_ptr< ReplaceStrItem >( new ReplaceStrItem );

    item->condition = condition;
    item->pattern = pattern;
    item->replace = replace;

    const auto* cdt = reinterpret_cast< const ReplaceStrCondition* >( &condition );
    if( cdt->flag.regex ){
        constexpr bool newline = true;
        constexpr bool migemo = true;

        if( ! item->creg.set( pattern, cdt->flag.icase, newline, migemo,
                              cdt->flag.wchar, cdt->flag.norm ) ){
            std::string msg ="invlid replacestr pattern: ";
            msg += item->creg.errstr() + ": " + pattern;
            MISC::ERRMSG( msg );
        }
    }

#ifdef _DEBUG
    std::cout << "conditnon=" << item->condition << " pattern=" << item->pattern
              << " replace=" << item->replace << std::endl;
#endif

    m_list[ id ].push_back( std::move( item ) );
}


//
// xml -> リスト
//
void ReplaceStr_Manager::xml2list( const std::string& xml )
{
    for( int i = 0; i < REPLACETARGET_MAX; ++i ) list_clear( i );
    if( xml.empty() ) return;

    XML::Document document( xml );

    XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME_REPLACESTR ) );
    if( ! root ) return;

    XML::DomList domlist = root->childNodes();

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::xml2list";
    std::cout << " children =" << document.childNodes().size() << std::endl;
#endif

    for( const XML::Dom* dom : domlist ) {

        if( dom->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

        const int type = XML::get_type( dom->nodeName() );
        if( type != TYPE_DIR ) continue;

        const size_t id = target_id( dom->getAttribute( "name" ) );
        if( id >= REPLACETARGET_MAX ) continue;

        m_chref[ id ] = dom->getAttribute( "chref" ) == "true";

        XML::DomList domlist_query = dom->childNodes();
        for( const XML::Dom* query : domlist_query ) {

            if( query->nodeType() != XML::NODE_TYPE_ELEMENT ) continue;

            const int type = XML::get_type( query->nodeName() );
            if( type != TYPE_REPLACESTR ) continue;

            list_append( id, std::stoi( query->getAttribute( "condition" ) ),
                         query->getAttribute( "pattern" ),
                         query->getAttribute( "replace" ) );
        }
    }
}


//
// XML 保存
//
void ReplaceStr_Manager::save_xml()
{
    XML::Document document;
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME_REPLACESTR ) );
    if( ! root ) return;

    for( int i = 0; i < REPLACETARGET_MAX; ++i ){

        if( m_list[ i ].empty() ) continue;

        XML::Dom* node = dom_append( root, i, m_chref[ i ] );

        for( const auto& item : m_list[ i ] ) {
            dom_append( node, item->condition, item->pattern, item->replace );
        }
    }

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::save_xml" << std::endl;
    std::cout << document.get_xml() << std::endl;
#endif

    CACHE::save_rawdata( CACHE::path_replacestr(), document.get_xml() );
}


//
// 置換対象の要素名からid取得
//
int ReplaceStr_Manager::target_id( const std::string& name )
{
    int id;

    for( id = 0; id < REPLACETARGET_MAX; ++id )
        if( name.compare( 0, std::string::npos, target_str[ id ] ) == 0 ) break;

    return id;
}


//
// 置換対象idから要素名取得
//
std::string ReplaceStr_Manager::target_name( const int id )
{
    const char *name;

    if( id >= REPLACETARGET_MAX ) name = nullptr;
    else name = target_str[ id ];

    return std::string( name );
}


//
// 実行
//
std::string ReplaceStr_Manager::replace( const char* str, const int lng, const int id ) const
{
    if( id >= REPLACETARGET_MAX || m_list[ id ].empty() ) return std::string( str, lng );

    std::string str_work;

    if( m_chref[ id ] ) str_work = MISC::chref_decode( str, lng, false );
    else str_work.assign( str, lng );

#ifdef _DEBUG
    std::cout << "ReplaceStr_Manager::replace str=" << str_work << std::endl;
#endif

    JDLIB::Regex regex;

    for( const auto& item : m_list[ id ] ) {

        ReplaceStrCondition cdt;
        cdt.raw = item->condition;

        if( ! cdt.flag.active || item->pattern.empty() ) continue;

#ifdef _DEBUG
        std::cout << "icase=" << cdt.flag.icase << " regex=" << cdt.flag.regex
                  << " wchar=" << cdt.flag.wchar << " norm=" << cdt.flag.norm
                  << " pattern=" << item->pattern
                  << " replace=" << item->replace << std::endl;
#endif
        if( cdt.flag.regex ){
            bool match = false;
            int offset = 0;
            const int lng_str = str_work.length();
            std::string str_tmp;

            // 文字列が空の時は"^$"などにマッチするように改行を追加
            if( lng_str == 0 ) str_work.push_back( '\n' );

            while( regex.match( item->creg, str_work, offset, match ) ){
                match = true;

                const int p0 = regex.pos( 0 );
                if( p0 != offset ) str_tmp += str_work.substr( offset, p0 - offset );

                // \0 ... \9 の文字列を置換
                str_tmp += regex.replace( item->replace );

                offset = p0 + regex.str( 0 ).length();
                if( offset >= lng_str ) break;

                // 0文字にマッチするパターンは繰り返さない
                if( regex.str( 0 ).length() == 0 ) break;
            }

            if( match ){
                if( lng_str > offset )
                    str_work = str_tmp + str_work.substr( offset, lng_str - offset );
                else
                    str_work = str_tmp;
            }
        }

        // 正規表現を使わない置換処理
        else if( cdt.flag.icase ) str_work = MISC::replace_casestr( str_work, item->pattern, item->replace );
        else str_work = MISC::replace_str( str_work, item->pattern, item->replace );
    }

#ifdef _DEBUG
    std::cout << "replaced str=" << str_work << std::endl;
#endif

    return str_work;
}


//
// 置換対象のXMLノード作成
//
XML::Dom* ReplaceStr_Manager::dom_append( XML::Dom* node, const int id, const bool chref )
{
    XML::Dom* const dir = node->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_DIR ) );

    dir->setAttribute( "name", target_name( id ) );
    dir->setAttribute( "chref", chref ? "true" : "false" );

    return dir;
}


//
// 置換条件のXMLノード作成
//
XML::Dom* ReplaceStr_Manager::dom_append( XML::Dom* node, const int condition, const std::string& pattern,
                                          const std::string& replace )
{
    XML::Dom* const query = node->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_REPLACESTR ) );

    query->setAttribute( "condition", condition );
    query->setAttribute( "pattern", pattern );
    query->setAttribute( "replace", replace );

    return query;
}
