// License: GPL2

#include "jdlib/miscutil.h"

#include "gtest/gtest.h"


namespace {

class SplitLineTest : public ::testing::Test {};

TEST_F(SplitLineTest, split_empty)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( "" ) );
}

TEST_F(SplitLineTest, split_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( "    " ) );

    expect.assign( { "the", "quick", "brown", "fox" } );
    EXPECT_EQ( expect, MISC::split_line( " the quick  brown   fox  " ) );
}

TEST_F(SplitLineTest, split_U_3000)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( "\u3000 \u3000 " ) );

    expect.assign( { "the", "quick", "brown", "fox" } );
    EXPECT_EQ( expect, MISC::split_line( "\u3000the\u3000quick \u3000brown\u3000 \u3000fox\u3000 " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_0020)
{
    std::list< std::string > expect = {};
    EXPECT_EQ( expect, MISC::split_line( "  \"\"  " ) );

    expect.assign( { "the quick", " ", " brown   fox " } );
    EXPECT_EQ( expect, MISC::split_line( " \"the quick\" \" \" \" brown   fox \" " ) );
}

TEST_F(SplitLineTest, split_doublequote_U_3000)
{
    std::list< std::string > expect = { "the\u3000quick", "\u3000", "\u3000brown \u3000fox\u3000" };
    EXPECT_EQ( expect, MISC::split_line( "\u3000\"the\u3000quick\" \"\u3000\" \"\u3000brown \u3000fox\u3000\"" ) );
}


class RemoveSpaceTest : public ::testing::Test {};

TEST_F(RemoveSpaceTest, remove_empty)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( "" ) );
}

TEST_F(RemoveSpaceTest, remove_U_0020)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( "    " ) );

    expect.assign( "the quick  brown   fox" );
    EXPECT_EQ( expect, MISC::remove_space( " the quick  brown   fox  " ) );
}

TEST_F(RemoveSpaceTest, remove_U_3000)
{
    std::string expect = {};
    EXPECT_EQ( expect, MISC::remove_space( "\u3000 \u3000 " ) );

    expect.assign( "the quick\u3000brown\u3000 fox" );
    EXPECT_EQ( expect, MISC::remove_space( "\u3000the quick\u3000brown\u3000 fox\u3000 " ) );
}

TEST_F(RemoveSpaceTest, remove_doublequote)
{
    std::string expect = "\"\"";
    EXPECT_EQ( expect, MISC::remove_space( "\u3000 \"\"\u3000 " ) );
}


class IsUrlSchemeTest : public ::testing::Test {};

TEST_F(IsUrlSchemeTest, url_none)
{
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "foo" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "http:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ttp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "tp:/" ) );
    EXPECT_EQ( MISC::SCHEME_NONE, MISC::is_url_scheme( "ftp:/" ) );
    // "sssp:/" はバッファオーバーランを起こす
}

TEST_F(IsUrlSchemeTest, url_http)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "http://", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "http://foobar", &length ) );
    EXPECT_EQ( 7, length );
}

TEST_F(IsUrlSchemeTest, url_ttp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TTP, MISC::is_url_scheme( "ttp://", &length ) );
    EXPECT_EQ( 6, length );

    EXPECT_EQ( MISC::SCHEME_TTP, MISC::is_url_scheme( "ttp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_tp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_TP, MISC::is_url_scheme( "tp://", &length ) );
    EXPECT_EQ( 5, length );

    EXPECT_EQ( MISC::SCHEME_TP, MISC::is_url_scheme( "tp://foobar", &length ) );
    EXPECT_EQ( 5, length );
}

TEST_F(IsUrlSchemeTest, url_ftp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_FTP, MISC::is_url_scheme( "ftp://", &length ) );
    EXPECT_EQ( 6, length );

    EXPECT_EQ( MISC::SCHEME_FTP, MISC::is_url_scheme( "ftp://foobar", &length ) );
    EXPECT_EQ( 6, length );
}

TEST_F(IsUrlSchemeTest, url_sssp)
{
    int length;
    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_HTTP, MISC::is_url_scheme( "sssp://foobar", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_SSSP, MISC::is_url_scheme( "sssp://img.2ch", &length ) );
    EXPECT_EQ( 7, length );

    EXPECT_EQ( MISC::SCHEME_SSSP, MISC::is_url_scheme( "sssp://img.5ch", &length ) );
    EXPECT_EQ( 7, length );
}



class MISC_AscTest : public ::testing::Test {};

