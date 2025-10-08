#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_tolower_basic, "ft_to_lower basic")
{
    char string[4];

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    string[0] = 'A';
    string[1] = 'B';
    string[2] = 'C';
    string[3] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "abc"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_tolower_mixed, "ft_to_lower mixed characters")
{
    char string[7];
    string[0] = 'A';
    string[1] = '1';
    string[2] = 'B';
    string[3] = '!';
    string[4] = 'C';
    string[5] = '?';
    string[6] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "a1b!c?"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_tolower_empty, "ft_to_lower empty string")
{
    char string[1];
    string[0] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_tolower_nullptr, "ft_to_lower nullptr")
{
    ft_errno = ER_SUCCESS;
    ft_to_lower(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_tolower_non_ascii_preserved, "ft_to_lower leaves non-ASCII bytes unchanged")
{
    char string[5];

    string[0] = static_cast<char>(0xC7);
    string[1] = 'A';
    string[2] = static_cast<char>(0x80);
    string[3] = 'Z';
    string[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_to_lower(string);
    FT_ASSERT_EQ(static_cast<char>(0xC7), string[0]);
    FT_ASSERT_EQ('a', string[1]);
    FT_ASSERT_EQ(static_cast<char>(0x80), string[2]);
    FT_ASSERT_EQ('z', string[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_tolower_stops_at_terminator, "ft_to_lower stops when encountering the null terminator")
{
    char string[6];

    string[0] = 'H';
    string[1] = 'I';
    string[2] = '\0';
    string[3] = 'X';
    string[4] = 'Y';
    string[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_to_lower(string);
    FT_ASSERT_EQ('h', string[0]);
    FT_ASSERT_EQ('i', string[1]);
    FT_ASSERT_EQ('\0', string[2]);
    FT_ASSERT_EQ('X', string[3]);
    FT_ASSERT_EQ('Y', string[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
