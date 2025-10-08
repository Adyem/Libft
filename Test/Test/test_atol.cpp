#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_atol_simple_success, "ft_atol simple parses value")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(1234L, ft_atol("1234"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atol_overflow_sets_erange, "ft_atol clamps overflow to FT_LONG_MAX")
{
    ft_string overflow_string;

    overflow_string = ft_to_string(FT_LONG_MAX);
    overflow_string += "9";
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_atol(overflow_string.c_str()));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_atol_underflow_sets_erange, "ft_atol clamps underflow to FT_LONG_MIN")
{
    ft_string underflow_string;

    underflow_string = ft_to_string(FT_LONG_MIN);
    underflow_string += "9";
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MIN, ft_atol(underflow_string.c_str()));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_atol_null_input_sets_einval, "ft_atol null input returns zero")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0L, ft_atol(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atol_trailing_chars_sets_einval, "ft_atol stops at non-digit")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(98765L, ft_atol("98765abc"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atol_no_digits_sets_einval, "ft_atol rejects non-numeric input")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0L, ft_atol("abc"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atol_long_min_exact_success, "ft_atol FT_LONG_MIN exact")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_LONG_MIN);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_LONG_MIN, ft_atol(integer_string.c_str()));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atol_long_max_exact_success, "ft_atol FT_LONG_MAX exact")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_LONG_MAX);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_atol(integer_string.c_str()));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_atol_skips_leading_whitespace_and_sign, "ft_atol trims whitespace before sign")
{
    const char *input_string = " \t\n +0042";

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(42L, ft_atol(input_string));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
