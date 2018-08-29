// ライセンス: GPL2

//#define _DEBUG
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "jddebug.h"

#include "dispatchmanager.h"

#include "skeleton/dispatchable.h"

#ifdef WITH_STD_THREAD
#include <mutex>
typedef std::lock_guard< std::mutex > LockGuard;
#else
typedef Glib::Mutex::Lock LockGuard;
#endif


#ifdef WITH_STD_THREAD
static std::mutex dispatch_mutex;
#else
Glib::StaticMutex dispatch_mutex = GLIBMM_STATIC_MUTEX_INIT;
#endif
CORE::DispatchManager* instance_dispmanager = NULL;


CORE::DispatchManager* CORE::get_dispmanager()
{
    if( ! instance_dispmanager ) instance_dispmanager = new CORE::DispatchManager();
    return instance_dispmanager;
}

void CORE::delete_dispatchmanager()
{
    if( instance_dispmanager ) delete instance_dispmanager;
    instance_dispmanager = NULL;
}


//////////////////////


using namespace CORE;


DispatchManager::DispatchManager()
{
#ifdef _DEBUG
    std::cout << "DispatchManager::DispatchManager\n";
#endif

    m_dispatch.connect( sigc::mem_fun( *this, &DispatchManager::slot_dispatch ) );
}


DispatchManager::~DispatchManager()
{
#ifdef _DEBUG
    std::cout << "DispatchManager::~DispatchManager size = " << m_children.size() << std::endl;
#endif
}


void DispatchManager::add( SKELETON::Dispatchable* child )
{
    LockGuard lock( dispatch_mutex );

    // 既にlistに登録されていたらキャンセルする
    std::list< SKELETON::Dispatchable* >::iterator it = m_children.begin();
    for( ; it != m_children.end(); ++it ){
        if( *it == child ){
#ifdef _DEBUG
            std::cout << "DispatchManager::add canceled\n";
#endif
            return;
        }
    }

    m_children.push_back( child );
    m_dispatch.emit();

#ifdef _DEBUG
    std::cout << "DispatchManager::add size = " << m_children.size() << std::endl;
#endif
}


void DispatchManager::remove( SKELETON::Dispatchable* child )
{
    LockGuard lock( dispatch_mutex );

    size_t size = m_children.size();
    if( ! size  ) return;

    m_children.remove( child );

#ifdef _DEBUG
    if( size != m_children.size() ) std::cout << "!!!!!!!\nDispatchManager::remove size "
                                              << size << " -> " << m_children.size() << "\n!!!!!!!\n";
#endif
}


void DispatchManager::slot_dispatch()
{
#ifdef WITH_STD_THREAD
    std::unique_lock< std::mutex > lock( dispatch_mutex );
#else
    Glib::Mutex::Lock lock( dispatch_mutex );
#endif

    const size_t size = m_children.size();
    if( ! size  ) return;

    SKELETON::Dispatchable* child = *( m_children.begin() );

    // child->callback_dispatch()の中で再び Dispatchable::add()が呼び出されると
    // キャンセルされてしまうので callback_dispatch() を呼び出す前にremoveする
    m_children.remove( child );
#ifdef WITH_STD_THREAD
    lock.unlock();
#else
    lock.release();
#endif

    if( child ) child->callback_dispatch();

#ifdef _DEBUG
    std::cout << "DispatchManager::slot_dispatch size = " << size << " -> " << m_children.size() << std::endl;
#endif
}
