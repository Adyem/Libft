#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_factorial_int_basic, "math_factorial computes factorial for positive int")
{
    int result;

    ft_errno = FT_EINVAL;
    result = math_factorial(5);
    FT_ASSERT_EQ(120, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_zero_is_one, "math_factorial(0) returns one and clears errno")
{
    long result;

    ft_errno = FT_EINVAL;
    result = math_factorial(0L);
    FT_ASSERT_EQ(1L, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_negative_sets_errno, "math_factorial rejects negative inputs")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = math_factorial(-3);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_int_overflow_sets_errno, "math_factorial detects int overflow")
{
    int result;

    ft_errno = ER_SUCCESS;
    result = math_factorial(13);
    FT_ASSERT_EQ(0, result);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_long_large_value, "math_factorial handles large long results without overflow")
{
    long result;

    ft_errno = FT_EINVAL;
    result = math_factorial(20L);
    FT_ASSERT_EQ(2432902008176640000L, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_long_overflow_sets_errno, "math_factorial detects long overflow")
{
    long result;

    ft_errno = ER_SUCCESS;
    result = math_factorial(21L);
    FT_ASSERT_EQ(0L, result);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_long_long_large_value, "math_factorial handles long long inputs")
{
    long long result;

    ft_errno = FT_EINVAL;
    result = math_factorial(20LL);
    FT_ASSERT_EQ(2432902008176640000LL, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_factorial_long_long_overflow_sets_errno, "math_factorial detects long long overflow")
{
    long long result;

    ft_errno = ER_SUCCESS;
    result = math_factorial(21LL);
    FT_ASSERT_EQ(0LL, result);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}
