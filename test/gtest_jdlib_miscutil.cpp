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

} // namespace
