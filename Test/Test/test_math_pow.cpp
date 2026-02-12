#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>
#include <cfloat>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_pow_positive_exponent, "math_pow handles positive exponents and clears errno")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_pow(2.0, 10);
    FT_ASSERT(math_fabs(result - 1024.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_zero_exponent_returns_one, "math_pow returns one for zero exponent")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_pow(-3.5, 0);
    FT_ASSERT(math_fabs(result - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_negative_exponent_inverts_base, "math_pow handles negative exponents")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_pow(4.0, -2);
    FT_ASSERT(math_fabs(result - 0.0625) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_zero_base_negative_exponent_sets_errno, "math_pow rejects zero base with negative exponent")
{
    double result;

    ft_errno = FT_ERR_SUCCESS;
    result = math_pow(0.0, -1);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_handles_int_min_exponent, "math_pow handles the smallest integer exponent")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_pow(2.0, INT_MIN);
    FT_ASSERT(result < DBL_MIN);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_unity_base_stable, "math_pow preserves unity base for varied exponents")
{
    double positive_exponent_result;
    double negative_exponent_result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    positive_exponent_result = math_pow(1.0, 12345);
    FT_ASSERT(math_fabs(positive_exponent_result - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    negative_exponent_result = math_pow(1.0, -9876);
    FT_ASSERT(math_fabs(negative_exponent_result - 1.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_pow_negative_base_odd_exponent, "math_pow preserves sign for odd exponents")
{
    double result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_pow(-2.0, 3);
    FT_ASSERT(math_fabs(result + 8.0) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}
