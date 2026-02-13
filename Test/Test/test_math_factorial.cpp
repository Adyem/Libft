#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_factorial_int_basic, "math_factorial computes factorial for positive int")
{
    int result;

    result = math_factorial(5);
    FT_ASSERT_EQ(120, result);
    return (1);
}

FT_TEST(test_math_factorial_zero_is_one, "math_factorial(0) returns one")
{
    long result;

    result = math_factorial(0L);
    FT_ASSERT_EQ(1L, result);
    return (1);
}

FT_TEST(test_math_factorial_negative_input, "math_factorial rejects negative inputs")
{
    int result;

    result = math_factorial(-3);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_factorial_int_overflow, "math_factorial detects int overflow")
{
    int result;

    result = math_factorial(13);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_factorial_long_large_value, "math_factorial handles large long results without overflow")
{
    long result;

    result = math_factorial(20L);
    FT_ASSERT_EQ(2432902008176640000L, result);
    return (1);
}

FT_TEST(test_math_factorial_long_overflow, "math_factorial detects long overflow")
{
    long result;

    result = math_factorial(21L);
    FT_ASSERT_EQ(0L, result);
    return (1);
}

FT_TEST(test_math_factorial_long_long_large_value, "math_factorial handles long long inputs")
{
    long long result;

    result = math_factorial(20LL);
    FT_ASSERT_EQ(2432902008176640000LL, result);
    return (1);
}

FT_TEST(test_math_factorial_long_long_overflow, "math_factorial detects long long overflow")
{
    long long result;

    result = math_factorial(21LL);
    FT_ASSERT_EQ(0LL, result);
    return (1);
}

FT_TEST(test_math_factorial_recovers_after_invalid_input, "math_factorial succeeds after invalid input")
{
    long result;

    result = math_factorial(-5L);
    FT_ASSERT_EQ(0L, result);
    result = math_factorial(6L);
    FT_ASSERT_EQ(720L, result);
    return (1);
}
