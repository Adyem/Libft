#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strstr_basic, "ft_strstr basic match")
{
    const char *haystack = "hello world";
    const char *needle = "world";

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(haystack + 6, ft_strstr(haystack, needle));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strstr_not_found, "ft_strstr not found")
{
    const char *haystack = "hello";
    const char *needle = "world";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strstr_empty_needle, "ft_strstr empty needle")
{
    const char *haystack = "abc";

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(haystack, ft_strstr(haystack, ""));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strstr_empty_haystack, "ft_strstr empty haystack")
{
    const char *haystack = "";
    const char *needle = "a";

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, needle));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strstr_null, "ft_strstr with nullptr")
{
    const char *haystack = "abc";

    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(ft_nullptr, "a"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strstr_overlapping_partial_match, "ft_strstr restarts after overlapping failure")
{
    const char *haystack = "ababac";
    const char *needle = "abac";

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(haystack + 2, ft_strstr(haystack, needle));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strstr_recovers_after_missing_match, "ft_strstr clears errno after prior miss")
{
    const char *haystack;

    haystack = "pattern";
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, "xyz"));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(haystack + 3, ft_strstr(haystack, "tern"));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
