// License: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "misccharcode.h"

#include <cstring>


// チェックする最大バイト数
#define CHECK_LIMIT 1024

/*--- 制御文字とASCII -----------------------------------------------*/

// [ 制御文字 ] 0x00〜0x1F 0x7F
#define CTRL_RNAGE( target ) ( target < 0x20 || target == 0x7F )

// [ ASCII ] 0x20〜0x7E
#define ASCII_RANGE( target ) ( (unsigned char)( target - 0x20 ) < 0x5F )

// [ 制御文字とASCII ] 0x00〜0x7F
#define CTRL_AND_ASCII_RANGE( target ) ( (unsigned char)target < 0x80 )


/*---- EUC-JP -------------------------------------------------------*/
//
// [ カナ ]
// 1バイト目 0x8E
#define EUC_CODE_KANA( target ) ( (unsigned char)target == 0x8E )
// 2バイト目 0xA1〜0xDF
#define EUC_RANGE_KANA( target ) ( (unsigned char)( target - 0xA1 ) < 0x3F )
//
// [ 補助漢字 ]
// 1バイト目 0x8F
#define EUC_CODE_SUB_KANJI( target ) ( (unsigned char)target == 0x8F )
//
// [ 漢字 ]
// 1バイト目 0xA1〜0xFE
// 2バイト目 0xA1〜0xFE
#define EUC_RANGE_MULTI( target ) ( (unsigned char)( target - 0xA1 ) < 0x5E )
//
bool MISC::is_euc( const char* input, size_t read_byte )
{
    if( ! input ) return false;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
        }
        // カナ
        else if( EUC_CODE_KANA( input[ byte ] )
            && EUC_RANGE_KANA( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // 補助漢字
        else if( EUC_CODE_SUB_KANJI( input[ byte ] )
                  && EUC_RANGE_MULTI( input[ byte + 1 ] )
                  && EUC_RANGE_MULTI( input[ byte + 2 ] ) )
        {
            byte += 3;
        }
        // 漢字
        else if( EUC_RANGE_MULTI( input[ byte ] )
                  && EUC_RANGE_MULTI( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // その他
        else
        {
            return false;
        }
    }

    return true;
}


/*---- ISO-2022-JP --------------------------------------------------*/
//
// エスケープシーケンスの開始文字 ESC(0x1B)
#define JIS_ESC_SEQ_START( target ) ( target == 0x1B )
//
bool MISC::is_jis( const char* input, size_t& byte )
{
    if( ! input ) return false;

    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // ESCが出現したか否かだけで判断
        if( JIS_ESC_SEQ_START( input[ byte ] ) ) return true;
        // JISに該当しないコード 0x80〜
        else if( ! CTRL_AND_ASCII_RANGE( input[ byte ] ) ) return false;

        ++byte;
    }

    // ループが終了していたら制御文字かアスキー
    return false;
}


/*---- Shift_JIS ----------------------------------------------------*/
//
// [ カナ ] 0xA1〜0xDF
#define SJIS_RANGE_KANA( target ) ( (unsigned char)( target - 0xA1 ) < 0x3F )
//
// [ 漢字 ]
// 1バイト目 0x81〜0x9F 0xE0〜0xFC( 0xEF )
//#define SJIS_RANGE_1( target ) ( (unsigned char)( target ^ 0x20 ) - 0xA1 < 0x2F )
#define SJIS_RANGE_1( target ) ( ( (unsigned char)target ^ 0x20 ) - 0xA1 < 0x3C )
// 0x81〜0x9F
#define SJIS_RANGE_1_H( target ) ( (unsigned char)( target - 0x81 ) < 0x1F )
// 0xE0〜0xFC
#define SJIS_RANGE_1_T( target ) ( (unsigned char)( target - 0xE0 ) < 0x1D )
//
// 2バイト目 0x40〜0x7E 0x80〜0xFC
#define SJIS_RANGE_2( target ) ( (unsigned char)( target - 0x40 ) < 0xBD && target != 0x7F )
// 0x40〜0x7E
#define SJIS_RANGE_2_H( target ) ( (unsigned char)( target - 0x40 ) < 0x3F )
// 0x80〜0xFC
#define SJIS_RANGE_2_T( target ) ( (unsigned char)( target - 0x80 ) < 0x7D )
//
bool MISC::is_sjis( const char* input, size_t read_byte )
{
    if( ! input ) return false;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
        }
        // カナ
        else if( SJIS_RANGE_KANA( input[ byte ] ) )
        {
            ++byte;
        }
        // 漢字(MS932)
        else if( SJIS_RANGE_1( input[ byte ] )
                  && SJIS_RANGE_2( input[ byte + 1 ] ) )
        {
            byte += 2;
        }
        // その他
        else
        {
            return false;
        }
    }

    return true;
}


