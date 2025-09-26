#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strstr_basic, "ft_strstr basic match")
{
    const char *haystack = "hello world";
    const char *needle = "world";

    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(haystack + 6, ft_strstr(haystack, needle));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
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

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(ft_nullptr, "a"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(haystack, ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