TEST_F(MISC_AscTest, empty_input)
{
    std::string output;
    std::vector<int> table;

    // 入力はヌル終端文字列
    MISC::asc( "", output, table );

    EXPECT_EQ( "", output );
    EXPECT_EQ( 0, output.size() );
    // 文字列の終端（ヌル文字）の位置が追加されるためtableのサイズが+1大きくなる
    EXPECT_EQ( 1, table.size() );
    EXPECT_EQ( 0, table.at( 0 ) );
}

TEST_F(MISC_AscTest, halfwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output, table );

    EXPECT_EQ( "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "the quick brown fox jumps over the lazy dog.", output, table );

    EXPECT_EQ( "the quick brown fox jumps over the lazy dog.", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "1234567890+-*/", output, table );

    EXPECT_EQ( "1234567890+-*/", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_append_data)
{
    std::string output = "123";
    std::vector<int> table = { 0, 1, 2 };

    // アウトプット引数は初期化せずデータを追加する
    MISC::asc( "hello", output, table );

    EXPECT_EQ( "123hello", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = { 0, 1, 2, 0, 1, 2, 3, 4, 5 };
    EXPECT_EQ( expected_table, table );
}


std::vector<int> expected_table_fullwidth_quick_brown_fox()
{
    // 全角英数字から半角英数字に変換したときの文字列の位置を保存しておくテーブルのテストデータ
    return {
        // TH E  U+3000     Q   U   I   C   K   U+3000      B   R   O   W   N   U+3000      F   O   X   U+3000
        0, 3, 6, 9, 10, 11, 12, 15, 18, 21, 24, 27, 28, 29, 30, 33, 36, 39, 42, 45, 46, 47, 48, 51, 54, 57, 58, 59,
        // JU   M   P   S   U+3000      O   V   E   R   U+3000      T   H   E   U+3000         L    A    Z    Y
        60, 63, 66, 69, 72, 75, 76, 77, 78, 81, 84, 87, 90, 91, 92, 93, 96, 99, 102, 103, 104, 105, 108, 111, 114,
        // U+3000      D    O    G    U+FF0E         U+0000
        117, 118, 119, 120, 123, 126, 129, 130, 131, 132,
    };
}

TEST_F(MISC_AscTest, fullwidth_latin_capital_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "ＴＨＥ　ＱＵＩＣＫ　ＢＲＯＷＮ　ＦＯＸ　ＪＵＭＰＳ　ＯＶＥＲ　ＴＨＥ　ＬＡＺＹ　ＤＯＧ．", output,
               table );

    // 和字間隔(U+3000)は半角スペースに変換されない
    EXPECT_EQ( "THE　QUICK　BROWN　FOX　JUMPS　OVER　THE　LAZY　DOG．", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_AscTest, fullwidth_latin_small_letter)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "ｔｈｅ　ｑｕｉｃｋ　ｂｒｏｗｎ　ｆｏｘ　ｊｕｍｐｓ　ｏｖｅｒ　ｔｈｅ　ｌａｚｙ　ｄｏｇ．", output,
               table );

    EXPECT_EQ( "the　quick　brown　fox　jumps　over　the　lazy　dog．", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    EXPECT_EQ( expected_table_fullwidth_quick_brown_fox(), table );
}

TEST_F(MISC_AscTest, fullwidth_digit_sign)
{
    std::string output;
    std::vector<int> table;

    MISC::asc( "１２３４５６７８９０＋−＊／", output, table );

    // 全角数字は半角に変換されるが、全角記号は半角に変換されない
    EXPECT_EQ( "1234567890＋−＊／", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
        30, 31, 32,  33, 34, 35,  36, 37, 38,  39, 40, 41,  42
    };
    EXPECT_EQ( expected_table, table );
}


TEST_F(MISC_AscTest, halfwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] {
        "\uFF61\uFF62\uFF63\uFF64\uFF65" "\uFF66" "\uFF67\uFF68\uFF69\uFF6A\uFF6B"
        "\uFF6C\uFF6D\uFF6E\uFF6F\uFF70" "\uFF71\uFF72\uFF73\uFF74\uFF75" "\uFF76\uFF77\uFF78\uFF79\uFF7A"
        "\uFF7B\uFF7C\uFF7D\uFF7E\uFF7F" "\uFF80\uFF81\uFF82\uFF83\uFF84" "\uFF85\uFF86\uFF87\uFF88\uFF89"
        "\uFF8A\uFF8B\uFF8C\uFF8D\uFF8E" "\uFF8F\uFF90\uFF91\uFF92\uFF93" "\uFF94\uFF95\uFF96"
        "\uFF97\uFF98\uFF99\uFF9A\uFF9B" "\uFF9C\uFF9D"
    };
    constexpr const char fullwidth[] {
        "。「」、・" "ヲ" "ァィゥェォ"
        "ャュョッー" "アイウエオ" "カキクケコ"
        "サシスセソ" "タチツテト" "ナニヌネノ"
        "ハヒフヘホ" "マミムメモ" "ヤユヨ"
        "ラリルレロ" "ワン"
    };

    // 半角片仮名から全角片仮名へ一対一の変換
    MISC::asc( halfwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_katakana_only_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;

    // 半角の濁点と半濁点は単独では全角に変換されない
    MISC::asc( "\uFF9E\uFF9F", output, table );

    EXPECT_EQ( "\uFF9E\uFF9F", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_to_precomposed)
{
    std::string output;
    std::vector<int> table;
    constexpr const char halfwidth[] = {
        "\uFF76\uFF9E\uFF77\uFF9E\uFF78\uFF9E\uFF79\uFF9E\uFF7A\uFF9E"
        "\uFF7B\uFF9E\uFF7C\uFF9E\uFF7D\uFF9E\uFF7E\uFF9E\uFF7F\uFF9E"
        "\uFF80\uFF9E\uFF81\uFF9E\uFF82\uFF9E\uFF83\uFF9E\uFF84\uFF9E"
        "\uFF8A\uFF9E\uFF8B\uFF9E\uFF8C\uFF9E\uFF8D\uFF9E\uFF8E\uFF9E"
        "\uFF8A\uFF9F\uFF8B\uFF9F\uFF8C\uFF9F\uFF8D\uFF9F\uFF8E\uFF9F"
    };
    constexpr const char fullwidth[] {
        "\u30AC\u30AE\u30B0\u30B2\u30B4" // ガギグゲゴ
        "\u30B6\u30B8\u30BA\u30BC\u30BE" // ザジズゼゾ
        "\u30C0\u30C2\u30C5\u30C7\u30C9" // ダヂヅデド
        "\u30D0\u30D3\u30D6\u30D9\u30DC" // バビブベボ
        "\u30D1\u30D4\u30D7\u30DA\u30DD" // パピプペポ
    };

    // 合成済み文字が存在する(半)濁点付き半角片仮名は全角へ変換される
    MISC::asc( halfwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26,
        30, 31, 32, 36, 37, 38, 42, 43, 44, 48, 49, 50, 54, 55, 56,
        60, 61, 62, 66, 67, 68, 72, 73, 74, 78, 79, 80, 84, 85, 86,
        90, 91, 92, 96, 97, 98, 102, 103, 104, 108, 109, 110, 114, 115, 116,
        120, 121, 122, 126, 127, 128, 132, 133, 134, 138, 139, 140, 144, 145, 146,
        150,
    };
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_wagyo)
{
    std::string output;
    std::vector<int> table;

    // 濁点付き半角片仮名のウは全角の合成済み文字に変換される：ヴ
    // ワ行の濁点付き半角片仮名は全角に変換されない(変換表が未実装) : ヷヺ
    MISC::asc( "\uFF73\uFF9E\uFF9C\uFF9E\uFF66\uFF9E", output, table );

    EXPECT_EQ( "\u30F4\uFF9C\uFF9E\uFF66\uFF9E", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    const std::vector<int> expected_table = {
        0, 1, 2, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18
    };
    EXPECT_EQ( expected_table, table );
}

TEST_F(MISC_AscTest, halfwidth_katakana_combining_voiced_sound_mark_through)
{
    std::string output;
    std::vector<int> table;

    // 合成済み文字が存在しない(半)濁点付き半角片仮名は全角に変換されない : カ゚キ゚ク゚ケ゚コ゚
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( "\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", output, table );

    EXPECT_EQ( "\uFF76\uFF9F\uFF77\uFF9F\uFF78\uFF9F\uFF79\uFF9F\uFF7A\uFF9F", output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}


TEST_F(MISC_AscTest, fullwidth_katakana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30A1\u30A2\u30A3\u30A4\u30A5\u30A6\u30A7\u30A8\u30A9\u30AA" // ァ - オ
        "\u30AB\u30AD\u30AF\u30B1\u30B3\u30B5\u30B7\u30B9\u30BB\u30BD" // カ - ソ
        "\u30BF\u30C1\u30C3\u30C4\u30C6\u30C8\u30CA\u30CB\u30CC\u30CD\u30CE" // タ - ノ
        "\u30CF\u30D2\u30D5\u30D8\u30DB\u30DE\u30DF\u30E0\u30E1\u30E2" // ハ - モ
        "\u30E3\u30E4\u30E5\u30E6\u30E7\u30E8\u30E9\u30EA\u30EB\u30EC\u30ED" // ャ - ロ
        "\u30EE\u30EF\u30F0\u30F1\u30F2\u30F3\u30F5\u30F6" // ヮ - ヶ
    };

    // (半)濁点の付いていない全角片仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_katakana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30AC\u30AE\u30B0\u30B2\u30B4\u30B6\u30B8\u30BA\u30BC\u30BE" // ガ - ゾ
        "\u30C0\u30C2\u30C5\u30C7\u30C9\u30D0\u30D3\u30D6\u30D9\u30DC" // ダ - ボ
        "\u30F4\u30F7\u30F8\u30F9\u30FA" // ヴ - ヺ
        "\u30D1\u30D4\u30D7\u30DA0x30DD" // パ - ポ
    };

    // 合成済み文字の(半)濁点付き全角片仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_katakana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u30AB\u3099\u30AD\u3099\u30AF\u3099\u30B1\u3099\u30B3\u3099" // ガギグゲゴ
        "\u30AB\u309A\u30AD\u309A\u30AF\u309A\u30B1\u309A\u30B3\u309A" // カ゚キ゚ク゚ケ゚コ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角片仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_without_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u3041\u3042\u3043\u3044\u3045\u3046\u3047\u3048\u3049\u304A" // ぁ - お
        "\u304B\u304D\u304F\u3051\u3053\u3055\u3057\u3059\u305B\u305D" // か - そ
        "\u305F\u3061\u3063\u3064\u3066\u3068\u306A\u306B\u306C\u306D\u306E" // た - の
        "\u306F\u3072\u3075\u3078\u307B\u307E\u307F\u3080\u3081\u3082" // は - も
        "\u3083\u3084\u3085\u3086\u3087\u3088\u3089\u308A\u308B\u308C\u308D" // ゃ - ろ
        "\u308E\u308F\u3090\u3091\u3092\u3093\u3095\u3096" // ゎ - ゖ
    };

    // (半)濁点の付いていない全角平仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_precomposed_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u304C\u304E\u3050\u3052\u3054\u3056\u3058\u305A\u305C\u305E" // が - ぞ
        "\u3060\u3062\u3065\u3067\u3069\u3070\u3073\u3076\u3079\u307C" // だ - ぼ
        "\u3094" // ゔ
        "\u3071\u3074\u3077\u307A0x307D" // ぱ - ぽ
    };

    // 合成済み文字の(半)濁点付き全角平仮名はそのまま
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}

