// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "heap.h"

using namespace JDLIB;

HEAP::HEAP( std::size_t blocksize ) noexcept
    : m_max( blocksize )
{
#ifdef _DEBUG
    std::cout << "HEAP::HEAP: block size = " << m_max << std::endl;
#endif
}


HEAP::~HEAP()
{
#ifdef _DEBUG
    std::cout << "HEAP::~HEAP: block size = " << m_max << " number = " << m_heap_list.size() << std::endl;
#endif

    clear();
}


void HEAP::clear()
{
#ifdef _DEBUG
    std::cout << "HEAP::clear: block size = " << m_max << " number = " << m_heap_list.size() << std::endl;
#endif

    m_space = 0;
    m_heap_list.clear();
}


void* HEAP::heap_alloc( std::size_t size, std::size_t align )
{
    assert( m_max > size && size > 0 );
    assert( size >= align );

    while(1) {
        if( !m_ptr || m_space < size ) {
            m_heap_list.emplace_back( new unsigned char[m_max]{} );
            m_ptr = &m_heap_list.back()[0];
            m_space = m_max;
#ifdef _DEBUG
            std::cout << "HEAP::heap_alloc: block size = " << m_max << " number = " << m_heap_list.size() << std::endl;
#endif
        }

        // アライメント調整された指定サイズのバッファを探す
        if(std::align( align, size, m_ptr, m_space )) {
            void* result = m_ptr;
            m_ptr = std::next( reinterpret_cast<unsigned char*>(m_ptr), size );
            m_space -= size;
            return result;
        }
        else {
            m_ptr = nullptr;
            m_space = 0;
        }
    }
}
