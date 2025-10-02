#include "../../Math/math.hpp"
#include "../../Libft/limits.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_abs_positive_value_returns_same, "math_abs returns positive values unchanged")
{
    int result;

    result = math_abs(84);
    FT_ASSERT_EQ(84, result);
    return (1);
}

FT_TEST(test_math_abs_negative_value_returns_positive, "math_abs negates negative integers")
{
    int result;

    result = math_abs(-57);
    FT_ASSERT_EQ(57, result);
    return (1);
}

FT_TEST(test_math_abs_long_min_value_clamps_to_max, "math_abs clamps long minimum to maximum")
{
    long result;

    result = math_abs(FT_LONG_MIN);
    FT_ASSERT_EQ(FT_LONG_MAX, result);
    return (1);
}

FT_TEST(test_math_abs_long_long_min_value_clamps_to_max, "math_abs clamps long long minimum to maximum")
{
    long long result;

    result = math_abs(FT_LLONG_MIN);
    FT_ASSERT_EQ(FT_LLONG_MAX, result);
    return (1);
}
