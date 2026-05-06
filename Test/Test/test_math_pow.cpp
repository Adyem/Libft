#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <climits>
#include <cfloat>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_pow_positive_exponent)
{
    double result;

    result = math_pow(2.0, 10);
    FT_ASSERT(math_fabs(result - 1024.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_pow_zero_exponent_returns_one)
{
    double result;

    result = math_pow(-3.5, 0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_pow_negative_exponent_inverts_base)
{
    double result;

    result = math_pow(4.0, -2);
    FT_ASSERT(math_fabs(result - 0.0625) < 0.000001);
    return (1);
}

FT_TEST(test_math_pow_zero_base_negative_exponent)
{
    double result;

    result = math_pow(0.0, -1);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_pow_handles_int_min_exponent)
{
    double result;

    result = math_pow(2.0, INT_MIN);
    FT_ASSERT(result < DBL_MIN);
    return (1);
}

FT_TEST(test_math_pow_unity_base_stable)
{
    double positive_exponent_result;
    double negative_exponent_result;

    positive_exponent_result = math_pow(1.0, 12345);
    FT_ASSERT(math_fabs(positive_exponent_result - 1.0) < 0.000001);
    negative_exponent_result = math_pow(1.0, -9876);
    FT_ASSERT(math_fabs(negative_exponent_result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_pow_negative_base_odd_exponent)
{
    double result;

    result = math_pow(-2.0, 3);
    FT_ASSERT(math_fabs(result + 8.0) < 0.000001);
    return (1);
}
