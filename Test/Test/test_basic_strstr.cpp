#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strstr_basic)
{
    const char *haystack = "hello world";
    const char *needle = "world";

    FT_ASSERT_EQ(haystack + 6, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_basic_strstr_not_found)
{
    const char *haystack = "hello";
    const char *needle = "world";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_basic_strstr_empty_needle)
{
    const char *haystack = "abc";

    FT_ASSERT_EQ(haystack, ft_strstr(haystack, ""));
    return (1);
}

FT_TEST(test_basic_strstr_empty_haystack)
{
    const char *haystack = "";
    const char *needle = "a";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_basic_strstr_null)
{
    const char *haystack = "abc";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(ft_nullptr, "a"));
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, ft_nullptr));
    return (1);
}

FT_TEST(test_basic_strstr_overlapping_partial_match)
{
    const char *haystack = "ababac";
    const char *needle = "abac";

    FT_ASSERT_EQ(haystack + 2, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_basic_strstr_recovers_after_missing_match)
{
    const char *haystack;

    haystack = "pattern";
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, "xyz"));
    FT_ASSERT_EQ(haystack + 3, ft_strstr(haystack, "tern"));
    return (1);
}
