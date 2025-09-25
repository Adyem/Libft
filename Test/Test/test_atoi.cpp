#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_atoi_simple, "ft_atoi simple")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(42, ft_atoi("42"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_negative, "ft_atoi negative")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(-13, ft_atoi("-13"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_intmax, "ft_atoi INT_MAX")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MAX);
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(integer_string.c_str()));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_intmin, "ft_atoi INT_MIN")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MIN);
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(integer_string.c_str()));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_overflow_sets_erange, "ft_atoi clamps overflow to INT_MAX")
{
    ft_string overflow_string;

    overflow_string = ft_to_string(FT_INT_MAX);
    overflow_string += "9";
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(overflow_string.c_str()));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_atoi_underflow_sets_erange, "ft_atoi clamps underflow to INT_MIN")
{
    ft_string underflow_string;

    underflow_string = ft_to_string(FT_INT_MIN);
    underflow_string += "9";
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(underflow_string.c_str()));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_atoi_trailing_whitespace_sets_einval, "ft_atoi trailing whitespace sets FT_EINVAL")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(42, ft_atoi("  \t\n42  "));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_atoi_trailing_chars_sets_einval, "ft_atoi stops at non-digit")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(123, ft_atoi("123abc"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_atoi_plus_sign_success, "ft_atoi plus sign")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(7, ft_atoi("+7"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_zero_success, "ft_atoi zero")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(0, ft_atoi("0"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_null_input_sets_einval, "ft_atoi null input returns zero")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_atoi(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_atoi_no_digits_sets_einval, "ft_atoi no digits sets FT_EINVAL")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_atoi("abc"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_atoi_digits_only_clears_errno, "ft_atoi digits clear ft_errno")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(314, ft_atoi("314"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
