// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "article2ch.h"
#include "nodetree2ch.h"
#include "interface.h"

#include "jdlib/misccharcode.h"
#include "jdlib/miscutil.h"
#include "jdlib/misctime.h"

#include "config/globalconf.h"

#include "login2ch.h"

#include <sstream>

using namespace DBTREE;


Article2ch::Article2ch( const std::string& datbase, const std::string& id, bool cached, const CharCode charcode )
    : Article2chCompati( datbase, id, cached, charcode )
{}


Article2ch::~Article2ch() noexcept = default;


// 書き込みメッセージ変換
std::string Article2ch::create_write_message( const std::string& name, const std::string& mail, const std::string& msg )
{
    if( msg.empty() ) return std::string();

    std::stringstream ss_post;
    ss_post << "FROM=" << MISC::url_encode( name, get_charcode() )
            << "&mail=" << MISC::url_encode( mail, get_charcode() )
            << "&MESSAGE=" << MISC::url_encode( msg, get_charcode() )
            << "&bbs=" << DBTREE::board_id( get_url() )
            << "&key=" << get_key()
            << "&time=" << get_time_modified()
            << "&submit=" << MISC::url_encode( std::string{ "書き込む" }, get_charcode() )
            // XXX: ブラウザの種類に関係なく含めて問題ないか？
            << "&oekaki_thread1=";

    // キーワード( hana=mogera や suka=pontan など )
    const std::string& keyword = DBTREE::board_keyword_for_write( get_url() );
    if( ! keyword.empty() ) ss_post << "&" << keyword;

    // ログイン中
    if( CORE::get_login2ch()->login_now() && ! CORE::get_login2ch()->get_username().empty() ){
        const std::string sid = CORE::get_login2ch()->get_sessionid();
        ss_post << "&sid=" << MISC::url_encode( sid );
    }

#ifdef _DEBUG
    std::cout << "Article2ch::create_write_message " << ss_post.str() << std::endl;
#endif

    // 書き込みメッセージを作成したらキーワードはリセットする
    DBTREE::board_set_keyword_for_write( get_url(), std::string{} );

    return ss_post.str();
}


NodeTreeBase* Article2ch::create_nodetree()
{
    return new NodeTree2ch( get_url(), get_org_url(), get_date_modified(), get_since_time() );
}



//
// dat落ちしたスレをロードするか
//
bool Article2ch::is_load_olddat() const
{
    // 2chにログインしている場合
    // または、read.cgiを使う設定の場合
    return ( CORE::get_login2ch()->login_now() && ! CORE::get_login2ch()->get_username().empty() )
            || CONFIG::get_use_external_log();
}
