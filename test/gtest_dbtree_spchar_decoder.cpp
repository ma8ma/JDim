#include "dbtree/node.h"
#include "dbtree/spchar_decoder.h"

#include "gtest/gtest.h"

#include <cstring>


namespace {

class DBTREE_DecodeCharTest : public ::testing::Test {};

TEST_F(DBTREE_DecodeCharTest, decimal_hiragana_a)
{
    char out_char[64] = {0};
    int n_in;
    int n_out;
    constexpr bool only_check = true;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&#12354;", n_in, out_char, n_out, !only_check ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( u8"あ", out_char );
    EXPECT_EQ( 3, n_out );
}

TEST_F(DBTREE_DecodeCharTest, hexadecimal_hiragana_a)
{
    char out_char[64] = {0};
    int n_in;
    int n_out;
    constexpr bool only_check = true;
    EXPECT_EQ( DBTREE::NODE_TEXT, DBTREE::decode_char( "&#x3042;", n_in, out_char, n_out, !only_check ) );
    EXPECT_EQ( 8, n_in );
    EXPECT_STREQ( u8"あ", out_char );
    EXPECT_EQ( 3, n_out );
}

} // namespace
