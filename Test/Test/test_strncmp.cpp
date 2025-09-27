#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strncmp_prefix_equal, "ft_strncmp equal prefix")
{
    FT_ASSERT_EQ(0, ft_strncmp("abcdef", "abcxyz", 3));
    return (1);
}

FT_TEST(test_strncmp_less, "ft_strncmp less")
{
    FT_ASSERT(ft_strncmp("abc", "abd", 3) < 0);
    return (1);
}

FT_TEST(test_strncmp_greater, "ft_strncmp greater")
{
    FT_ASSERT(ft_strncmp("abd", "abc", 3) > 0);
    return (1);
}

FT_TEST(test_strncmp_zero_length, "ft_strncmp zero length")
{
    FT_ASSERT_EQ(0, ft_strncmp("abc", "xyz", 0));
    return (1);
}

FT_TEST(test_strncmp_shorter_first, "ft_strncmp shorter first")
{
    FT_ASSERT(ft_strncmp("ab", "abc", 3) < 0);
    return (1);
}

FT_TEST(test_strncmp_shorter_second, "ft_strncmp shorter second")
{
    FT_ASSERT(ft_strncmp("abc", "ab", 3) > 0);
    return (1);
}

FT_TEST(test_strncmp_null_arguments, "ft_strncmp null arguments return error")
{
    FT_ASSERT_EQ(-1, ft_strncmp(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(-1, ft_strncmp("abc", ft_nullptr, 3));
    return (1);
}

FT_TEST(test_strncmp_high_bit_values, "ft_strncmp orders high-bit characters")
{
    char first[3];
    char second[3];

    first[0] = static_cast<char>(0x90);
    first[1] = 'a';
    first[2] = '\0';
    second[0] = static_cast<char>(0x10);
    second[1] = 'a';
    second[2] = '\0';
    FT_ASSERT(ft_strncmp(first, second, 2) > 0);
    return (1);
}

FT_TEST(test_strncmp_embedded_nulls_stop_comparison, "ft_strncmp stops comparing after embedded null")
{
    char first[5];
    char second[5];

    first[0] = 'a';
    first[1] = 'b';
    first[2] = '\0';
    first[3] = 'c';
    first[4] = '\0';
    second[0] = 'a';
    second[1] = 'b';
    second[2] = '\0';
    second[3] = 'd';
    second[4] = '\0';
    FT_ASSERT_EQ(0, ft_strncmp(first, second, 4));
    return (1);
}
