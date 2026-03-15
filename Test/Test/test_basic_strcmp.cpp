#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strcmp_equal)
{
    FT_ASSERT_EQ(0, ft_strcmp("abc", "abc"));
    return (1);
}

FT_TEST(test_basic_strcmp_null)
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    return (1);
}

FT_TEST(test_basic_strcmp_second_null)
{
    FT_ASSERT_EQ(-1, ft_strcmp("abc", ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strcmp_less)
{
    FT_ASSERT(ft_strcmp("abc", "abd") < 0);
    return (1);
}

FT_TEST(test_basic_strcmp_greater)
{
    FT_ASSERT(ft_strcmp("abd", "abc") > 0);
    return (1);
}

FT_TEST(test_basic_strcmp_prefix)
{
    FT_ASSERT(ft_strcmp("abc", "abcd") < 0);
    FT_ASSERT(ft_strcmp("abcd", "abc") > 0);
    return (1);
}

FT_TEST(test_basic_strcmp_empty)
{
    FT_ASSERT_EQ(0, ft_strcmp("", ""));
    FT_ASSERT(ft_strcmp("", "a") < 0);
    FT_ASSERT(ft_strcmp("a", "") > 0);
    return (1);
}

FT_TEST(test_basic_strcmp_both_null)
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strcmp_high_bit_ordering)
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

FT_TEST(test_basic_strcmp_recovers_after_null_failure)
{
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    FT_ASSERT_EQ(0, ft_strcmp("match", "match"));
    return (1);
}
