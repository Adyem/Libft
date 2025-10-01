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

    ft_errno = FT_EINVAL;
    result = ft_sin(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
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

    ft_errno = FT_EINVAL;
    result = math_cos(0.0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_zero, "ft_tan returns zero for zero input")
{
    double result;

    result = ft_tan(0.0);
    FT_ASSERT(math_fabs(result) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_forty_five, "ft_tan returns one for forty five degrees")
{
    double result;

    result = ft_tan(math_deg2rad(45.0));
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_tan_near_pi_over_two_sets_errno, "ft_tan near pi over two reports FT_EINVAL")
{
    double result;
    double angle;

    angle = math_deg2rad(90.0);
    ft_errno = ER_SUCCESS;
    result = ft_tan(angle);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
