#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_toupper_basic, "ft_to_upper basic")
{
    char string[4];

    string[0] = 'a';
    string[1] = 'b';
    string[2] = 'c';
    string[3] = '\0';
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "ABC"));
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
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "A1B!C?"));
    return (1);
}

FT_TEST(test_toupper_empty, "ft_to_upper empty string")
{
    char string[1];

    string[0] = '\0';
    ft_to_upper(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, ""));
    return (1);
}

FT_TEST(test_toupper_nullptr, "ft_to_upper nullptr")
{
    ft_to_upper(ft_nullptr);
    FT_ASSERT_EQ(1, 1);
    return (1);
}
