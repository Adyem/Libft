#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/limits.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_lcm_basic_values, "math_lcm computes least common multiple")
{
    int result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_lcm(21, 6);
    FT_ASSERT_EQ(42, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_with_zero, "math_lcm returns zero when one argument is zero")
{
    long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_lcm(0L, 12L);
    FT_ASSERT_EQ(0L, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_negative_inputs, "math_lcm returns positive result for negative inputs")
{
    long long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_lcm(-15LL, 20LL);
    FT_ASSERT_EQ(60LL, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_int_overflow, "math_lcm detects overflow for int inputs")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_INT_MAX, 2);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_INT_MIN, 1);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_long_overflow, "math_lcm detects overflow for long inputs")
{
    long result;

    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_LONG_MAX, 2L);
    FT_ASSERT_EQ(0L, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_LONG_MIN, 1L);
    FT_ASSERT_EQ(0L, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_long_long_overflow, "math_lcm detects overflow for long long inputs")
{
    long long result;

    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_LLONG_MAX, 2LL);
    FT_ASSERT_EQ(0LL, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = ER_SUCCESS;
    result = math_lcm(FT_LLONG_MIN, 1LL);
    FT_ASSERT_EQ(0LL, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}
