// ライセンス: GPL2

//#define _DEBUG
#include "jddebug.h"

#include "font.h"

#include "jdlib/miscutil.h"

#include "fontid.h"
#include "config/globalconf.h"

#include <cstdlib>
#include <cstring>


struct WIDTH_DATA
{
    // 半角モードの時の幅
    unsigned int *width;

    // 全角モードの時の幅
    unsigned int width_wide;
};


static WIDTH_DATA* width_of_char[ FONT_NUM ];
static bool strict_of_char = false;


// UnicodeのPlane 0 基本多言語面(BMP)からPlane 3 第三漢字面(TIP)までキャッシュを持つ。
// 現状のメモリ消費を抑えるためPlane 4からPlane 13は将来割り当てられたときにキャッシュ対応する。
constexpr char32_t kMaxCacheCodePoint{ 0x40000 };


//
// 初期化
//
void ARTICLE::init_font()
{
    // スレビューで文字幅の近似を厳密にするか
    strict_of_char = CONFIG::get_strict_char_width();

    for( WIDTH_DATA*& char_data : width_of_char ) {

        if( char_data ) {

            for( size_t j = 0; j < kMaxCacheCodePoint; ++j ){

                if( char_data[ j ].width ) delete[] char_data[ j ].width;
            }
            delete[] char_data;
            char_data = nullptr;
        }
    }
}



//
// 登録された文字の幅を返す関数
//
// code   : 入力文字 (コードポイント)
// pre_char : ひとつ前の文字 ( 前の文字が全角の場合は 0 )
// width  : 半角モードでの幅
// width_wide : 全角モードでの幅
// mode   : fontid.h で定義されているフォントのID
// 戻り値 : 登録されていればtrue
// 
bool ARTICLE::get_width_of_char( const char32_t code, const char pre_char, int& width, int& width_wide, const int mode )
{
    width = 0;
    width_wide = 0;

    if( ! width_of_char[ mode ] ){
        width_of_char[ mode ] = new WIDTH_DATA[ kMaxCacheCodePoint ]{};

        // 合成文字の初期化
        for( int i = 0x300; i <= 0x36f; i++ ) // Combining Diacritical Marks
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0x180b; i <= 0x180d; i++ ) // Mongolian Free Variation Selector
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0x200b; i <= 0x200f; i++ ) // ZWSP,ZWNJ,ZWJ,LRM,RLM
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0x202a; i <= 0x202e; i++ ) // LRE,RLE,PDF,LRO,RLO
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0x20d0; i <= 0x20ff; i++ ) // Combining Diacritical Marks for Symbols
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0x3099; i <= 0x309a; i++ ) // COMBINING KATAKANA-HIRAGANA (SEMI-)VOICED SOUND MARK
            width_of_char[ mode ][ i ].width_wide = -1;
        for( int i = 0xfe00; i <= 0xfe0f; i++ ) // VS1-VS16
            width_of_char[ mode ][ i ].width_wide = -1;
        width_of_char[ mode ][ 0xfeff ].width_wide = -1; // ZERO WIDTH NO-BREAK SPACE
    }

    if( code > 0 && code < kMaxCacheCodePoint ){

        // 全角モードの幅
        width_wide = width_of_char[ mode ][ code ].width_wide;

        // 半角モードの幅
        width = width_wide;

        // 厳密に求める場合
        if( code < 128 && strict_of_char ){

            if( ! width_of_char[ mode ][ code ].width ){
                width_of_char[ mode ][ code ].width = new unsigned int[ 128 ]{};
            }

            const int pre_char_num = ( int ) pre_char;
            if( pre_char_num < 128 ) width = width_of_char[ mode ][ code ].width[ pre_char_num ];
        }
    }
    // Plane 14 追加特殊用途面(SSP)
    // 制御コードが追加されたら条件を追加する
    else if( 0xE0001 == code || ( 0xE0020 <= code && code <= 0xE007F ) // タグ文字
             || ( 0xE0100 <= code && code <= 0xE01EF )                // 異字体セレクタ
    ) {
        width = width_wide = 0;
        return true;
    }

    if( width == -1 ){ // フォント幅の取得に失敗した場合
        width = width_wide = 0;
        return true;
    }
    else if( width && width_wide ) return true;

    return false;
}



//
// 文字幅を登録する関数
//
// width == -1 はフォント幅の取得に失敗した場合
//
void ARTICLE::set_width_of_char( const char* utfstr, int& byte, const char pre_char, const int width, const int width_wide, const int mode )
{    
    const int c32 = MISC::utf8toucs2( utfstr, byte );
    if( ! byte ) return;
    if( c32 >= kMaxCacheCodePoint ) return;

    // 半角モードの幅を厳密に求める場合
    if( byte == 1 && strict_of_char ){

        const int pre_char_num = ( int ) pre_char;
        if( pre_char_num < 128 ) width_of_char[ mode ][ c32 ].width[ pre_char_num ] = width;
    }

    // 全角モードの幅
    width_of_char[ mode ][ c32 ].width_wide = width_wide;
}
