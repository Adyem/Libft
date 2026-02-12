#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_strstr_basic, "ft_strstr basic match")
{
    const char *haystack = "hello world";
    const char *needle = "world";

    FT_ASSERT_EQ(haystack + 6, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_strstr_not_found, "ft_strstr not found")
{
    const char *haystack = "hello";
    const char *needle = "world";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_strstr_empty_needle, "ft_strstr empty needle")
{
    const char *haystack = "abc";

    FT_ASSERT_EQ(haystack, ft_strstr(haystack, ""));
    return (1);
}

FT_TEST(test_strstr_empty_haystack, "ft_strstr empty haystack")
{
    const char *haystack = "";
    const char *needle = "a";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_strstr_null, "ft_strstr with nullptr")
{
    const char *haystack = "abc";

    FT_ASSERT_EQ(ft_nullptr, ft_strstr(ft_nullptr, "a"));
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, ft_nullptr));
    return (1);
}

FT_TEST(test_strstr_overlapping_partial_match, "ft_strstr restarts after overlapping failure")
{
    const char *haystack = "ababac";
    const char *needle = "abac";

    FT_ASSERT_EQ(haystack + 2, ft_strstr(haystack, needle));
    return (1);
}

FT_TEST(test_strstr_recovers_after_missing_match, "ft_strstr clears errno after prior miss")
{
    const char *haystack;

    haystack = "pattern";
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, "xyz"));
    FT_ASSERT_EQ(haystack + 3, ft_strstr(haystack, "tern"));
    return (1);
}
