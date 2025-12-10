#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

FT_TEST(test_math_sqrt_clears_errno, "math_sqrt clears errno on success")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_sqrt(25.0);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT(math_fabs(result - 5.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_sqrt_zero_clears_errno, "math_sqrt zero clears errno")
{
    double result;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    result = math_sqrt(0.0);
    FT_ASSERT_EQ(0.0, result);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_nan_sets_errno, "math_sqrt sets errno for nan input")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_sqrt(math_nan());
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_negative_returns_nan, "math_sqrt returns nan for negative input")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_sqrt(-4.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_positive_infinity, "math_sqrt returns infinity for positive infinity input")
{
    double input;
    double result;

    input = std::numeric_limits<double>::infinity();
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_sqrt(input);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(input, result);
    return (1);
}

FT_TEST(test_math_sqrt_small_positive_input, "math_sqrt returns precise result for small positive input")
{
    double input;
    double result;

    input = 1e-14;
    ft_errno = FT_ERR_OUT_OF_RANGE;
    result = math_sqrt(input);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT(math_fabs(result - 1e-7) < 1e-12);
    return (1);
}

FT_TEST(test_math_sqrt_recovers_after_nan, "math_sqrt clears errno after nan failure")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_sqrt(math_nan());
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    result = math_sqrt(81.0);
    FT_ASSERT(math_fabs(result - 9.0) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
