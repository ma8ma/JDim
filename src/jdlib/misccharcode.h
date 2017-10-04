// License: GPL2

// 日本語文字コードの判定

#ifndef _MISCCHARCODE_H
#define _MISCCHARCODE_H

#include <string>

namespace MISC
{
    enum CodeSet
    {
        CHARCODE_UNKNOWN = -1,
        CHARCODE_ASCII = 0,
        CHARCODE_EUC_JP,
        CHARCODE_JIS,
        CHARCODE_SJIS,
        CHARCODE_UTF
    };

    bool is_euc( const char* input, size_t read_byte );
    bool is_jis( const char* input, size_t& read_byte );
    bool is_sjis( const char* input, size_t read_byte );
    bool is_utf8( const char* input, size_t read_byte );
    int judge_char_code( const std::string& str );

    // utf-8 -> code point 変換
    // 入力 : utfstr 入力文字 (UTF-8)
    // 出力 :  byte  長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
    // 戻り値 : unicode code point
    char32_t utf8tocp( const char* utfstr, int& byte );

    // utf-8文字のbyte数
    int utf8bytes( const char* utfstr );
}

#endif
