// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"
#include "aamanager.h"

#include "jdlib/miscutil.h"

#include "xml/document.h"
#include "xml/tools.h"

#include "config/globalconf.h"

#include "cache.h"
#include "type.h"

CORE::AAManager* instance_aamanager = nullptr;

CORE::AAManager* CORE::get_aamanager()
{
    if( ! instance_aamanager ) instance_aamanager = new CORE::AAManager();
    return instance_aamanager;
}


void CORE::delete_aamanager()
{
    if( instance_aamanager ) delete instance_aamanager;
    instance_aamanager = nullptr;
}


////////////////////////////////////


// ルート要素名
#define ROOT_NODE_NAME "history"

enum
{
    AA_LIMIT = 4096
};

using namespace CORE;

AAManager::AAManager()
{
#ifdef _DEBUG
    std::cout << "AAManager::AAManager\n";
#endif

    load_label();
    load_history();
}


AAManager::~AAManager()
{
#ifdef _DEBUG
    std::cout << "AAManager::~AAManager\n";
#endif
}


//
// ラベル、AA読み込み
//
void AAManager::load_label()
{
    std::string aa_lines;
    if( CACHE::load_rawdata( CACHE::path_aalist(), aa_lines ) ){

        std::list< std::string > list_label = MISC::get_lines( aa_lines );
        list_label = MISC::remove_nullline_from_list( list_label );

        std::list< std::string >::iterator it = list_label.begin();
        for( int id = 0 ; it != list_label.end() ; ++it, ++id )
        {
            std::string asciiart = *it;
            m_vec_label.push_back( asciiart );

#ifdef _DEBUG
            std::cout << "id = " << id << " label = " << asciiart << std::endl;
#endif

            // 先頭に"**"がある場合は、"*"を一つ取り除いて一行AAとして扱う
            if( asciiart.rfind( "**", 0 ) == 0 )
            {
                // **example -> *example
                asciiart.erase( 0, 1 );
            }
            // "*"が一つだけある場合は、"*"を取り除いてファイル名とみなす
            else if( asciiart.rfind( '*', 0 ) == 0 )
            {
                // *example -> example
                asciiart.erase( 0, 1 );

                // example -> .jd/aa/example
                std::string aafile_path = CACHE::path_aadir().append( asciiart );

#ifdef _DEBUG
                std::cout << "load : " << aafile_path << std::endl;
#endif

                // ファイルが存在しなければ".txt"を追加( .jd/aa/example -> .jd/aa/example.txt )
                if( CACHE::file_exists( aafile_path ) != CACHE::EXIST_FILE ) aafile_path.append( ".txt" );

                // ファイル読み込み
                if( CACHE::file_exists( aafile_path ) != CACHE::EXIST_FILE ) asciiart = aafile_path + " が存在しません";
                else if( CACHE::get_filesize( aafile_path ) > AA_LIMIT ) asciiart = "ファイルサイズが大きすぎます";
                else CACHE::load_rawdata( aafile_path, asciiart );
            }

#ifdef _DEBUG
            std::cout << "AA : " << asciiart << std::endl;
#endif
            m_vec_aa.push_back( asciiart );

            // ショートカット
            char shortcut = 0;
            const int base_a = 10;
            const int base_A = base_a + 'z' - 'a' -4 + 1;
            if( id <= 9 ) shortcut = '0' + id;
            else if( id <= base_a + 'z' - 'a' -4 ){
                shortcut = 'a' + id - base_a;
                if( shortcut >= 'h' ) ++shortcut; // hを除く
                if( shortcut >= 'j' ) shortcut += 3; // j,k,lを除く
            }
            else if( id <= base_A + 'Z' - 'A' ){
                shortcut = 'A' + id - base_A;
            }
            if( shortcut ) m_map_shortcut.insert( std::make_pair( id, shortcut ) );
        }
    }
}


