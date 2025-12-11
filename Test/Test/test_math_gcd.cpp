#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_gcd_basic_values, "math_gcd computes gcd for positive integers")
{
    int result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(48, 18);
    FT_ASSERT_EQ(6, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_negative_inputs, "math_gcd ignores signs and returns positive result")
{
    long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(-42L, -56L);
    FT_ASSERT_EQ(14L, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_zero_arguments, "math_gcd handles zeros and clears errno")
{
    long long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(0LL, 0LL);
    FT_ASSERT_EQ(0LL, result);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_one_zero_argument, "math_gcd returns absolute value when one argument is zero")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(25, math_gcd(0, -25));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(9L, math_gcd(9L, 0L));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_mixed_signs_match_positive_result, "math_gcd ignores sign ordering")
{
    long result_first_sign;
    long result_second_sign;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result_first_sign = math_gcd(-84L, 30L);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result_second_sign = math_gcd(84L, -30L);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(result_first_sign, result_second_sign);
    FT_ASSERT_EQ(6L, result_first_sign);
    return (1);
}
