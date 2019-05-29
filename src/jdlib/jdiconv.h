// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include "charcode.h"

#include <iconv.h>
#include <vector>


constexpr int BUF_SIZE_ICONV_OUT = 512 * 1024;


namespace JDLIB
{
    class Iconv
    {
        iconv_t m_cd;

        std::vector< char > m_buf;

        CharCode m_coding_from;
        CharCode m_coding_to;

    public:
        
        Iconv( const CharCode from, const CharCode to );
        ~Iconv();

        const char* convert( const char* str_in, int size_in, int& size_out );

    private:
        bool grow();
    };
}

#endif
