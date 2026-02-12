#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_strcmp_equal, "ft_strcmp equal strings")
{
    FT_ASSERT_EQ(0, ft_strcmp("abc", "abc"));
    return (1);
}

FT_TEST(test_strcmp_null, "ft_strcmp with nullptr")
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    return (1);
}

FT_TEST(test_strcmp_second_null, "ft_strcmp second string nullptr")
{
    FT_ASSERT_EQ(-1, ft_strcmp("abc", ft_nullptr));
    return (1);
}

FT_TEST(test_strcmp_less, "ft_strcmp lexicographically less")
{
    FT_ASSERT(ft_strcmp("abc", "abd") < 0);
    return (1);
}

FT_TEST(test_strcmp_greater, "ft_strcmp lexicographically greater")
{
    FT_ASSERT(ft_strcmp("abd", "abc") > 0);
    return (1);
}

FT_TEST(test_strcmp_prefix, "ft_strcmp prefix difference")
{
    FT_ASSERT(ft_strcmp("abc", "abcd") < 0);
    FT_ASSERT(ft_strcmp("abcd", "abc") > 0);
    return (1);
}

FT_TEST(test_strcmp_empty, "ft_strcmp empty strings")
{
    FT_ASSERT_EQ(0, ft_strcmp("", ""));
    FT_ASSERT(ft_strcmp("", "a") < 0);
    FT_ASSERT(ft_strcmp("a", "") > 0);
    return (1);
}

FT_TEST(test_strcmp_both_null, "ft_strcmp both nullptr")
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, ft_nullptr));
    return (1);
}

FT_TEST(test_strcmp_high_bit_ordering, "ft_strcmp treats bytes as unsigned")
{
    char left[2];
    char right[2];

    left[0] = static_cast<char>(0x90);
    left[1] = '\0';
    right[0] = static_cast<char>(0x7F);
    right[1] = '\0';
    FT_ASSERT(ft_strcmp(left, right) > 0);
    FT_ASSERT(ft_strcmp(right, left) < 0);
    return (1);
}

FT_TEST(test_strcmp_recovers_after_null_failure, "ft_strcmp resets errno after null input")
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    FT_ASSERT_EQ(0, ft_strcmp("match", "match"));
    return (1);
}
