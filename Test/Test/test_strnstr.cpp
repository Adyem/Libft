#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strnstr_basic, "ft_strnstr basic")
{
    const char *haystack = "hello world";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack + 6, ft_strnstr(haystack, "world", 11));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_not_found, "ft_strnstr not found")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "xyz", 5));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_empty_needle, "ft_strnstr empty needle")
{
    const char *haystack = "hello";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack, ft_strnstr(haystack, "", 5));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_size_limit, "ft_strnstr size limit")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "lo", 3));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_zero_size, "ft_strnstr zero size")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("hello", "he", 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_empty_needle_zero_size, "ft_strnstr empty needle ignores size limit")
{
    const char *haystack = "hello";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack, ft_strnstr(haystack, "", 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_zero_size_allows_null_haystack, "ft_strnstr zero size allows null haystack")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "needle", 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_zero_size_null_haystack_empty_needle, "ft_strnstr zero size allows null haystack for empty needle")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "", 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_null_arguments, "ft_strnstr null arguments return nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("abc", ft_nullptr, 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_errno_resets_on_success, "ft_strnstr resets errno on success")
{
    const char *haystack = "prefix";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "fix", 6));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_restart_within_limit, "ft_strnstr restarts search within limit")
{
    const char *haystack = "abcabcd";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "abcd", 7));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_same_buffer_with_length_limit, "ft_strnstr same buffer respects length limit")
{
    char buffer[] = "abcdef";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(buffer, buffer, 3));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
