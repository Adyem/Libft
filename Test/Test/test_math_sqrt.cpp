#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_sqrt_basic, "math_sqrt returns correct square root on success")
{
    double result;

    result = math_sqrt(25.0);
    FT_ASSERT(math_fabs(result - 5.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_sqrt_zero, "math_sqrt returns zero for zero input")
{
    double result;

    result = math_sqrt(0.0);
    FT_ASSERT_EQ(0.0, result);
    return (1);
}

FT_TEST(test_math_sqrt_nan_input, "math_sqrt returns nan for nan input")
{
    double result;

    result = math_sqrt(math_nan());
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_sqrt_negative_returns_nan, "math_sqrt returns nan for negative input")
{
    double result;

    result = math_sqrt(-4.0);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_sqrt_positive_infinity, "math_sqrt returns infinity for positive infinity input")
{
    double input;
    double result;

    input = std::numeric_limits<double>::infinity();
    result = math_sqrt(input);
    FT_ASSERT_EQ(input, result);
    return (1);
}

FT_TEST(test_math_sqrt_small_positive_input, "math_sqrt returns precise result for small positive input")
{
    double input;
    double result;

    input = 1e-14;
    result = math_sqrt(input);
    FT_ASSERT(math_fabs(result - 1e-7) < 1e-12);
    return (1);
}

FT_TEST(test_math_sqrt_recovers_after_nan, "math_sqrt succeeds after prior nan failure")
{
    double result;

    result = math_sqrt(math_nan());
    FT_ASSERT(math_isnan(result));
    result = math_sqrt(81.0);
    FT_ASSERT(math_fabs(result - 9.0) < 0.000001);
    return (1);
}
