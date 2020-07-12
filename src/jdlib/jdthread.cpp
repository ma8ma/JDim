// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "jdthread.h"
#include "miscmsg.h"

#include <system_error>

using namespace JDLIB;


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
    if( m_thread.get_id() != std::thread::id() ){
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
    std::cout << "thread = " << m_thread.get_id() << std::endl;
#endif

    return true;
}


bool Thread::join()
{
    if( m_thread.get_id() == std::thread::id() ) return true;

#ifdef _DEBUG
    std::cout << "Thread:join thread = " << m_thread.get_id() << std::endl;
#endif

    if( m_thread.joinable() ) {
        m_thread.join();
    }
    m_thread = std::thread();

    return true;
}
