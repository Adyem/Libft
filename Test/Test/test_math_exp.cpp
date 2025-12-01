#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_exp_clears_errno_after_previous_failure, "math_exp resets errno to success after previous failure")
{
    double result;
    double difference;
    double expected;

    ft_errno = ER_SUCCESS;
    result = math_log(0.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    expected = 2.71828182845904523536;
    result = math_exp(1.0);
    difference = math_fabs(result - expected);
    FT_ASSERT(difference < 0.0000000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_exp_zero_input, "math_exp returns one for zero input")
{
    double result;
    double difference;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_exp(0.0);
    difference = math_fabs(result - 1.0);
    FT_ASSERT(difference < 0.0000000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_exp_negative_input_produces_fraction, "math_exp handles negative exponents")
{
    double result;
    double expected;
    double difference;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_exp(-2.0);
    expected = 0.13533528323661269189;
    difference = math_fabs(result - expected);
    FT_ASSERT(difference < 0.0000000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
