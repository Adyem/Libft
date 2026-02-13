#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_fabs_negative_value_returns_positive, "math_fabs converts negative double to positive")
{
    double result;

    result = math_fabs(-123.5);
    FT_ASSERT(result > 0.0);
    FT_ASSERT(math_fabs(result - 123.5) < 0.000001);
    return (1);
}

FT_TEST(test_math_fabs_positive_value_unchanged, "math_fabs preserves positive inputs")
{
    double result;

    result = math_fabs(98.25);
    FT_ASSERT(math_fabs(result - 98.25) < 0.000001);
    return (1);
}

FT_TEST(test_math_fabs_negative_zero_returns_positive_zero, "math_fabs normalizes negative zero")
{
    double result;

    result = math_fabs(-0.0);
    FT_ASSERT(result == 0.0);
    FT_ASSERT(math_signbit(result) == 0);
    return (1);
}

FT_TEST(test_math_fabs_propagates_nan, "math_fabs propagates NaN inputs")
{
    double result;

    result = math_fabs(math_nan());
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_fabs_preserves_infinity, "math_fabs returns infinity unchanged")
{
    double result;

    result = math_fabs(math_infinity());
    FT_ASSERT(math_isinf(result));
    FT_ASSERT(result > 0.0);
    return (1);
}

FT_TEST(test_math_fabs_consistent_after_nan, "math_fabs remains consistent after nan input")
{
    double result;

    result = math_fabs(math_nan());
    FT_ASSERT(math_isnan(result));
    result = math_fabs(-42.0);
    FT_ASSERT(math_fabs(result - 42.0) < 0.000001);
    return (1);
}
