#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_rad2deg_converts_positive_angle, "math_rad2deg converts radians to degrees and clears errno")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_rad2deg(3.14159265358979323846 / 3.0);
    FT_ASSERT(math_fabs(result - 60.0) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_rad2deg_converts_negative_angle, "math_rad2deg preserves sign of radians input")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_rad2deg(-3.14159265358979323846 / 4.0);
    FT_ASSERT(math_fabs(result + 45.0) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_deg2rad_converts_full_rotation, "math_deg2rad converts degrees to radians")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_deg2rad(180.0);
    FT_ASSERT(math_fabs(result - 3.14159265358979323846) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_deg2rad_handles_fractional_degrees, "math_deg2rad supports fractional degree inputs")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_deg2rad(22.5);
    FT_ASSERT(math_fabs(result - 0.39269908169872414) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_rad2deg_zero_angle, "math_rad2deg returns zero for zero input and clears errno")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_rad2deg(0.0);
    FT_ASSERT(math_fabs(result - 0.0) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
