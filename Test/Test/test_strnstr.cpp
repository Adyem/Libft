#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

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

FT_TEST(test_strnstr_null_arguments, "ft_strnstr null arguments return nullptr")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr(ft_nullptr, "abc", 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, ft_strnstr("abc", ft_nullptr, 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_strnstr_errno_resets_on_success, "ft_strnstr resets errno on success")
{
    const char *haystack = "prefix";

    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(haystack + 3, ft_strnstr(haystack, "fix", 6));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
