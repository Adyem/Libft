#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_sin_zero)
{
    double result;

    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_ft_sin_ninety)
{
    double result;

    result = ft_sin(math_deg2rad(90.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_sin_success_after_other_calls)
{
    double result;

    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_zero)
{
    double result;

    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_success)
{
    double result;

    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_pi_returns_negative_one)
{
    double angle;
    double result;

    angle = math_deg2rad(180.0);
    result = math_cos(angle);
    FT_ASSERT(math_fabs(result + 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_zero)
{
    double result;

    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_forty_five)
{
    double result;

    result = ft_tan(math_deg2rad(45.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_ft_tan_near_pi_over_two_returns_nan)
{
    double result;
    double angle;

    angle = math_deg2rad(90.0);
    result = ft_tan(angle);
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_ft_tan_recovers_after_invalid_argument)
{
    double result;

    FT_ASSERT(math_isnan(ft_tan(math_deg2rad(90.0))));
    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    return (1);
}
