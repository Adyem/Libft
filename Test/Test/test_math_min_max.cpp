#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_min_int)
{
    int result;

    result = math_min(5, -3);
    FT_ASSERT_EQ(-3, result);
    return (1);
}

FT_TEST(test_math_min_equal_values_preserve_value)
{
    long result;

    result = math_min(42L, 42L);
    FT_ASSERT_EQ(42L, result);
    return (1);
}

FT_TEST(test_math_min_double_negative_inputs)
{
    double result;

    result = math_min(-1.5, -3.25);
    FT_ASSERT(math_fabs(result - (-3.25)) < 0.000001);
    return (1);
}

FT_TEST(test_math_max_int)
{
    int result;

    result = math_max(5, -3);
    FT_ASSERT_EQ(5, result);
    return (1);
}

FT_TEST(test_math_max_equal_values_preserve_value)
{
    int64_t result;

    result = math_max(static_cast<int64_t>(-18), static_cast<int64_t>(-18));
    FT_ASSERT_EQ(-18LL, result);
    return (1);
}

FT_TEST(test_math_max_double_positive_inputs)
{
    double result;

    result = math_max(7.5, 12.25);
    FT_ASSERT(math_fabs(result - 12.25) < 0.000001);
    return (1);
}

FT_TEST(test_math_max_infinity_dominates)
{
    double result;

    result = math_max(-math_infinity(), 2.5);
    FT_ASSERT(math_fabs(result - 2.5) < 0.000001);
    return (1);
}