//
// 履歴読み込み
//
void AAManager::load_history()
{
#ifdef _DEBUG
    std::cout << "AAManager::load_history\n";
#endif

    std::string xml;
    if( ! CACHE::load_rawdata( CACHE::path_aahistory(), xml ) ) return;

#ifdef _DEBUG
    std::cout << xml << std::endl;
#endif

    const XML::Document document( xml );
    const XML::Dom* root = document.get_root_element( std::string( ROOT_NODE_NAME ) );
    if( ! root ) return;

    std::list< std::string > tmp_history;
    for( const XML::Dom* child : *root ) {
        if( static_cast<int>( tmp_history.size() ) >= CONFIG::get_aahistory_size() ) break;

        if( child->nodeType() == XML::NODE_TYPE_ELEMENT ){

            const int type = XML::get_type( child->nodeName() );
            std::string name = child->getAttribute( "name" );

            if( type == TYPE_AA && ! name.empty() ) tmp_history.push_back( std::move( name ) );
        }
    }

    std::vector< bool > tmp_vec;
    tmp_vec.resize( get_size() );

    for( const std::string& hist : tmp_history ) {

        for( int i = 0; i < get_size() ; ++i ){
            if( ! tmp_vec[ i ] && m_vec_label[ i ] == hist ){
                m_history.push_back( i );
                tmp_vec[ i ] = true;
                break;
            }
        }
    }

#ifdef _DEBUG
    for( const int index : m_history ) std::cout << index << std::endl;
#endif
}


//
// 履歴保存
//
void AAManager::save_history()
{
#ifdef _DEBUG
    std::cout << "AAManager::save_history\n";
#endif

    XML::Document document;
    XML::Dom* root = document.appendChild( XML::NODE_TYPE_ELEMENT, std::string( ROOT_NODE_NAME ) );

    for( const int index : m_history ) {

        const std::string& name = m_vec_label[ index ];

        if( ! name.empty() ){

            XML::Dom* node = root->appendChild( XML::NODE_TYPE_ELEMENT, XML::get_name( TYPE_AA ) );
            node->setAttribute( "name", name );
        }
    }

    std::string xml;
    if( root->hasChildNodes() ) xml = document.get_xml();

#ifdef _DEBUG
    std::cout << xml << std::endl;
#endif

    if( ! xml.empty() ) CACHE::save_rawdata( CACHE::path_aahistory(), xml );
}


// ラベル、AA取得
std::string AAManager::get_label( const int id ) const
{
    if( id >= (int) m_vec_label.size() ) return std::string();
    return m_vec_label[ id ];
}

std::string AAManager::get_aa( const int id ) const
{
    if( id >= (int) m_vec_aa.size() ) return std::string();
    return m_vec_aa[ id ]; 
}


//
// ショートカットキー取得
//
std::string AAManager::id2shortcut( const int id )
{
    if( id >= (int) m_map_shortcut.size() ) return std::string();

#ifdef _DEBUG
    std::cout << "AAManager::id2shortcut id = " << id << std::endl;
#endif

    int key = m_map_shortcut[ id ];
    if( !key ) return std::string();

    char tmpchar[2];
    tmpchar[0] = key;
    tmpchar[1] = '\0';
    return std::string( tmpchar );
}


// ショートカットからid取得
int AAManager::shortcut2id( const char key ) const
{
    if( key == '\0' ) return -1;

    auto it = std::find_if( m_map_shortcut.cbegin(), m_map_shortcut.cend(),
                            [key]( const auto& id_key ) { return id_key.second == key; } );
    if( it != m_map_shortcut.cend() ) {
        return it->first;
    }

    return -1;
}



// id 番を履歴に追加
void AAManager::append_history( const int id )
{
    if( id >= 0 && id < get_size() ){

        // 既に履歴に含まれている場合
        auto it = std::find( m_history.cbegin(), m_history.cend(), id );
        if( it != m_history.end() ) {
            m_history.splice( m_history.cbegin(), m_history, it );
            return;
        }

        // 含まれていない場合
        const int size = CONFIG::get_aahistory_size();
        if( size > 0 ) {
            if( static_cast<int>( m_history.size() ) >= size ) m_history.resize( size - 1 );
            m_history.push_front( id );
        }
    }
}


// num 番目の履歴をIDに変換
int AAManager::history2id( const int num ) const
{
    if( num < 0 || num >= get_historysize() ) return -1;

    auto it = std::next( m_history.cbegin(), num );

#ifdef _DEBUG
    std::cout << "AAManager::conv_history2id " << num << " -> " << *it << std::endl;
#endif

    return *it;
}
