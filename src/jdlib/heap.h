// ライセンス: GPL2

// ヒープクラス

#ifndef HEAP_H
#define HEAP_H

#include <list>
#include <memory>
#include <string>

namespace JDLIB
{
    class HEAP
    {
        std::list< std::unique_ptr<unsigned char[]> >m_heap_list;
        std::size_t m_max;  // ブロックサイズ
        std::size_t m_space = 0; // ブロックの未使用サイズ
        void* m_ptr = nullptr;

      public:
        HEAP( std::size_t blocksize ) noexcept;
        ~HEAP();

        HEAP( const HEAP& ) = delete;
        HEAP& operator=( const HEAP& ) = delete;
        HEAP( HEAP&& ) noexcept = default;
        HEAP& operator=( HEAP&& ) = default;

        void clear();

        void* heap_alloc( std::size_t size, std::size_t align );
        template<typename T>
        void* heap_alloc( std::size_t len = 1 )
        {
            return heap_alloc( sizeof( T ) * len, alignof( T ) );
        }
    };
}

#endif
