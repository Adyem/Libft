#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_clamp_within_range)
{
    int result;

    result = math_clamp(7, 1, 10);
    FT_ASSERT_EQ(7, result);
    return (1);
}

FT_TEST(test_math_clamp_below_minimum)
{
    int result;

    result = math_clamp(-5, -2, 8);
    FT_ASSERT_EQ(-2, result);
    return (1);
}

FT_TEST(test_math_clamp_above_maximum)
{
    int result;

    result = math_clamp(42, -3, 12);
    FT_ASSERT_EQ(12, result);
    return (1);
}

FT_TEST(test_math_clamp_equal_bounds)
{
    int result;

    result = math_clamp(15, 15, 15);
    FT_ASSERT_EQ(15, result);
    return (1);
}

FT_TEST(test_math_clamp_double_upper_edge)
{
    double result;

    result = math_clamp(4.5, -2.0, 4.5);
    FT_ASSERT(math_fabs(result - 4.5) < 0.000001);
    return (1);
}

FT_TEST(test_math_clamp_double_below_minimum)
{
    double result;

    result = math_clamp(-7.25, -3.5, 9.0);
    FT_ASSERT(math_fabs(result + 3.5) < 0.000001);
    return (1);
}
