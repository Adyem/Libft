#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_sin_zero, "ft_sin returns zero for zero input")
{
    double result;

    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_ft_sin_ninety, "ft_sin returns one for ninety degrees")
{
    double result;

    result = ft_sin(math_deg2rad(90.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_sin_success_after_other_calls, "ft_sin remains correct after other calls")
{
    double result;

    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_zero, "math_cos returns one for zero input")
{
    double result;

    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_success, "math_cos returns expected value at zero")
{
    double result;

    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_pi_returns_negative_one, "math_cos returns minus one at pi radians")
{
    double angle;
    double result;

    angle = math_deg2rad(180.0);
    result = math_cos(angle);
    FT_ASSERT(math_fabs(result + 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_zero, "ft_tan returns zero for zero input")
{
    double result;

    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_forty_five, "ft_tan returns one for forty five degrees")
{
    double result;

    result = ft_tan(math_deg2rad(45.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_near_pi_over_two_returns_nan, "ft_tan near pi over two returns nan")
{
    double result;
    double angle;

    angle = math_deg2rad(90.0);
    result = ft_tan(angle);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_ft_tan_recovers_after_invalid_argument, "ft_tan succeeds after asymptote input")
{
    double result;

    FT_ASSERT(math_isnan(ft_tan(math_deg2rad(90.0))));
    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}
