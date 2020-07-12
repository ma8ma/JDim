// ライセンス: GPL2

// スレッドクラス

#ifndef _JDTHREAD_H
#define _JDTHREAD_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <thread>
#define JDTH_TYPE std::thread
#define JDTH_ISRUNNING( pth ) ( ( pth ).get_id() != std::thread::id() )
#define JDTH_CLEAR( pth ) ( ( pth ) = std::thread() )

typedef void* ( *STARTFUNC )( void * );

enum
{
    DEFAULT_STACKSIZE = 64
};

namespace JDLIB
{
    enum
    {
        DETACH = true,
        NODETACH = false
    };

    class Thread
    {
        JDTH_TYPE m_thread;

    public:

        Thread();
        virtual ~Thread();

        bool is_running() const { return JDTH_ISRUNNING( m_thread ); }

        // スレッド作成
        bool create( STARTFUNC func , void * arg, const bool detach, const int stack_kbyte = DEFAULT_STACKSIZE );

        bool join();
    };
}

#endif
