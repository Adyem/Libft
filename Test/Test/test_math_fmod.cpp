#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_fmod_nan_input, "math_fmod returns nan for nan input")
{
    double result;

    result = math_fmod(math_nan(), 2.0);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_fmod_infinite_value, "math_fmod returns nan for infinite value")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    result = math_fmod(infinite_value, 3.0);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_fmod_zero_modulus, "math_fmod returns nan for zero modulus")
{
    double result;

    result = math_fmod(4.0, 0.0);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_fmod_infinite_modulus, "math_fmod returns dividend when modulus is infinite")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    result = math_fmod(7.0, infinite_value);
    FT_ASSERT_EQ(7.0, result);
    return (1);
}

FT_TEST(test_math_fmod_success, "math_fmod returns expected remainder on success")
{
    double result;

    result = math_fmod(5.5, 2.0);
    FT_ASSERT(math_fabs(result - 1.5) < 0.000001);
    return (1);
}

FT_TEST(test_math_fmod_recovers_after_zero_modulus, "math_fmod succeeds after zero-divisor case")
{
    double result;

    result = math_fmod(3.0, 0.0);
    FT_ASSERT(math_isnan(result));
    result = math_fmod(9.0, 4.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}
