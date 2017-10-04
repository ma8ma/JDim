// ライセンス: GPL2

//#define _DEBUG
//#define _DEBUG_ICONV
#include "jddebug.h"

#include "jdiconv.h"
#include "misccharcode.h"
#include "miscmsg.h"

#include "config/globalconf.h"

#include <errno.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>


using namespace JDLIB;

Iconv::Iconv( const CharCode to, const CharCode from )
    : m_cd( (GIConv)-1 )
    , m_buf_size( BUF_SIZE_ICONV_OUT )
    , m_coding_from( from )
    , m_coding_to( to )
{
#ifdef _DEBUG
    std::cout << "Iconv::Iconv coding = " << m_coding_from << " to " << m_coding_to << std::endl;
#endif
    
    m_buf = ( char* )malloc( m_buf_size );
    const char* from_str = MISC::charcode_to_cstr( ( from == CHARCODE_UNKNOWN ) ? to : from );
    const char* to_str = MISC::charcode_to_cstr( ( to == CHARCODE_UNKNOWN ) ? from : to );
    
    m_cd = g_iconv_open( to_str, from_str );

    // MS932で失敗したらCP932で試してみる
    if( m_cd == ( GIConv ) -1 ){
        if( to == CHARCODE_SJIS ) m_cd = g_iconv_open( "CP932", from_str );
        else if( from == CHARCODE_SJIS ) m_cd = g_iconv_open( to_str, "CP932" );
    }

    // "EUCJP-*"で失敗したら"EUCJP"で試してみる
    if( m_cd == ( GIConv ) - 1 && ( errno & EINVAL ) != 0 )
    {
        if( to == CHARCODE_EUCJP ) m_cd = g_iconv_open( "EUCJP//TRANSLIT", from_str );
        else if( from == CHARCODE_EUCJP )
        {
            const std::string coding_to_translit = std::string( to_str ) + "//TRANSLIT";
            m_cd = g_iconv_open( coding_to_translit.c_str(), "EUCJP" );
        }
    }

    if( m_cd == ( GIConv ) -1 ){
        std::string msg = "can't open iconv coding = ";
        msg += MISC::charcode_to_cstr( from );
        msg += " to ";
        msg += MISC::charcode_to_cstr( to );
        MISC::ERRMSG( msg );
    }
}

Iconv::~Iconv()
{
#ifdef _DEBUG
    std::cout << "Iconv::~Iconv\n";
#endif    
    
    if( m_buf ) free( m_buf );
    if( m_cd != ( GIConv ) -1 ) g_iconv_close( m_cd );
}


