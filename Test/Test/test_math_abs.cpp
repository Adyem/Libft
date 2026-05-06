#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_abs_positive_value_returns_same)
{
    int result;

    result = math_abs(84);
    FT_ASSERT_EQ(84, result);
    return (1);
}

FT_TEST(test_math_abs_negative_value_returns_positive)
{
    int result;

    result = math_abs(-57);
    FT_ASSERT_EQ(57, result);
    return (1);
}

FT_TEST(test_math_abs_long_min_value_clamps_to_max)
{
    long result;

    result = math_abs(static_cast<long>(FT_LLONG_MIN));
    FT_ASSERT_EQ(static_cast<long>(FT_LLONG_MAX), result);
    return (1);
}

FT_TEST(test_math_abs_long_long_min_value_clamps_to_max)
{
    long long result;

    result = math_abs(FT_LLONG_MIN);
    FT_ASSERT_EQ(FT_LLONG_MAX, result);
    return (1);
}

FT_TEST(test_math_abs_zero_is_neutral)
{
    int result;

    result = math_abs(0);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_abs_int_min_clamps_to_max)
{
    int result;

    result = math_abs(FT_INT32_MIN);
    FT_ASSERT_EQ(FT_INT32_MAX, result);
    return (1);
}
