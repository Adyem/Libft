#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../Basic/limits.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_lcm_basic_values, "math_lcm computes least common multiple")
{
    int result;

    result = math_lcm(21, 6);
    FT_ASSERT_EQ(42, result);
    return (1);
}

FT_TEST(test_math_lcm_with_zero, "math_lcm returns zero when one argument is zero")
{
    long result;

    result = math_lcm(0L, 12L);
    FT_ASSERT_EQ(0L, result);
    return (1);
}

FT_TEST(test_math_lcm_negative_inputs, "math_lcm returns positive result for negative inputs")
{
    long long result;

    result = math_lcm(-15LL, 20LL);
    FT_ASSERT_EQ(60LL, result);
    return (1);
}

FT_TEST(test_math_lcm_int_overflow, "math_lcm returns zero on int overflow")
{
    int result;

    result = math_lcm(FT_INT32_MAX, 2);
    FT_ASSERT_EQ(0, result);
    result = math_lcm(FT_INT32_MIN, 1);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_lcm_long_overflow, "math_lcm returns zero on long overflow")
{
    long result;

    result = math_lcm(static_cast<long>(FT_LLONG_MAX), 2L);
    FT_ASSERT_EQ(0L, result);
    result = math_lcm(static_cast<long>(FT_LLONG_MIN), 1L);
    FT_ASSERT_EQ(0L, result);
    return (1);
}

FT_TEST(test_math_lcm_long_long_overflow, "math_lcm returns zero on long long overflow")
{
    long long result;

    result = math_lcm(static_cast<long long>(FT_LLONG_MAX), 2LL);
    FT_ASSERT_EQ(0LL, result);
    result = math_lcm(static_cast<long long>(FT_LLONG_MIN), 1LL);
    FT_ASSERT_EQ(0LL, result);
    return (1);
}
