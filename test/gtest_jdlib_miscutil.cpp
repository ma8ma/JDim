// License: GPL2

#include "jdlib/miscutil.h"

#include "gtest/gtest.h"


namespace {

class SplitLineTest : public ::testing::Test {};

TEST_F(SplitLineTest, split_empty)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"" ) );
}

TEST_F(SplitLineTest, split_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"    " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8" the quick  brown   fox  " ) );
}

TEST_F(SplitLineTest, split_U_3000)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000 \u3000 " ) );

    expect.assign( { u8"the", u8"quick", u8"brown", u8"fox" } );
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000the\u3000quick \u3000brown\u3000 \u3000fox\u3000 " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( u8"  \"\"  " ) );

    expect.assign( { u8"the quick", u8" ", u8" brown   fox " } );
    EXPECT_EQ( expect, MISC::split_line( u8" \"the quick\" \" \" \" brown   fox \" " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_3000)
{
    std::list< std::string > expect = { u8"the\u3000quick", u8"\u3000", u8"\u3000brown \u3000fox\u3000" };
    EXPECT_EQ( expect, MISC::split_line( u8"\u3000\"the\u3000quick\" \"\u3000\" \"\u3000brown \u3000fox\u3000\"" ) );
}


class RemoveSpaceTest : public ::testing::Test {};

TEST_F(RemoveSpaceTest, remove_empty)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"" ) );
}

TEST_F(RemoveSpaceTest, remove_U_0020)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"    " ) );

    expect.assign( u8"the quick  brown   fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8" the quick  brown   fox  " ) );
}

TEST_F(RemoveSpaceTest, remove_U_3000)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \u3000 " ) );

    expect.assign( u8"the quick\u3000brown\u3000 fox" );
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000the quick\u3000brown\u3000 fox\u3000 " ) );
}

TEST_F(RemoveSpaceTest, remove_doublequote)
{
    std::string expect = u8"\"\"";
    EXPECT_EQ( expect, MISC::remove_space( u8"\u3000 \"\"\u3000 " ) );
}


class IsUrlSchemeTest : public ::testing::Test {};

TEST_F(IsUrlSchemeTest, url_none)
{
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "foo" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "http:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ttp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "tp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ftp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "sssp:/" ) );
}

TEST_F(IsUrlSchemeTest, url_http)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "http://foobar", &length ) );
    EXPECT_EQ( 7, length );
}

