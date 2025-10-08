#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_gcd_basic_values, "math_gcd computes gcd for positive integers")
{
    int result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(48, 18);
    FT_ASSERT_EQ(6, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_negative_inputs, "math_gcd ignores signs and returns positive result")
{
    long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(-42L, -56L);
    FT_ASSERT_EQ(14L, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_gcd_zero_arguments, "math_gcd handles zeros and clears errno")
{
    long long result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    result = math_gcd(0LL, 0LL);
    FT_ASSERT_EQ(0LL, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
