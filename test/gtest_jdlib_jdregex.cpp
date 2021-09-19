// SPDX-License-Identifier: GPL-2.0-only

#include "jdlib/jdregex.h"

#include "gtest/gtest.h"


namespace {

constexpr bool icase = false;
constexpr bool newline = false;
constexpr bool notbol = false;
constexpr bool noteol = false;

class Regex_FindFirstStrOfTest : public ::testing::Test {};

TEST_F(Regex_FindFirstStrOfTest, invalid_both_arguments)
{
    const JDLIB::RegexPattern pattern( "(?<name>foobar)", icase, newline );
    JDLIB::Regex regex;
    const std::vector<std::string> named_caps = { "name" };
    EXPECT_TRUE( regex.match( pattern, "foobar", 0, notbol, noteol, named_caps ) );
    constexpr std::size_t invalid_group = 10;
    EXPECT_EQ( regex.find_first_str_of( "invalid_name", invalid_group ), "" );
}

TEST_F(Regex_FindFirstStrOfTest, prioritize_named_capture)
{
    const JDLIB::RegexPattern pattern( "(?<name>foobar) (fallback)", icase, newline );
    JDLIB::Regex regex;
    const std::vector<std::string> named_caps = { "name" };
    EXPECT_TRUE( regex.match( pattern, "foobar fallback", 0, notbol, noteol, named_caps ) );
    constexpr std::size_t valid_group = 2;
    EXPECT_EQ( regex.find_first_str_of( "name", valid_group ), "foobar" );
}

TEST_F(Regex_FindFirstStrOfTest, unregistered_name)
{
    const JDLIB::RegexPattern pattern( "(?<name>foobar) (fallback)", icase, newline );
    JDLIB::Regex regex;
    EXPECT_TRUE( regex.match( pattern, "foobar fallback", 0, notbol, noteol ) );
    constexpr std::size_t valid_group = 2;
    EXPECT_EQ( regex.find_first_str_of( "name", valid_group ), "fallback" );
}

TEST_F(Regex_FindFirstStrOfTest, register_invalid_name)
{
    JDLIB::RegexPattern pattern( "(fallback) (?<name>foobar)", icase, newline );
    JDLIB::Regex regex;
    const std::vector<std::string> named_caps = { "invalid_name" };
    EXPECT_TRUE( regex.match( pattern, "fallback foobar", 0, notbol, noteol, named_caps ) );
    constexpr std::size_t valid_group = 1;
    EXPECT_EQ( regex.find_first_str_of( "name", valid_group ), "fallback" );
}

} // namespace
