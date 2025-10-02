#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_clamp_within_range, "math_clamp returns value when within range")
{
    int result;

    result = math_clamp(7, 1, 10);
    FT_ASSERT_EQ(7, result);
    return (1);
}

FT_TEST(test_math_clamp_below_minimum, "math_clamp clamps values below minimum")
{
    int result;

    result = math_clamp(-5, -2, 8);
    FT_ASSERT_EQ(-2, result);
    return (1);
}

FT_TEST(test_math_clamp_above_maximum, "math_clamp clamps values above maximum")
{
    int result;

    result = math_clamp(42, -3, 12);
    FT_ASSERT_EQ(12, result);
    return (1);
}

FT_TEST(test_math_clamp_equal_bounds, "math_clamp respects identical bounds")
{
    int result;

    result = math_clamp(15, 15, 15);
    FT_ASSERT_EQ(15, result);
    return (1);
}
