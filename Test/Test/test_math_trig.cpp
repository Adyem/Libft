#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

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

FT_TEST(test_ft_sin_success_resets_errno, "ft_sin clears ft_errno after prior failure")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_cos_zero, "math_cos returns one for zero input")
{
    double result;

    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    return (1);
}

FT_TEST(test_math_cos_success_resets_errno, "math_cos clears ft_errno after prior failure")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_cos_pi_returns_negative_one, "math_cos returns minus one at pi radians")
{
    double angle;
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    angle = math_deg2rad(180.0);
    result = math_cos(angle);
    FT_ASSERT(math_fabs(result + 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_zero, "ft_tan returns zero for zero input")
{
    double result;

    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_forty_five, "ft_tan returns one for forty five degrees")
{
    double result;

    result = ft_tan(math_deg2rad(45.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_near_pi_over_two_sets_errno, "ft_tan near pi over two reports FT_ERR_INVALID_ARGUMENT")
{
    double result;
    double angle;

    angle = math_deg2rad(90.0);
    ft_errno = FT_ER_SUCCESSS;
    result = ft_tan(angle);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_recovers_after_invalid_argument, "ft_tan clears errno after handling asymptote")
{
    double result;

    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT(math_isnan(ft_tan(math_deg2rad(90.0))));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
