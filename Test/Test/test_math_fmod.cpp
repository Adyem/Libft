#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

FT_TEST(test_math_fmod_nan_sets_errno, "math_fmod returns nan and sets errno for nan input")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_fmod(math_nan(), 2.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_fmod_infinite_value_sets_errno, "math_fmod returns nan and sets errno for infinite value")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    ft_errno = ER_SUCCESS;
    result = math_fmod(infinite_value, 3.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_fmod_zero_modulus_sets_errno, "math_fmod returns nan and sets errno for zero modulus")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_fmod(4.0, 0.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_math_fmod_infinite_modulus_clears_errno, "math_fmod clears errno when modulus is infinite")
{
    double result;
    double infinite_value;

    infinite_value = std::numeric_limits<double>::infinity();
    ft_errno = FT_EINVAL;
    result = math_fmod(7.0, infinite_value);
    FT_ASSERT_EQ(7.0, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_fmod_success_clears_errno, "math_fmod clears errno on success")
{
    double result;

    ft_errno = FT_EINVAL;
    result = math_fmod(5.5, 2.0);
    FT_ASSERT(math_fabs(result - 1.5) < 0.000001);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
