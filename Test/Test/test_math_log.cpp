#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cmath>
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_log_zero_sets_errno, "math_log returns nan and sets errno for zero")
{
    double result;

    ft_errno = FT_ERR_SUCCESS;
    result = math_log(0.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_log_negative_sets_errno, "math_log returns nan and sets errno for negative values")
{
    double result;

    ft_errno = FT_ERR_SUCCESS;
    result = math_log(-4.2);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_log_subnormal_positive, "math_log matches std::log for subnormal values")
{
    double input;
    double result;
    double expected;
    double difference;
    double tolerance;

    input = std::numeric_limits<double>::denorm_min();
    ft_errno = FT_ERR_TERMINATED;
    result = math_log(input);
    expected = std::log(input);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    difference = math_fabs(result - expected);
    tolerance = 0.000000001;
    FT_ASSERT(difference < tolerance);
    return (1);
}

FT_TEST(test_math_log_positive_clears_errno, "math_log clears errno for positive inputs")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_log(8.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT(math_fabs(result - std::log(8.0)) < 0.000000001);
    return (1);
}

FT_TEST(test_math_log_unity_returns_zero, "math_log of one returns zero without errno changes")
{
    double result;
    double difference;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_log(1.0);
    difference = math_fabs(result - 0.0);
    FT_ASSERT(difference < 0.0000000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_log_recovers_after_invalid_input, "math_log clears errno after rejecting negative value")
{
    double result;

    ft_errno = FT_ERR_SUCCESS;
    FT_ASSERT(math_isnan(math_log(-2.0)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_log(10.0);
    FT_ASSERT(math_fabs(result - std::log(10.0)) < 0.000000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}
