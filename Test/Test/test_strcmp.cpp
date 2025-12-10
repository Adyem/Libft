#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strcmp_equal, "ft_strcmp equal strings")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strcmp("abc", "abc"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_null, "ft_strcmp with nullptr")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_second_null, "ft_strcmp second string nullptr")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strcmp("abc", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_less, "ft_strcmp lexicographically less")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("abc", "abd") < 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_greater, "ft_strcmp lexicographically greater")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("abd", "abc") > 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_prefix, "ft_strcmp prefix difference")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("abc", "abcd") < 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("abcd", "abc") > 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_empty, "ft_strcmp empty strings")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strcmp("", ""));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("", "a") < 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp("a", "") > 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_both_null, "ft_strcmp both nullptr")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp(left, right) > 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT(ft_strcmp(right, left) < 0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strcmp_recovers_after_null_failure, "ft_strcmp resets errno after null input")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_strcmp(ft_nullptr, "abc"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strcmp("match", "match"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
