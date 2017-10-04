// License GPL2

// std::list< Dom* > の代わりのクラス
//
// Domクラスで std::list のメンバ関数の front() と back() の戻り値が
// 存在しないインスタンスへの参照になっていると都合が悪いので、その対策
// のためだけに作られたアホみたいなクラスです。ついでに添字を使えるよう
// にしてあります。

#ifndef _DOMLIST_H
#define _DOMLIST_H

#include <list>
#include <cstddef>

namespace XML
{
    class Dom;

    class DomList
    {
        std::list< Dom* > m_list;

        //DomList( const DomList& );

      public:

        DomList() = default;
        ~DomList() noexcept = default;

        // std::list< Dom* > が代入された場合
        DomList& operator =( const std::list< Dom* >& list ){ m_list = list; return *this; }

        // 添字によるアクセス
        Dom* operator []( const unsigned int n );

        std::list< Dom* > get_list() const noexcept { return m_list; }

        std::list< Dom* >::iterator begin() noexcept { return m_list.begin(); }
        std::list< Dom* >::reverse_iterator rbegin() noexcept { return m_list.rbegin(); }
        std::list< Dom* >::iterator end() noexcept { return m_list.end(); }
        std::list< Dom* >::reverse_iterator rend() noexcept { return m_list.rend(); }
        size_t size() const noexcept { return m_list.size(); }
        size_t max_size() const noexcept { return m_list.max_size(); }
        bool empty() const noexcept { return m_list.empty(); }
        Dom* front(); // front() と back()は参照ではなくポインタを返す
        Dom* back();
        void push_front( Dom* dom ){ m_list.push_front( dom ); }
        void push_back( Dom* dom ){ m_list.push_back( dom ); }
        void pop_front(){ m_list.pop_front(); }
        void pop_back(){ m_list.pop_back(); }
        std::list< Dom* >::iterator insert( std::list< Dom* >::iterator it, Dom* dom ){ return m_list.insert( it, dom ); }
        std::list< Dom* >::iterator erase( std::list< Dom* >::iterator it ){ return m_list.erase( it ); }
        void clear(){ m_list.clear(); }
        void splice( std::list< Dom* >::iterator it, DomList& domlist ){ m_list.splice( it, domlist.m_list ); }
        void remove( Dom* dom ){ m_list.remove( dom ); }
        void unique(){ m_list.unique(); }
        void merge( DomList& domlist ){ m_list.merge( domlist.m_list ); }
        void sort(){ m_list.sort(); }
        void reverce(){ m_list.reverse(); }
        void swap( DomList& domlist ){ m_list.swap( domlist.m_list ); }
    };
}

#endif