TEST_F(IsUrlSchemeTest, url_ttp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TTP, MISC::is_url_scheme( "ttp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_tp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TP, MISC::is_url_scheme( "tp://foobar", &length ) );
    EXPECT_EQ( 5, length );
}

TEST_F(IsUrlSchemeTest, url_ftp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_FTP, MISC::is_url_scheme( "ftp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_sssp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://foobar", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_SSSP, MISC::is_url_scheme( "sssp://img.2ch", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://img.5ch", &length ) );
    EXPECT_EQ( 7, length );
}


class MISC_DecodeSpcharNumberTest : public ::testing::Test {};

TEST_F(MISC_DecodeSpcharNumberTest, result_ok)
{
    EXPECT_EQ( 0xC, MISC::decode_spchar_number( "&#xC;", 3, 4 ) );
    EXPECT_EQ( 32, MISC::decode_spchar_number( "&#32;", 2, 4 ) );
    EXPECT_EQ( 0x20, MISC::decode_spchar_number( "&#x20;", 3, 5 ) );
    EXPECT_EQ( 0xA0, MISC::decode_spchar_number( "&#xA0;", 3, 5 ) );
    EXPECT_EQ( 1234, MISC::decode_spchar_number( "&#1234;", 2, 6 ) );
    EXPECT_EQ( 0x1234, MISC::decode_spchar_number( "&#x1234;", 3, 7 ) );

    EXPECT_EQ( 0xD7FF, MISC::decode_spchar_number( "&#xD7FF;", 3, 7 ) );
    EXPECT_EQ( 0xE000, MISC::decode_spchar_number( "&#xE000;", 3, 7 ) );
    EXPECT_EQ( 0xFDCF, MISC::decode_spchar_number( "&#xFDCF;", 3, 7 ) );
    EXPECT_EQ( 0xFDF0, MISC::decode_spchar_number( "&#xFDF0;", 3, 7 ) );

    EXPECT_EQ( 1114109, MISC::decode_spchar_number( "&#1114109;", 2, 9 ) );
    EXPECT_EQ( 0x10FFFD, MISC::decode_spchar_number( "&#x10FFFD;", 3, 9 ) );
    EXPECT_EQ( 0x0abcde, MISC::decode_spchar_number( "&#xabcde;", 3, 8 ) );
}

TEST_F(MISC_DecodeSpcharNumberTest, result_error)
{
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#0;", 2, 3 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#8;", 2, 3 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xB;", 3, 4 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xD;", 3, 4 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#31;", 2, 4 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xD800;", 3, 7 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xDFFF;", 3, 7 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xFDD0;", 3, 7 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#xFDEF;", 3, 7 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x110000;", 3, 9 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#1114112;", 2, 9 ) );
}

TEST_F(MISC_DecodeSpcharNumberTest, result_transform)
{
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x7F;", 3, 5 ) );
    EXPECT_EQ( 0x20AC, MISC::decode_spchar_number( "&#x80;", 3, 5 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x81;", 3, 5 ) );
    EXPECT_EQ( 0x201A, MISC::decode_spchar_number( "&#x82;", 3, 5 ) );
    EXPECT_EQ( 0x0192, MISC::decode_spchar_number( "&#x83;", 3, 5 ) );
    EXPECT_EQ( 0x201E, MISC::decode_spchar_number( "&#x84;", 3, 5 ) );
    EXPECT_EQ( 0x2026, MISC::decode_spchar_number( "&#x85;", 3, 5 ) );
    EXPECT_EQ( 0x2020, MISC::decode_spchar_number( "&#x86;", 3, 5 ) );
    EXPECT_EQ( 0x2021, MISC::decode_spchar_number( "&#x87;", 3, 5 ) );
    EXPECT_EQ( 0x02C6, MISC::decode_spchar_number( "&#x88;", 3, 5 ) );
    EXPECT_EQ( 0x2030, MISC::decode_spchar_number( "&#x89;", 3, 5 ) );
    EXPECT_EQ( 0x0160, MISC::decode_spchar_number( "&#x8A;", 3, 5 ) );
    EXPECT_EQ( 0x2039, MISC::decode_spchar_number( "&#x8B;", 3, 5 ) );
    EXPECT_EQ( 0x0152, MISC::decode_spchar_number( "&#x8C;", 3, 5 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x8D;", 3, 5 ) );
    EXPECT_EQ( 0x017D, MISC::decode_spchar_number( "&#x8E;", 3, 5 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x8F;", 3, 5 ) );

    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x90;", 3, 5 ) );
    EXPECT_EQ( 0x2018, MISC::decode_spchar_number( "&#x91;", 3, 5 ) );
    EXPECT_EQ( 0x2019, MISC::decode_spchar_number( "&#x92;", 3, 5 ) );
    EXPECT_EQ( 0x201C, MISC::decode_spchar_number( "&#x93;", 3, 5 ) );
    EXPECT_EQ( 0x201D, MISC::decode_spchar_number( "&#x94;", 3, 5 ) );
    EXPECT_EQ( 0x2022, MISC::decode_spchar_number( "&#x95;", 3, 5 ) );
    EXPECT_EQ( 0x2013, MISC::decode_spchar_number( "&#x96;", 3, 5 ) );
    EXPECT_EQ( 0x2014, MISC::decode_spchar_number( "&#x97;", 3, 5 ) );
    EXPECT_EQ( 0x02DC, MISC::decode_spchar_number( "&#x98;", 3, 5 ) );
    EXPECT_EQ( 0x2122, MISC::decode_spchar_number( "&#x99;", 3, 5 ) );
    EXPECT_EQ( 0x0161, MISC::decode_spchar_number( "&#x9A;", 3, 5 ) );
    EXPECT_EQ( 0x203A, MISC::decode_spchar_number( "&#x9B;", 3, 5 ) );
    EXPECT_EQ( 0x0153, MISC::decode_spchar_number( "&#x9C;", 3, 5 ) );
    EXPECT_EQ( 0xFFFD, MISC::decode_spchar_number( "&#x9D;", 3, 5 ) );
    EXPECT_EQ( 0x017E, MISC::decode_spchar_number( "&#x9E;", 3, 5 ) );
    EXPECT_EQ( 0x0178, MISC::decode_spchar_number( "&#x9F;", 3, 5 ) );
}


class MISC_AscTest : public ::testing::Test {};

TEST_F(MISC_AscTest, hankaku_alpha_num)
{
    std::string out;
    std::vector< int > table;

    MISC::asc( u8"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", out, table );
    EXPECT_EQ( u8"THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }

    out.clear();
    table.clear();
    MISC::asc( u8"the quick brown fox jumps over the lazy dog.", out, table );
    EXPECT_EQ( u8"the quick brown fox jumps over the lazy dog.", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }

    out.clear();
    table.clear();
    MISC::asc( u8"1234567890+-*/", out, table );
    EXPECT_EQ( u8"1234567890+-*/", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }

    out.resize( 3 );
    table.resize( 3 );
    MISC::asc( u8"hello", out, table );
    EXPECT_EQ( u8"123hello", out );
    EXPECT_EQ( out.size(), table.size() );
    const std::vector< int > expected = { 0, 1, 2, 0, 1, 2, 3, 4 };
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( table[ i ], expected[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_alpha_num)
{
    std::string out;
    std::vector< int > table;

    // 和字間隔(U+3000)は半角スペースに変換されない
    MISC::asc( u8"ＴＨＥ　ＱＵＩＣＫ　ＢＲＯＷＮ　ＦＯＸ　ＪＵＭＰＳ　ＯＶＥＲ　ＴＨＥ　ＬＡＺＹ　ＤＯＧ．", out,
               table );
    EXPECT_EQ( u8"THE　QUICK　BROWN　FOX　JUMPS　OVER　THE　LAZY　DOG．", out );
    EXPECT_EQ( out.size(), table.size() );
    EXPECT_EQ( 33, table.at( 15 ) );
    EXPECT_EQ( 131, table.at( 61 ) );

    out.clear();
    table.clear();
    MISC::asc( u8"ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ．", out,
               table );
    EXPECT_EQ( u8"the　quick　brown　fox　jumps　over　the　lazy　dog．", out );
    EXPECT_EQ( out.size(), table.size() );
    EXPECT_EQ( 33, table.at( 15 ) );
    EXPECT_EQ( 131, table.at( 61 ) );

    // 全角数字は半角に変換されるが、記号は半角に変換されない
    out.clear();
    table.clear();
    MISC::asc( u8"１２３４５６７８９０＋−＊／", out, table );
    EXPECT_EQ( u8"1234567890＋−＊／", out );
    EXPECT_EQ( out.size(), table.size() );
    EXPECT_EQ( 30, table.at( 10 ) );
    EXPECT_EQ( 41, table.at( 21 ) );
}

TEST_F(MISC_AscTest, hankaku_katakana_without_dakuten)
{
    std::string out;
    std::vector< int > table;

    // 半角から全角へ一対一の変換
    const char* hankaku = (
        u8"\uFF61\uFF62\uFF63\uFF64\uFF65" u8"\uFF66" u8"\uFF67\uFF68\uFF69\uFF6A\uFF6B"
        u8"\uFF6C\uFF6D\uFF6E\uFF6F\uFF70" u8"\uFF71\uFF72\uFF73\uFF74\uFF75" u8"\uFF76\uFF77\uFF78\uFF79\uFF7A"
        u8"\uFF7B\uFF7C\uFF7D\uFF7E\uFF7F" u8"\uFF80\uFF81\uFF82\uFF83\uFF84" u8"\uFF85\uFF86\uFF87\uFF88\uFF89"
        u8"\uFF8A\uFF8B\uFF8C\uFF8D\uFF8E" u8"\uFF8F\uFF90\uFF91\uFF92\uFF93" u8"\uFF94\uFF95\uFF96"
        u8"\uFF97\uFF98\uFF99\uFF9A\uFF9B" u8"\uFF9C\uFF9D"
    );
    const char* zenkaku = (
        u8"。「」、・" u8"ヲ" u8"ァィゥェォ"
        u8"ャュョッー" u8"アイウエオ" u8"カキクケコ"
        u8"サシスセソ" u8"タチツテト" u8"ナニヌネノ"
        u8"ハヒフヘホ" u8"マミムメモ" u8"ヤユヨ"
        u8"ラリルレロ" u8"ワン"
    );
    MISC::asc( hankaku, out, table );
    EXPECT_EQ( zenkaku, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }

    // 半角の濁点と半濁点は単独では全角に変換されない
    out.clear();
    table.clear();
    MISC::asc( u8"\uFF9E\uFF9F", out, table );
    EXPECT_EQ( u8"\uFF9E\uFF9F", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, hankaku_katakana_with_dakuten)
{
    std::string out;
    std::vector< int > table, expect_table;

    // 単一のコードポイントで表現可能な(半)濁点付き半角カタカナは合成される
    const char* hankaku = (
        u8"\uFF76\uFF9E\uFF77\uFF9E\uFF78\uFF9E\uFF79\uFF9E\uFF7A\uFF9E"
        u8"\uFF7B\uFF9E\uFF7C\uFF9E\uFF7D\uFF9E\uFF7E\uFF9E\uFF7F\uFF9E"
        u8"\uFF80\uFF9E\uFF81\uFF9E\uFF82\uFF9E\uFF83\uFF9E\uFF84\uFF9E"
        u8"\uFF8A\uFF9E\uFF8B\uFF9E\uFF8C\uFF9E\uFF8D\uFF9E\uFF8E\uFF9E"
        u8"\uFF8A\uFF9F\uFF8B\uFF9F\uFF8C\uFF9F\uFF8D\uFF9F\uFF8E\uFF9F"
    );
    const char* zenkaku = (
        u8"\u30AC\u30AE\u30B0\u30B2\u30B4" // ガギグゲゴ
        u8"\u30B6\u30B8\u30BA\u30BC\u30BE" // ザジズゼゾ
        u8"\u30C0\u30C2\u30C5\u30C7\u30C9" // ダヂヅデド
        u8"\u30D0\u30D3\u30D6\u30D9\u30DC" // バビブベボ
        u8"\u30D1\u30D4\u30D7\u30DA\u30DD" // パピプペポ
    );
    MISC::asc( hankaku, out, table );
    EXPECT_EQ( zenkaku, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, j = 0, size = out.size(); i < size; i += 3, j += 6 ) {
        EXPECT_EQ( j, table[ i ] );
        EXPECT_EQ( j + 1, table[ i + 1 ] );
        EXPECT_EQ( j + 2, table[ i + 2 ] );
    }

    // ワ行の濁点付き半角カタカナは全角に変換されない(変換表が未実装) : ヴヷヺ
    out.clear();
    table.clear();
    MISC::asc( u8"\uFF73\uFF9E\uFF9C\uFF9E\uFF66\uFF9E", out, table );
    EXPECT_EQ( u8"\u30F4\uFF9C\uFF9E\uFF66\uFF9E", out );
    EXPECT_EQ( out.size(), table.size() );
    expect_table.assign( { 0, 1, 2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 } );
    EXPECT_EQ( expect_table, table );

    // 単一のコードポイントで表現できない(半)濁点付き半角カタカナは全角に変換されない : カ゚キ゚ク゚ケ゚コ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    out.clear();
    table.clear();
    MISC::asc( u8"\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", out, table );
    EXPECT_EQ( u8"\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_katakana_without_dakuten)
{
    std::string out;
    std::vector< int > table;

    // (半)濁点の付いていない全角カタカナ
    const char* without_dakuten = (
        u8"\u30A1\u30A2\u30A3\u30A4\u30A5\u30A6\u30A7\u30A8\u30A9\u30AA" // ァ - オ
        u8"\u30AB\u30AD\u30AF\u30B1\u30B3\u30B5\u30B7\u30B9\u30BB\u30BD" // カ - ソ
        u8"\u30BF\u30C1\u30C3\u30C4\u30C6\u30C8\u30CA\u30CB\u30CC\u30CD\u30CE" // タ - ノ
        u8"\u30CF\u30D2\u30D5\u30D8\u30DB\u30DE\u30DF\u30E0\u30E1\u30E2" // ハ - モ
        u8"\u30E3\u30E4\u30E5\u30E6\u30E7\u30E8\u30E9\u30EA\u30EB\u30EC\u30ED" // ャ - ロ
        u8"\u30EE\u30EF\u30F0\u30F1\u30F2\u30F3\u30F5\u30F6" // ヮ - ヶ
    );
    MISC::asc( without_dakuten, out, table );
    EXPECT_EQ( without_dakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_katakana_with_dakuten)
{
    // TODO: Unicodeの正規化を参考に濁点、半濁点を分解する
    std::string out;
    std::vector< int > table;

    // 濁点付き全角カタカナ
    const char* with_dakuten = (
        u8"\u30AC\u30AE\u30B0\u30B2\u30B4\u30B6\u30B8\u30BA\u30BC\u30BE" // ガ - ゾ
        u8"\u30C0\u30C2\u30C5\u30C7\u30C9\u30D0\u30D3\u30D6\u30D9\u30DC" // ダ - ボ
        u8"\u30F4\u30F7\u30F8\u30F9\u30FA" // ヴ - ヺ
    );
    MISC::asc( with_dakuten, out, table );
    EXPECT_EQ( with_dakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_katakana_with_handakuten)
{
    // TODO: Unicodeの正規化を参考に濁点、半濁点を分解する
    std::string out;
    std::vector< int > table;

    // 半濁点付き全角カタカナ
    const char* with_handakuten = u8"\u30D1\u30D4\u30D7\u30DA0x30DD"; // パ - ポ
    MISC::asc( with_handakuten, out, table );
    EXPECT_EQ( with_handakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_katakana_precomposed)
{
    std::string out;
    std::vector< int > table, expect_table;

    // 合成済み文字 (半)濁点付き全角カタカナ : カ゚キ゚ク゚ケ゚コ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    out.clear();
    table.clear();
    MISC::asc( u8"\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A", out, table );
    EXPECT_EQ( u8"\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_hiragana_without_dakuten)
{
    std::string out;
    std::vector< int > table;

    // (半)濁点の付いていない全角ひらがな
    const char* without_dakuten = (
        u8"\u3041\u3042\u3043\u3044\u3045\u3046\u3047\u3048\u3049\u304A" // ぁ - お
        u8"\u304B\u304D\u304F\u3051\u3053\u3055\u3057\u3059\u305B\u305D" // か - そ
        u8"\u305F\u3061\u3063\u3064\u3066\u3068\u306A\u306B\u306C\u306D\u306E" // た - の
        u8"\u306F\u3072\u3075\u3078\u307B\u307E\u307F\u3080\u3081\u3082" // は - も
        u8"\u3083\u3084\u3085\u3086\u3087\u3088\u3089\u308A\u308B\u308C\u308D" // ゃ - ろ
        u8"\u308E\u308F\u3090\u3091\u3092\u3093\u3095\u3096" // ゎ - ゖ
    );
    MISC::asc( without_dakuten, out, table );
    EXPECT_EQ( without_dakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_hiragana_with_dakuten)
{
    // TODO: Unicodeの正規化を参考に濁点、半濁点を分解する
    std::string out;
    std::vector< int > table;

    // 濁点付き全角ひらがな
    const char* with_dakuten = (
        u8"\u304C\u304E\u3050\u3052\u3054\u3056\u3058\u305A\u305C\u305E" // が - ぞ
        u8"\u3060\u3062\u3065\u3067\u3069\u3070\u3073\u3076\u3079\u307C" // だ - ぼ
        u8"\u3094" // ゔ
    );
    MISC::asc( with_dakuten, out, table );
    EXPECT_EQ( with_dakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_hiragana_with_handakuten)
{
    // TODO: Unicodeの正規化を参考に濁点、半濁点を分解する
    std::string out;
    std::vector< int > table;

    // 半濁点付き全角ひらがな
    const char* with_handakuten = u8"\u3071\u3074\u3077\u307A0x307D"; // パ - ポ
    MISC::asc( with_handakuten, out, table );
    EXPECT_EQ( with_handakuten, out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

TEST_F(MISC_AscTest, zenkaku_hiragana_precomposed)
{
    std::string out;
    std::vector< int > table, expect_table;

    // 合成済み文字 (半)濁点付き全角ひらがな : か゚き゚く゚け゚こ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    out.clear();
    table.clear();
    MISC::asc( u8"\u304B\u309A\u304D\u309A\u304F\u309A\u3051\u309A\u3053\u309A", out, table );
    EXPECT_EQ( u8"\u304B\u309A\u304D\u309A\u304F\u309A\u3051\u309A\u3053\u309A", out );
    EXPECT_EQ( out.size(), table.size() );
    for( int i = 0, size = out.size(); i < size; ++i ) {
        EXPECT_EQ( i, table[ i ] );
    }
}

} // namespace
