#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_acos_nan_input, "math_acos returns nan for nan input")
{
    double result;

    result = math_acos(math_nan());
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_acos_infinite_input, "math_acos returns nan for infinite input")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    result = math_acos(infinite_value);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_acos_above_one, "math_acos rejects inputs greater than one")
{
    double result;

    result = math_acos(1.5);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_acos_below_negative_one, "math_acos rejects inputs less than negative one")
{
    double result;

    result = math_acos(-1.5);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_acos_success, "math_acos computes expected value on success")
{
    double result;
    double expected;

    expected = 1.0471975511965979;
    result = math_acos(0.5);
    FT_ASSERT(math_fabs(result - expected) < 0.000001);
    return (1);
}

FT_TEST(test_math_acos_tolerance_clamps_slightly_above_one,
        "math_acos treats near-one inputs within tolerance as one")
{
    double result;
    double input_value;

    input_value = 1.0 + 5e-14;
    result = math_acos(input_value);
    FT_ASSERT(math_fabs(result) <= 1e-12);
    return (1);
}

FT_TEST(test_math_acos_tolerance_clamps_slightly_below_negative_one,
        "math_acos treats near-negative-one inputs within tolerance as negative one")
{
    double result;
    double input_value;
    double pi_value;

    input_value = -1.0 - 5e-14;
    pi_value = 3.14159265358979323846;
    result = math_acos(input_value);
    FT_ASSERT(math_fabs(result - pi_value) <= 1e-12);
    return (1);
}

FT_TEST(test_math_acos_tolerance_handles_near_zero,
        "math_acos maps near-zero inputs to half pi within tolerance")
{
    double result;
    double expected;

    expected = 3.14159265358979323846 * 0.5;
    result = math_acos(5e-14);
    FT_ASSERT(math_fabs(result - expected) <= 1e-12);
    return (1);
}

FT_TEST(test_math_acos_value_beyond_tolerance,
        "math_acos rejects inputs just outside the tolerance window")
{
    double result;
    double input_value;

    input_value = 1.0 + 1e-9;
    result = math_acos(input_value);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_acos_recovers_after_invalid_input, "math_acos succeeds after invalid input")
{
    double result;
    double expected;

    result = math_acos(2.0);
    FT_ASSERT(math_isnan(result));
    expected = 3.14159265358979323846 * 0.25;
    result = math_acos(0.7071067811865475);
    FT_ASSERT(math_fabs(result - expected) <= 1e-12);
    return (1);
}
