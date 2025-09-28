#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

FT_TEST(test_math_acos_nan_sets_errno, "math_acos returns nan and sets errno for nan input")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_acos(math_nan());
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_infinite_sets_errno, "math_acos returns nan and sets errno for infinite input")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    ft_errno = ER_SUCCESS;
    result = math_acos(infinite_value);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_above_one_sets_errno, "math_acos rejects inputs greater than one")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_acos(1.5);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_below_negative_one_sets_errno, "math_acos rejects inputs less than negative one")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_acos(-1.5);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_acos_success_clears_errno, "math_acos clears errno on success")
{
    double result;
    double expected;

    ft_errno = FT_EINVAL;
    expected = 1.0471975511965979;
    result = math_acos(0.5);
    FT_ASSERT(math_fabs(result - expected) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
