#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_tolower_basic, "ft_to_lower basic")
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
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, "a1b!c?"));
    return (1);
}

FT_TEST(test_tolower_empty, "ft_to_lower empty string")
{
    char string[1];
    string[0] = '\0';
    ft_to_lower(string);
    FT_ASSERT_EQ(0, ft_strcmp(string, ""));
    return (1);
}

FT_TEST(test_tolower_nullptr, "ft_to_lower nullptr")
{
    ft_to_lower(ft_nullptr);
    FT_ASSERT_EQ(1, 1);
    return (1);
}
