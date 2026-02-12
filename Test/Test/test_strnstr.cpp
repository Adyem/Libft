#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_strnstr_basic, "ft_strnstr basic")
{
    const char *haystack = "hello world";

    FT_ASSERT_EQ(haystack + 6, ft_strnstr(haystack, "world", 11));
    return (1);
}

FT_TEST(test_strnstr_not_found, "ft_strnstr not found")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "xyz", 5));
    return (1);
}

FT_TEST(test_strnstr_empty_needle, "ft_strnstr empty needle")
{
    const char *haystack = "hello";

    FT_ASSERT_EQ(haystack, ft_strnstr(haystack, "", 5));
    return (1);
}

FT_TEST(test_strnstr_size_limit, "ft_strnstr size limit")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "lo", 3));
    return (1);
}

FT_TEST(test_strnstr_zero_size, "ft_strnstr zero size")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "he", 0));
    return (1);
}

FT_TEST(test_strnstr_empty_needle_zero_size, "ft_strnstr empty needle ignores size limit")
{
    const char *haystack = "hello";

    FT_ASSERT_EQ(haystack, ft_strnstr(haystack, "", 0));
    return (1);
}

FT_TEST(test_strnstr_zero_size_allows_null_haystack, "ft_strnstr zero size allows null haystack")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "needle", 0));
    return (1);
}

FT_TEST(test_strnstr_zero_size_null_haystack_empty_needle, "ft_strnstr zero size allows null haystack for empty needle")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "", 0));
    return (1);
}

FT_TEST(test_strnstr_null_arguments, "ft_strnstr null arguments return nullptr")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("abc", ft_nullptr, 3));
    return (1);
}

FT_TEST(test_strnstr_recovers_after_null_haystack, "ft_strnstr clears errno after null haystack failure")
{
    const char *haystack;

    haystack = "searchable";
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "search", 7));
    FT_ASSERT_EQ(haystack + 4, ft_strnstr(haystack, "ch", 8));
    return (1);
}

FT_TEST(test_strnstr_errno_resets_on_success, "ft_strnstr resets errno on success")
{
    const char *haystack = "prefix";

    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "fix", 6));
    return (1);
}

FT_TEST(test_strnstr_restart_within_limit, "ft_strnstr restarts search within limit")
{
    const char *haystack = "abcabcd";

    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "abcd", 7));
    return (1);
}

FT_TEST(test_strnstr_same_buffer_with_length_limit, "ft_strnstr same buffer respects length limit")
{
    char buffer[] = "abcdef";

    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(buffer, buffer, 3));
    return (1);
}

FT_TEST(test_strnstr_match_at_limit_boundary, "ft_strnstr finds match ending at search limit")
{
    const char *haystack = "abcd";

    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "d", 4));
    return (1);
}

FT_TEST(test_strnstr_limit_blocks_partial_match, "ft_strnstr does not return matches truncated by size")
{
    const char *haystack = "foobar";

    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(haystack, "bar", 5));
    return (1);
}