namespace {
/*---- UTF ---------------------------------------------------------*/
//
// 0xC0・0xC1はセキュリティ上の問題で使用が禁止されている
//
// [ 1バイト目の範囲 ] 0xC2～0xFD [ RFC2279(破棄) ]
// [ 1バイト目の範囲 ] 0xC2～0xF4 [ RFC6329 ]
inline bool utf8_range1( std::uint8_t target ) { return static_cast< std::uint8_t >( target - 0xC2 ) < 0x33; }
//
// [ 2バイト目以降 ] 0x80～0xBF 先頭ビットが10
inline bool utf8_range_multi_byte( std::uint8_t target ) { return ( target & 0xC0 ) == 0x80; }
//
// [ 1バイト目 (2バイト文字) ] 先頭ビットが110
//#define UTF_FLAG_2( target ) ( ( target & 0xE0 ) == 0xC0 )
// [ 1バイト目 (2バイト文字) ] 0xC2～0xDF
inline bool utf8_flag2( std::uint8_t target ) { return static_cast< std::uint8_t >( target - 0xC2 ) < 0x1E; }
//
// [ 1バイト目 (3バイト文字) ] 先頭ビットが1110
inline bool utf8_flag3( std::uint8_t target ) { return ( target & 0xF0 ) == 0xE0; }
//
// [ 1バイト目 (4バイト文字) ] 先頭ビットが11110
inline bool utf8_flag4( std::uint8_t target ) { return ( target & 0xF8 ) == 0xF0; }
//
} // namespace
bool MISC::is_utf8( const char* input, const size_t length, size_t read_byte )
{
    if( ! input ) return false;

    bool valid = true;

    size_t byte = read_byte;
    const size_t input_length = strlen( input );

    while( byte < input_length && byte < CHECK_LIMIT )
    {
        // 制御文字かアスキー
        if( CTRL_AND_ASCII_RANGE( input[ byte ] ) )
        {
            ++byte;
            continue;
        }
        // UTF-8の1バイト目の範囲ではない
        else if( ! utf8_range1( input[ byte ] ) )
        {
            return false;
        }

        int byte_count = 1;

        // 4,3,2バイト
        if( utf8_flag4( input[ byte ] ) ) byte_count = 4;
        else if( utf8_flag3( input[ byte ] ) ) byte_count = 3;
        else if( utf8_flag2( input[ byte ] ) ) byte_count = 2;

        ++byte;

        // 2バイト目以降
        while( byte_count > 1 )
        {
            if( utf8_range_multi_byte( input[ byte ] ) )
            {
                ++byte;
            }
            else
            {
                valid = false;
                break;
            }
            --byte_count;
        }
    }

    return valid;
}


//
// 日本語文字コードの判定
//
// 各コードの判定でtrueの間は文字数分繰り返されるので
// 速度の求められる繰り返し処理などで使わない事
//
int MISC::judge_char_code( const std::string& str )
{
    int code = CHARCODE_UNKNOWN;

    if( str.empty() ) return code;

    size_t read_byte = 0;

    // JISの判定
    if( is_jis( str.c_str(), read_byte ) ) code = CHARCODE_JIS;
    // JISの判定で最後まで進んでいたら制御文字かアスキー
    else if( read_byte == str.length() ) code = CHARCODE_ASCII;
    // is_jis()でASCII範囲外のバイトが現れた箇所から判定する
    // UTF-8の範囲
    else if( is_utf( str.c_str(), read_byte ) ) code = CHARCODE_UTF;
    // EUC-JPの範囲
    else if( is_euc( str.c_str(), read_byte ) ) code = CHARCODE_EUC_JP;
    // Shift_JISの範囲
    else if( is_sjis( str.c_str(), read_byte ) ) code = CHARCODE_SJIS;

    return code;
}


//
// utf-8 byte数
//
// 入力 : utfstr 入力文字 (UTF-8)
// 戻り値 :  byte  長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を返す
//
int MISC::utf8bytes( const char* utfstr )
{
    int byte = 0;

    if( utfstr && *utfstr != '\0' ){
        const unsigned char ch = static_cast< unsigned char >( *utfstr );
        if( ch <= 0x7f ) byte = 1;
        else if( utf8_flag3( ch ) ) byte = 3;
        else if( utf8_flag4( ch ) ) byte = 4;
        else if( utf8_flag2( ch ) ) byte = 2;
#ifdef _DEBUG
        else { // 不正なUTF8
            std::cout << "MISC::utf8bytes : invalid 1st byte: char = "
                      << static_cast< unsigned int >( ch ) << std::endl;
        }
#endif
    }

    for( int i = 1; i < byte; ++i ){
        if( ! utf8_range_multi_byte( utfstr[ i ] ) ){
#ifdef _DEBUG
            // 不正なUTF8
            std::cout << "MISC::utf8bytes : invalid code: char = " << static_cast< unsigned int >( utfstr[ 0 ] );
            std::cout << ", " << static_cast< unsigned int >( utfstr[ 1 ] );
            if( byte > 2 ) std::cout << ", " << static_cast< unsigned int >( utfstr[ 2 ] );
            if( byte > 3 ) std::cout << ", " << static_cast< unsigned int >( utfstr[ 3 ] );
            std::cout << std::endl;
#endif
            byte = 0;
            break;
        }
    }

    return byte;
}


//
// utf-8 -> code point 変換
//
// 入力 : utfstr 入力文字 (UTF-8)
// 出力 :  byte  長さ(バイト) utfstr が ascii なら 1, UTF-8 なら 2 or 3 or 4 を入れて返す
// 戻り値 : code point
//
char32_t MISC::utf8tocp( const char* utfstr, int& byte )
{
    char32_t code = 0;
    byte = utf8bytes( utfstr );

    switch( byte ){
    case 1:
        code =  utfstr[ 0 ];
        break;

    case 2:
        code = utfstr[ 0 ] & 0x1f;
        code = ( code << 6 ) + ( utfstr[ 1 ] & 0x3f );
        break;

    case 3:
        code = utfstr[ 0 ] & 0x0f;
        code = ( code << 6 ) + ( utfstr[ 1 ] & 0x3f );
        code = ( code << 6 ) + ( utfstr[ 2 ] & 0x3f );
        break;

    case 4:
        code = utfstr[ 0 ] & 0x07;
        code = ( code << 6 ) + ( utfstr[ 1 ] & 0x3f );
        code = ( code << 6 ) + ( utfstr[ 2 ] & 0x3f );
        code = ( code << 6 ) + ( utfstr[ 3 ] & 0x3f );
        break;

    default:
        break;
    }

    return code;
}
