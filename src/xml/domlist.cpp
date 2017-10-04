// License GPL2

//#define _DEBUG
#include "jddebug.h"

#include "domlist.h"


using namespace XML;


// 添字によるアクセス
Dom* DomList::operator []( const unsigned int n )
{
    if( m_list.empty() || n > m_list.size() ) return nullptr;

    size_t count = 0;
    std::list< Dom* >::iterator it = m_list.begin();
    while( it != m_list.end() )
    {
        if( count == n ) return *it;
        ++count;
        ++it;
    }

    return nullptr;
}


// 参照ではなくポインタを返す
Dom* DomList::front()
{
    if( m_list.empty() ) return nullptr;

    return *m_list.begin();
}

// 参照ではなくポインタを返す
Dom* DomList::back()
{
    if( m_list.empty() ) return nullptr;

    return *m_list.end();
}
