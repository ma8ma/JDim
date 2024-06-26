// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "articlejbbs.h"
#include "nodetreejbbs.h"
#include "interface.h"

#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include <sstream>

using namespace DBTREE;

ArticleJBBS::ArticleJBBS( const std::string& datbase, const std::string& _id, bool cached, const Encoding enc )
    : ArticleBase( datbase, _id, cached, enc )
{
    assert( ! get_id().empty() );

    // JBBS の場合は拡張子が無いので key = id
    set_key( get_id() );

    // key から since 計算
    set_since_time( atol( get_key().c_str() ) );
}


ArticleJBBS::~ArticleJBBS() noexcept = default;


std::string ArticleJBBS::create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
{
    if( msg.empty() ) return std::string();

    // DIR と BBS を分離する( ID = DIR/BBS )
    const std::string& boardid = DBTREE::board_id( get_url() );
    auto i = boardid.find( '/' );
    std::string_view dir = std::string_view{ boardid }.substr( 0, i );
    std::string_view bbs = std::string_view{ boardid }.substr( i + 1 );

    std::stringstream ss_post;
    ss_post.clear();
    ss_post << "BBS="      << bbs
            << "&KEY="     << get_key()
            << "&DIR="     << dir
            << "&TIME="    << get_time_modified()
            << "&submit="  << MISC::url_encode_plus( "書き込む", get_encoding() )
            << "&NAME="    << MISC::url_encode_plus( name, get_encoding() )
            << "&MAIL="    << MISC::url_encode_plus( mail, get_encoding() )
            << "&MESSAGE=" << MISC::url_encode_plus( msg, get_encoding() );

#ifdef _DEBUG
    std::cout << "Articlejbbs::create_write_message " << ss_post.str() << std::endl;
#endif

    return ss_post.str();
}




//
// bbscgi(write.cgi) のURL
//
// (例) "http://jbbs.shitaraba.net/bbs/write.cgi/computer/123/1234567/"
//
//
std::string ArticleJBBS::url_bbscgi() const
{
    return DBTREE::url_bbscgibase( get_url() ) + DBTREE::board_id( get_url() ) + "/" + get_key() + "/";
}


//
// subbbscgi のURL
//
// (例) "http://jbbs.shitaraba.net/bbs/write.cgi/computer/123/1234567/"
//
std::string ArticleJBBS::url_subbbscgi() const
{
    return DBTREE::url_subbbscgibase( get_url() ) + DBTREE::board_id( get_url() ) + "/" + get_key() + "/";
}



NodeTreeBase* ArticleJBBS::create_nodetree()
{
#ifdef _DEBUG
    std::cout << "ArticleJBBS::create_nodetree " << get_url() << std::endl;
#endif

    return new NodeTreeJBBS( get_url(), get_date_modified() );
}
