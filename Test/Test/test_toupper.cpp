#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_toupper_basic, "ft_to_upper basic")
{
    char string[4];

    ft_errno = FT_EINVAL;
    string[0] = 'a';
    string[1] = 'b';
    string[2] = 'c';
    string[3] = '\0';
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "ABC"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_toupper_mixed, "ft_to_upper mixed characters")
{
    char string[7];

    string[0] = 'a';
    string[1] = '1';
    string[2] = 'b';
    string[3] = '!';
    string[4] = 'c';
    string[5] = '?';
    string[6] = '\0';
    ft_errno = FT_EINVAL;
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "A1B!C?"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_toupper_empty, "ft_to_upper empty string")
{
    char string[1];

    string[0] = '\0';
    ft_errno = FT_EINVAL;
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, ""));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_toupper_nullptr, "ft_to_upper nullptr")
{
    ft_errno = ER_SUCCESS;
    ft_to_upper(ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_toupper_stops_at_terminator, "ft_to_upper stops at first null byte")
{
    char string[6];

    string[0] = 'a';
    string[1] = static_cast<char>(0xE1);
    string[2] = '\0';
    string[3] = 'x';
    string[4] = 'y';
    string[5] = '\0';
    ft_errno = FT_EINVAL;
    ft_to_upper(string);
    FT_ASSERT_EQ('A', string[0]);
    FT_ASSERT_EQ(static_cast<char>(0xE1), string[1]);
    FT_ASSERT_EQ('\0', string[2]);
    FT_ASSERT_EQ('x', string[3]);
    FT_ASSERT_EQ('y', string[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
