// License: GPL2

// 日本語文字コードの判定

#ifndef _MISCCHARCODE_H
#define _MISCCHARCODE_H

#include "charcode.h"

#include <string>


namespace MISC
{
    // get_ucstype()の戻り値
    enum class UcsType
    {
        BasicLatin = 0,
        Hira,
        Kata,

        Other,
    };


    // utf8_fix_wavedash のモード
    enum class WaveDashFix
    {
        UnixToWin = 0,
        WinToUnix,
    };


    const char* charcode_to_cstr( const CharCode charcode );
    CharCode charcode_from_cstr( const char* encoding );
    bool is_eucjp( const char* input, const size_t length, size_t read_byte );
    bool is_jis( const char* input, const size_t length, size_t& read_byte );
    bool is_sjis( const char* input, const size_t length, size_t read_byte );
    bool is_utf8( const char* input, const size_t length, size_t read_byte );
    CharCode judge_char_code( const std::string& str );

    // utf-8 -> code point 変換
    // 入力 : utfstr 入力文字 (UTF-8)
    // 出力 :  byte  長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
    // 戻り値 : unicode code point
    char32_t utf8tocp( const char* utfstr, int& byte );

    // utf-8文字のbyte数
    int utf8bytes( const char* utfstr );

    // ucs の種類
    UcsType get_ucstype( const char32_t code );

    // WAVEDASHなどのWindows系UTF-8文字をUnix系文字と相互変換
    std::string utf8_fix_wavedash( const std::string& str, const WaveDashFix mode );

    // 文字コードを from から to に変換
    // 遅いので連続的な処理が必要な時は使わないこと
    std::string Iconv( const std::string& str, const CharCode to, const CharCode from );

}

#endif
