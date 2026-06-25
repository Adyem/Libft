#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_tolower_basic)
{
    char string[4];

    string[0] = 'A';
    string[1] = 'B';
    string[2] = 'C';
    string[3] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "abc"));
    return (1);
}

FT_TEST(test_basic_tolower_mixed)
{
    char string[7];
    string[0] = 'A';
    string[1] = '1';
    string[2] = 'B';
    string[3] = '!';
    string[4] = 'C';
    string[5] = '?';
    string[6] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "a1b!c?"));
    return (1);
}

FT_TEST(test_basic_tolower_empty)
{
    char string[1];
    string[0] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, ""));
    return (1);
}

FT_TEST(test_basic_tolower_nullptr)
{
    ft_to_lower(ft_nullptr);
    return (1);
}

FT_TEST(test_basic_tolower_non_ascii_preserved)
{
    char string[5];

    string[0] = static_cast<char>(0xC7);
    string[1] = 'A';
    string[2] = static_cast<char>(0x80);
    string[3] = 'Z';
    string[4] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(static_cast<char>(0xC7), string[0]);
    FT_ASSERT_EQ('a', string[1]);
    FT_ASSERT_EQ(static_cast<char>(0x80), string[2]);
    FT_ASSERT_EQ('z', string[3]);
    return (1);
}

FT_TEST(test_basic_tolower_stops_at_terminator)
{
    char string[6];

    string[0] = 'H';
    string[1] = 'I';
    string[2] = '\0';
    string[3] = 'X';
    string[4] = 'Y';
    string[5] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ('h', string[0]);
    FT_ASSERT_EQ('i', string[1]);
    FT_ASSERT_EQ('\0', string[2]);
    FT_ASSERT_EQ('X', string[3]);
    FT_ASSERT_EQ('Y', string[4]);
    return (1);
}

FT_TEST(test_basic_tolower_recovers_after_nullptr)
{
    char string[4];

    string[0] = 'Q';
    string[1] = 'R';
    string[2] = 'S';
    string[3] = '\0';
    ft_to_lower(ft_nullptr);
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "qrs"));
    return (1);
}
