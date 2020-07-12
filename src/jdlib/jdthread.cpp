// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdthread.h"
#include "miscmsg.h"

#include <limits.h>
#include <cstring>
#include <system_error>

using namespace JDLIB;


#ifdef _DEBUG
template < class CharT, class Traits >
static std::basic_ostream< CharT, Traits >&
operator<<( std::basic_ostream< CharT, Traits >& ost, const std::thread& pth )
{
    return ost << pth.get_id();
}
#endif // _DEBUG


Thread::Thread()
{
    JDTH_CLEAR( m_thread );
}


Thread::~Thread()
{
#ifdef _DEBUG
    std::cout << "Thread::~Thread\n";
    assert( ! is_running() );
#endif

    join();
}


// スレッド作成
bool Thread::create( STARTFUNC func , void* arg, const bool detach, const int stack_kbyte )
{
    if( JDTH_ISRUNNING( m_thread ) ){
        MISC::ERRMSG( "Thread::create : thread is already running" );
        return false;
    }

    static_cast< void >( stack_kbyte );

#ifdef _DEBUG
    std::cout << "Thread::create (stdthread)\n";
#endif
    try {
        m_thread = std::thread( func, arg );
    }
    catch( std::system_error& err ) {
        MISC::ERRMSG( err.what() );
        return false;
    }

    if( detach ) {
#ifdef _DEBUG
        std::cout << "detach\n";
#endif
        m_thread.detach();
        assert( m_thread.get_id() == std::thread::id() );
    }

#ifdef _DEBUG
    std::cout << "thread = " << m_thread << std::endl;
#endif

    return true;
}


bool Thread::join()
{
    if( ! JDTH_ISRUNNING( m_thread ) ) return true;

#ifdef _DEBUG
    std::cout << "Thread:join thread = " << m_thread << std::endl;
#endif

    if( m_thread.joinable() ) {
        m_thread.join();
    }
    JDTH_CLEAR( m_thread );

    return true;
}
