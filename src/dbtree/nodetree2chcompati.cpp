// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "nodetree2chcompati.h"
#include "interface.h"

#include "jdlib/jdiconv.h"
#include "jdlib/loaderdata.h"
#include "jdlib/miscmsg.h"

#include "config/globalconf.h"

using namespace DBTREE;


NodeTree2chCompati::NodeTree2chCompati( const std::string& url, const std::string& date_modified )
    : NodeTreeBase( url, date_modified )
{
#ifdef _DEBUG
    std::cout << "NodeTree2chCompati::NodeTree2chCompati url = " << url << " modified = " << date_modified << std::endl;
#endif
}




NodeTree2chCompati::~NodeTree2chCompati()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::~NodeTree2chCompati : " << get_url() << std::endl;
#endif
}



//
// バッファなどのクリア
//
void NodeTree2chCompati::clear()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::clear : " << get_url() << std::endl;
#endif

    NodeTreeBase::clear();

    m_iconv.reset();
}



//
// ロード実行前に呼ぶ初期化関数
//
void NodeTree2chCompati::init_loading()
{
#ifdef _DEBUG    
    std::cout << "NodeTree2chCompati::init_loading : " << get_url() << std::endl;
#endif

    NodeTreeBase::init_loading();

    // iconv 初期化
    if( ! m_iconv ) m_iconv.reset( new JDLIB::Iconv( DBTREE::article_charcode( get_url() ), CHARCODE_UTF8 ) );
}



//
// raw データを dat 形式に変換
//
// 2ch型サーバの場合は文字コードを変換するだけ
//
const char* NodeTree2chCompati::raw2dat( char* rawlines, int& byte )
{
    assert( m_iconv );

    // バッファ自体はiconvクラスの中で持っているのでポインタだけもらう
    return  m_iconv->convert( rawlines, strlen( rawlines ), byte );
}



//
// ロード用データ作成
//
void NodeTree2chCompati::create_loaderdata( JDLIB::LOADERDATA& data )
{
    data.url = get_url();
    data.agent = DBTREE::get_agent( get_url() );

    // 1byte前からレジュームして '\n' が返ってこなかったらあぼーんがあったってこと
    if( get_lng_dat() ) {
        data.byte_readfrom = get_lng_dat() -1;
        set_resume( true );
    }
    else set_resume( false );

    data.host_proxy = DBTREE::get_proxy_host( get_url() );
    data.port_proxy = DBTREE::get_proxy_port( get_url() );
    data.basicauth_proxy = DBTREE::get_proxy_basicauth( get_url() );
    data.size_buf = CONFIG::get_loader_bufsize();
    data.timeout = CONFIG::get_loader_timeout();
    data.basicauth = DBTREE::board_basicauth( get_url() );

    if( ! get_date_modified().empty() ) data.modified = get_date_modified();
}