TEST_F(MISC_AscTest, fullwidth_hiragana_combining_voiced_sound_mark)
{
    std::string output;
    std::vector<int> table;
    constexpr const char fullwidth[] {
        "\u304B\u3099\u304D\u3099\u304F\u3099\u3051\u3099\u3053\u3099" // がぎぐげご
        "\u304B\u309A\u304D\u309A\u304F\u309A\u3051\u309A\u3053\u309A" // か゚き゚く゚け゚こ゚
    };

    // 合成済み文字の有無に関係なく(半)濁点の結合文字が付いている全角平仮名は変換されない
    // NOTE: 組み合わせが多いのでテストは網羅していない
    MISC::asc( fullwidth, output, table );

    EXPECT_EQ( fullwidth, output );
    EXPECT_EQ( output.size(), table.size() - 1 );
    for( int i = 0, size = table.size(); i < size; ++i ) {
        EXPECT_EQ( i, table.at( i ) );
    }
}


class MISC_StartsWith : public ::testing::Test {};

TEST_F(MISC_StartsWith, null_terminated_string_with_zero_length)
{
    EXPECT_TRUE( MISC::starts_with( "", "" ) );
    EXPECT_TRUE( MISC::starts_with( "helloworld", "" ) );
    EXPECT_FALSE( MISC::starts_with( "", "helloworld" ) );
}

