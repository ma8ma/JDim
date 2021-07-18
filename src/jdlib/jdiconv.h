// ライセンス: GPL2

#ifndef _JDICONV_H
#define _JDICONV_H

#include "charcode.h"

#include <string>
#include <vector>

#include <gmodule.h> // GIConv


constexpr int BUF_SIZE_ICONV_OUT = 512 * 1024;


namespace JDLIB
{
    class Iconv
    {
        GIConv m_cd; // iconv実装は環境で違いがあるためGlibのラッパーAPIを利用する

        std::vector< char > m_buf;

        CharCode m_coding_from;
        CharCode m_coding_to;

    public:
        
        Iconv( const CharCode to, const CharCode from );
        ~Iconv();

        const char* convert( const char* str_in, int size_in, int& size_out );

    private:
        bool grow();
    };
}

#endif
