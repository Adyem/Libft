#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strncmp_prefix_equal)
{
    FT_ASSERT_EQ(0, ft_strncmp("abcdef", "abcxyz", 3));
    return (1);
}

FT_TEST(test_basic_strncmp_less)
{
    FT_ASSERT(ft_strncmp("abc", "abd", 3) < 0);
    return (1);
}

FT_TEST(test_basic_strncmp_greater)
{
    FT_ASSERT(ft_strncmp("abd", "abc", 3) > 0);
    return (1);
}

FT_TEST(test_basic_strncmp_zero_length)
{
    FT_ASSERT_EQ(0, ft_strncmp("abc", "xyz", 0));
    return (1);
}

FT_TEST(test_basic_strncmp_zero_length_null_arguments)
{
    FT_ASSERT_EQ(0, ft_strncmp(ft_nullptr, "abc", 0));
    FT_ASSERT_EQ(0, ft_strncmp("abc", ft_nullptr, 0));
    FT_ASSERT_EQ(0, ft_strncmp(ft_nullptr, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_basic_strncmp_shorter_first)
{
    FT_ASSERT(ft_strncmp("ab", "abc", 3) < 0);
    return (1);
}

FT_TEST(test_basic_strncmp_shorter_second)
{
    FT_ASSERT(ft_strncmp("abc", "ab", 3) > 0);
    return (1);
}

FT_TEST(test_basic_strncmp_null_arguments)
{
    FT_ASSERT_EQ(-1, ft_strncmp(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(-1, ft_strncmp("abc", ft_nullptr, 3));
    return (1);
}

FT_TEST(test_basic_strncmp_high_bit_values)
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

FT_TEST(test_basic_strncmp_embedded_nulls_stop_comparison)
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

FT_TEST(test_basic_strncmp_recovers_after_null_failure)
{
    FT_ASSERT_EQ(-1, ft_strncmp(ft_nullptr, "abc", 2));
    FT_ASSERT(ft_strncmp("abc", "abd", 3) < 0);
    return (1);
}

FT_TEST(test_basic_strncmp_limit_blocks_mismatch_after_error)
{
    FT_ASSERT_EQ(-1, ft_strncmp("abc", ft_nullptr, 2));
    FT_ASSERT_EQ(0, ft_strncmp("prefix", "prelude", 3));
    return (1);
}

FT_TEST(test_basic_strncmp_stops_at_null_within_limit)
{
    char left[5];
    char right[5];

    left[0] = 'a';
    left[1] = 'b';
    left[2] = '\0';
    left[3] = 'x';
    left[4] = '\0';
    right[0] = 'a';
    right[1] = 'b';
    right[2] = 'c';
    right[3] = 'd';
    right[4] = '\0';
    FT_ASSERT(ft_strncmp(left, right, 4) < 0);
    return (1);
}
