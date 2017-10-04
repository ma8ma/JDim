// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "spchar_decoder.h"
#include "spchar_tbl.h"
#include "node.h"

#include "jdlib/miscutil.h"
#include "config/globalconf.h"

#include <string.h>
#include <stdlib.h>


bool check_spchar( const char* n_in, const char* spchar )
{
    int i = 0;
    while( spchar[ i ] != '\0' ){

        if( n_in[ i ] != spchar[ i ] ) return false;
        ++i;
    }

    return true;
}


//
// ユニコード文字参照  &#数字;
//
// in_char: 入力文字列、in_char[1] == "#" であること
// n_in : 入力で使用した文字数が返る
// out_char : 出力文字列
// n_out : 出力した文字数が返る
// only_check : チェックのみ実施 ( out_char は nullptr でも可 )
//
// 戻り値 : node.h で定義したノード番号
//
int decode_char_number( const char* in_char, int& n_in,  char* out_char, int& n_out, const bool only_check )
{
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    int offset;
    const int lng = MISC::spchar_number_ln( in_char, offset );
    if( lng == -1 ) return DBTREE::NODE_NONE;

    if( only_check ) return ret;

    int num = MISC::decode_spchar_number( in_char, offset, lng );

    // 特殊条件を処理
    if( num >= 0x80 && num <= 0x9F ) num = charref_tbl[ num - 0x80 ];
    else if( num == 0 ||  num > 0x10FFFF ) num = UCS_REPLACE;
    else if( num >= 0xD800 && num < 0xE000 ){

        // 間違ったエンコードで壊れた追加面の文字を修復する
        if( !CONFIG::get_correct_character_reference() ) num = UCS_REPLACE;
        else {
            const char *char_low = in_char + offset + lng + 1;
            int offset_low;
            int lng_low = MISC::spchar_number_ln( char_low, offset_low );
            if( lng_low == -1 ) num = UCS_REPLACE;
            else{
                const int num_low = MISC::decode_spchar_number( char_low, offset_low, lng_low );
                if( num < 0xDC01 && num_low >= 0xDC00 && num_low < 0xE000 ){
                    num = 0x10000 + ( num - 0xD800 ) * 0x400 + ( num_low - 0xDC00 );
                    offset += 1 + offset_low + lng_low;
                }
                else num = UCS_REPLACE;
            }
        }
    }

    switch( num ){

        //lfはspにする
        case UCS_SP:
        case UCS_LF:
            ret = DBTREE::NODE_SP;
            break;

        case UCS_HT:
            ret = DBTREE::NODE_HTAB;
            break;

        //zwnj,zwj,lrm,rlm,lre,rle,lro.rlo は今のところ無視
        case UCS_ZWSP:
//        case UCS_ZWNJ:
//        case UCS_ZWJ:
//        case UCS_LRM:
//        case UCS_RLM:
        case UCS_CR: // CRを無視
        case UCS_FF: // FFを無視
        case UCS_LS: // LSを無視
        case UCS_PS: // PSを無視
            ret = DBTREE::NODE_ZWSP;
            break;

        default:
            n_out = g_unichar_to_utf8( static_cast<char32_t>( num ), out_char );
            if( ! n_out ) return DBTREE::NODE_NONE;
    }

    n_in = offset + lng;
    if( in_char[n_in] == ';' ) n_in++; // 数値文字参照の終端「;」の場合は1文字削除
    
    if( out_char ) out_char[ n_out ] = '\0';

    return ret;
}    


//
// 文字参照のデコード
//
// in_char : 入力文字列, in_char[ 0 ] = '&' となっていること
// n_in : 入力で使用した文字数が返る
// out_char : 出力文字列
// n_out : 出力した文字数が返る
// only_check : チェックのみ実施 ( out_char は nullptr でも可 )
//
// 戻り値 : node.h で定義したノード番号
//
int DBTREE::decode_char( const char* in_char, int& n_in,  char* out_char, int& n_out, const bool only_check )
{
    // 1文字目が&以外の場合は出力しない
    if( in_char[ 0 ] != '&' ){
        n_in = n_out = 0;
        if( out_char ) out_char[ n_out ] = '\0';

        return DBTREE::NODE_NONE;
    }

    // 数字参照 &#数字;
    if( in_char[ 1 ] == '#' ) return decode_char_number( in_char, n_in, out_char, n_out, only_check );

    // 文字参照 -> ユニコード変換
    int ret = DBTREE::NODE_TEXT;
    n_in = n_out = 0;

    const char ch = in_char[ 1 ];
    UCSTBL const *tbl;

    if( ch >= 'a' && ch <= 'z' ) tbl = ucstbl_small[ ch - 'a' ];
    else if( ch >= 'A' && ch <= 'Z' ) tbl = ucstbl_large[ ch - 'A' ];
    else return DBTREE::NODE_NONE;

    for( int ucs = tbl->ucs; ucs != 0; ucs = ( ++tbl )->ucs ){
        if( check_spchar( in_char + 1, tbl->str ) ){

            if( only_check ) return ret;

            n_in = strlen( tbl->str ) + 1; // 先頭の '&' の分を+1する

            if( ucs == UCS_ZWSP ) ret = DBTREE::NODE_ZWSP;
            else n_out = g_unichar_to_utf8( static_cast<char32_t>( ucs ), out_char );

            break;
        }
    }

    if( !n_in ) ret = DBTREE::NODE_NONE;
    if( out_char ) out_char[ n_out ] = '\0';

    return ret;
}
