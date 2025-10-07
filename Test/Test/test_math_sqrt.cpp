#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

FT_TEST(test_math_sqrt_clears_errno, "math_sqrt clears errno on success")
{
    double result;

    ft_errno = FT_EINVAL;
    result = math_sqrt(25.0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(math_fabs(result - 5.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_sqrt_zero_clears_errno, "math_sqrt zero clears errno")
{
    double result;

    ft_errno = FT_ERANGE;
    result = math_sqrt(0.0);
    FT_ASSERT_EQ(0.0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_nan_sets_errno, "math_sqrt sets errno for nan input")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_sqrt(math_nan());
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_negative_returns_nan, "math_sqrt returns nan for negative input")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_sqrt(-4.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_sqrt_positive_infinity, "math_sqrt returns infinity for positive infinity input")
{
    double input;
    double result;

    input = std::numeric_limits<double>::infinity();
    ft_errno = FT_EINVAL;
    result = math_sqrt(input);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(input, result);
    return (1);
}
