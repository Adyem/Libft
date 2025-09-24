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