TEST_F(MISC_StartsWith, null_terminated_string)
{
    EXPECT_TRUE( MISC::starts_with( "hello", "hello" ) );
    EXPECT_TRUE( MISC::starts_with( "helloworld", "hello" ) );
    EXPECT_FALSE( MISC::starts_with( "hello", "helloworld" ) );
}


class MISC_ParseHtmlFormData : public ::testing::Test {};

TEST_F(MISC_ParseHtmlFormData, empty_html)
{
    auto result = MISC::parse_html_form_data( std::string{} );
    EXPECT_TRUE( result.empty() );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum)
{
    auto result = MISC::parse_html_form_data( "<input type=hidden name=FOO value=100>" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum_uppercase)
{
    auto result = MISC::parse_html_form_data( "<INPUT TYPE=hidden NAME=FOO VALUE=100>" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="FOO" value="100">)" );
    decltype(result) expect{ { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_datum)
{
    auto result = MISC::parse_html_form_data( "<input type=submit value=書き込む name=submit>" );
    decltype(result) expect{ { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value="書き込む" name="submit">)" );
    decltype(result) expect{ { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum)
{
    auto result = MISC::parse_html_form_data( "<textarea name=MESSAGE>あかさたな</textarea>" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_have_tag)
{
    auto result = MISC::parse_html_form_data( "<textarea name=MESSAGE>あか<br>さたな</textarea>" );
    decltype(result) expect{ { "MESSAGE", "あか<br>さたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum_uppercase)
{
    auto result = MISC::parse_html_form_data( "<TEXTAREA NAME=MESSAGE>あかさたな</TEXTAREA>" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_datum_with_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE">"あかさたな"</textarea>)" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_1)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE">"あかさたな"</textarea>)"
                                              "<input type=submit value=書き込む name=submit>"
                                              R"(<input type="hidden" name="FOO" value="100">)" );
    decltype(result) expect{ { "MESSAGE", "あかさたな" }, { "submit", "書き込む" }, { "FOO", "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_2)
{
    auto result = MISC::parse_html_form_data( "<input type=submit value=書き込む name=submit>"
                                              R"(<input type="hidden" name="FOO" value="100">)"
                                              R"(<textarea name="MESSAGE">"あかさたな"</textarea>)" );
    decltype(result) expect{ { "submit", "書き込む" }, { "FOO", "100" }, { "MESSAGE", "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, multi_data_3)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="FOO" value="100">)"
                                              R"(<textarea name="MESSAGE">"あかさたな"</textarea>)"
                                              "<input type=submit value=書き込む name=submit>" );
    decltype(result) expect{ { "FOO", "100" }, { "MESSAGE", "あかさたな" }, { "submit", "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name="" value="100">)" );
    decltype(result) expect{ { std::string{}, "100" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, hidden_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="hidden" name=FOO value="">)" );
    decltype(result) expect{ { "FOO", std::string{} } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value=書き込む name="">)" );
    decltype(result) expect{ { std::string{}, "書き込む" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, submit_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<input type="submit" value="" name=submit>)" );
    decltype(result) expect{ { "submit", std::string{} } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_empty_name_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="">あかさたな</textarea>)" );
    decltype(result) expect{ { std::string{}, "あかさたな" } };
    EXPECT_EQ( result, expect );
}

TEST_F(MISC_ParseHtmlFormData, textarea_empty_value_by_double_quotes)
{
    auto result = MISC::parse_html_form_data( R"(<textarea name="MESSAGE"></textarea>)" );
    decltype(result) expect{ { "MESSAGE", std::string{} } };
    EXPECT_EQ( result, expect );
}


class MISC_ParseHtmlFormActionTest : public ::testing::Test {};

TEST_F(MISC_ParseHtmlFormActionTest, empty_html)
{
    std::string result = MISC::parse_html_form_action( std::string{} );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, unquote_post)
{
    const std::string html = R"(<form method=POST action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, quote_post)
{
    const std::string html = R"(<form method="POST" action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, subbbscgi)
{
    const std::string html = R"(<form method="POST" action="/test/subbbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/subbbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, with_parameter)
{
    const std::string html = R"(<form method="POST" action="/test/bbs.cgi?foo=bar">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi?foo=bar" );
}

TEST_F(MISC_ParseHtmlFormActionTest, ignore_attributes)
{
    const std::string html = R"(<form method="POST" accept-charset="Shift_JIS" action="/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_an_upper_order)
{
    const std::string html = R"(<form method="POST" action="../test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_same_hierarchy)
{
    std::string html = R"(<form method="POST" action="./bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/bbs.cgi" );

    html = R"(<form method="POST" action="./subbbs.cgi">")";
    result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, "/test/subbbs.cgi" );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_double_upper_orders)
{
    const std::string html = R"(<form method="POST" action="../../test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, relative_path_middle_upper_order)
{
    const std::string html = R"(<form method="POST" action="/test/../bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, absolute_path_without_scheme)
{
    const std::string html = R"(<form method="POST" action="//example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, http_url)
{
    const std::string html = R"(<form method="POST" action="http://example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

TEST_F(MISC_ParseHtmlFormActionTest, https_url)
{
    const std::string html = R"(<form method="POST" action="https://example.test/test/bbs.cgi">")";
    std::string result = MISC::parse_html_form_action( html );
    EXPECT_EQ( result, std::string{} );
}

} // namespace
