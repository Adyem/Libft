#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

FT_TEST(test_math_acos_nan_sets_errno, "math_acos returns nan and sets errno for nan input")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(math_nan());
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_infinite_sets_errno, "math_acos returns nan and sets errno for infinite input")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(infinite_value);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_above_one_sets_errno, "math_acos rejects inputs greater than one")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(1.5);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_below_negative_one_sets_errno, "math_acos rejects inputs less than negative one")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(-1.5);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_success_clears_errno, "math_acos clears errno on success")
{
    double result;
    double expected;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    expected = 1.0471975511965979;
    result = math_acos(0.5);
    FT_ASSERT(math_fabs(result - expected) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_tolerance_clamps_slightly_above_one,
        "math_acos treats near-one inputs within tolerance as one")
{
    double result;
    double input_value;

    input_value = 1.0 + 5e-14;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_acos(input_value);
    FT_ASSERT(math_fabs(result) <= 1e-12);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_acos(input_value);
    FT_ASSERT(math_fabs(result - pi_value) <= 1e-12);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_tolerance_handles_near_zero,
        "math_acos maps near-zero inputs to half pi within tolerance")
{
    double result;
    double expected;

    expected = 3.14159265358979323846 * 0.5;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_acos(5e-14);
    FT_ASSERT(math_fabs(result - expected) <= 1e-12);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_value_beyond_tolerance_sets_errno,
        "math_acos rejects inputs just outside the tolerance window")
{
    double result;
    double input_value;

    input_value = 1.0 + 1e-9;
    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(input_value);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_recovers_after_invalid_input, "math_acos clears errno after failure")
{
    double result;
    double expected;

    ft_errno = FT_ER_SUCCESSS;
    result = math_acos(2.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    expected = 3.14159265358979323846 * 0.25;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_acos(0.7071067811865475);
    FT_ASSERT(math_fabs(result - expected) <= 1e-12);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