const char* Iconv::convert( const char* str_in, int size_in, int& size_out )
{
#ifdef _DEBUG
    std::cout << "Iconv::convert size_in = " << size_in << std::endl;
#endif

    if( m_cd == ( GIConv ) -1 ) return nullptr;
    
    char* buf_in_tmp = const_cast<char*>( str_in );
    const char* buf_in_end = str_in + size_in;

    char* buf_out_tmp = m_buf;
    char* buf_out_end = m_buf + m_buf_size;

    const char* pre_check = nullptr; // 前回チェックしたUTF-8の先頭

    // iconv 実行
    do{

        size_t byte_left_in = buf_in_end - buf_in_tmp;
        size_t byte_left_out = buf_out_end - buf_out_tmp;

#ifdef _DEBUG
        std::cout << "byte_left_in = " << byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif
    
        const int ret = g_iconv( m_cd, &buf_in_tmp, &byte_left_in, &buf_out_tmp, &byte_left_out );

#ifdef _DEBUG
        std::cout << "--> ret = " << ret << std::endl;
        std::cout << "byte_left_in = " << byte_left_in << std::endl;
        std::cout << "byte_left_out = " << byte_left_out << std::endl;
#endif

        //　エラー
        if( ret == -1 ){

            if( errno == EILSEQ ){

#ifdef _DEBUG_ICONV
                char str_tmp[256];
#endif
                const unsigned char code0 = *buf_in_tmp;
#ifdef _DEBUG_ICONV
                const unsigned char code1 = *( buf_in_tmp + 1 );
                const unsigned char code2 = *( buf_in_tmp + 2 );
#endif

                // MS932からUTF-8へマッピング失敗
                if( m_coding_from == CHARCODE_SJIS && m_coding_to == CHARCODE_UTF8
                        && CONFIG::get_broken_sjis_be_utf8() ){

                    // UTF-8へ変換する場合UTF-8の混在を許容する
                    char* bpos = buf_in_tmp;
                    const char* epos = buf_in_tmp;

                    // マルチバイトUTF-8文字列の先頭と終端を調べる
                    while( bpos > str_in && *( bpos - 1 ) < 0 ) --bpos;
                    while( epos < ( buf_in_tmp + byte_left_in ) && *epos < 0 ) ++epos;

                    size_t lng = epos - bpos;

                    // 1byteは不正、また毎回同じ文字列を判定しない
                    if( lng > 1 && bpos != pre_check ){

                        // 一文字ずつUTF-8の文字か確認
                        size_t byte = 0;
                        pre_check = bpos;

                        if( MISC::is_utf8( bpos, lng, byte ) ){
#ifdef _DEBUG_ICONV
                            std::string utf8( bpos, lng );
                            snprintf( str_tmp, 256, "iconv to be utf-8: %s", utf8.c_str() );
                            MISC::MSG( str_tmp );
#endif

                            constexpr const char* span_bgn = "<span class=\"BROKEN_SJIS\">";
                            constexpr const char* span_end = "</span>";
                            const size_t lng_span_bgn = strlen( span_bgn );
                            const size_t lng_span_end = strlen( span_end );

                            while( buf_out_tmp > m_buf && *( buf_out_tmp - 1 ) < 0 ) --buf_out_tmp;
                            if( ( buf_out_tmp + lng + lng_span_bgn + lng_span_end ) >= buf_out_end ){
                                const size_t used = buf_out_tmp - m_buf;
                                if( ! grow() ) break;
                                buf_out_tmp = m_buf + used;
                                buf_out_end = m_buf + m_buf_size;
                            }
                            memcpy( buf_out_tmp, span_bgn, lng_span_bgn );
                            buf_out_tmp += lng_span_bgn;
                            memcpy( buf_out_tmp, bpos, lng );
                            buf_out_tmp += lng;
                            memcpy( buf_out_tmp, span_end, lng_span_end );
                            buf_out_tmp += lng_span_end;
                            buf_in_tmp = bpos + lng;

                            continue;
                        }
                    }
                }

                // unicode 文字からの変換失敗
                // 数値文字参照(&#????;)形式にする
                if( m_coding_from == CHARCODE_UTF8 ){

                    // https://github.com/JDimproved/JDim/issues/214 （emoji subdivision flagの処理）について
                    //
                    // TAG LATIN SMALL LETTER と CANCEL TAG については、libcのiconv()関数にかけると、
                    // エラーを返さず、なおかつこれらの文字を無視してしまうので、
                    // U+1F3F4 WAVING BLACK FLAGが現れた時は、上の範囲のUTF-8文字が続いてないか、
                    // このblockの範囲で確認してから、脱出する。


                    bool is_converted_to_ucs2 = false;  // 数値文字参照に変換されたかどうか
                    bool is_handling_emoji_subdivision_flag = false;  // emoji subdivision flags の処理の途中か

                    for ( ; ; ) {
                        int byte;
                        const char32_t code = MISC::utf8tocp( buf_in_tmp, byte );
                        if( byte <= 1 ) break;

                        // emoji subdivision flags の処理
                        if ( is_handling_emoji_subdivision_flag ) {
                            // Tag Latin Small Letterの範囲か、Cancel Tagでなければ、処理中断
                            if ( byte != 4 ) break;
                            if ( code < 917601 ) break; // U+E0061 TAG LATIN SMALL LETTER A
                            if ( code > 917631 ) break; // U+E007F CANCEL TAG
                        }

                        const std::string ucs_str = std::to_string( code );
#ifdef _DEBUG
                        std::cout << "code = " << ucs_str << " byte = " << byte << std::endl;
#endif
                        buf_in_tmp += byte;

                        if( ( buf_out_tmp + ucs_str.length() + 3 ) >= buf_out_end ){
                            const size_t used = buf_out_tmp - m_buf;
                            if( ! grow() ) break;
                            buf_out_tmp = m_buf + used;
                            buf_out_end = m_buf + m_buf_size;
                        }

                        *(buf_out_tmp++) = '&';
                        *(buf_out_tmp++) = '#';
                        ucs_str.copy( buf_out_tmp, ucs_str.length() );
                        buf_out_tmp += ucs_str.length();
                        *(buf_out_tmp++) = ';';

                        byte_left_out -= ucs_str.size() + 3;
                        is_converted_to_ucs2 = true;  // 一度変換されたのでマーク

                        if ( ! is_handling_emoji_subdivision_flag ) {
                            if ( ( byte == 4 ) && ( code == 127988 ) ){ // U+1F3F4 WAVING BLACK FLAG
                                // emoji subdivision flags の処理開始
                                is_handling_emoji_subdivision_flag = true;
                                continue; // 連続処理
                            }
                        } else {
                            // まだ emoji subdivision flags の処理中
                            continue;
                        }

                        break;
                    }

                    // 数値文字参照に変換された場合は、continueする
                    if ( is_converted_to_ucs2 ) continue;

                }

                // 時々空白(0x20)で EILSEQ が出るときがあるのでもう一度トライする
                if( code0 == 0x20 ) continue;

#ifdef _DEBUG_ICONV
                snprintf( str_tmp, 256, "iconv EILSEQ left = %zu code = %x %x %x", byte_left_in, code0, code1, code2 );
                MISC::ERRMSG( str_tmp );
#endif

                // BOFの確認
                if( ( buf_out_end - buf_out_tmp ) <= 3 ){
                    const size_t used = buf_out_tmp - m_buf;
                    if( ! grow() ) break;
                    buf_out_tmp = m_buf + used;
                    buf_out_end = m_buf + m_buf_size;
                }

                // UTF-8へ変換する場合はREPLACEMENT CHARACTERに置き換える
                if( m_coding_to == CHARCODE_UTF8 ){
                    ++buf_in_tmp;
                    *(buf_out_tmp++) = static_cast< char >( 0xef );
                    *(buf_out_tmp++) = static_cast< char >( 0xbf );
                    *(buf_out_tmp++) = static_cast< char >( 0xbd );
                    continue;
                }

                //その他、1文字を?にして続行
                ++buf_in_tmp;
                *(buf_out_tmp++) = '?';
            }

            else if( errno == EINVAL ){
#ifdef _DEBUG_ICONV
                MISC::ERRMSG( "iconv EINVAL\n" );
#endif
                break;
            }

            else if( errno == E2BIG ){
#ifdef _DEBUG_ICONV
                MISC::ERRMSG( "iconv E2BIG\n" );
#endif
                const size_t used = buf_out_tmp - m_buf;
                if( ! grow() ) break;
                buf_out_tmp = m_buf + used;
                buf_out_end = m_buf + m_buf_size;
                continue;
            }
        }
    
    } while( buf_in_tmp < buf_in_end );

    size_out = buf_out_tmp - m_buf;
    *buf_out_tmp = '\0';
    
#ifdef _DEBUG
    std::cout << "Iconv::convert size_out = " << size_out << std::endl;
#endif
    return m_buf;
}



bool Iconv::grow()
{
    m_buf_size += BUF_SIZE_ICONV_OUT;
    char *tmp = ( char* )realloc( m_buf, m_buf_size );
    if( ! tmp ) return false;
    m_buf = tmp;
    return true;
}
