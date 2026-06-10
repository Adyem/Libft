#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_average_even_ints)
{
    int result;

    result = math_average(10, 4);
    FT_ASSERT_EQ(7, result);
    return (1);
}

FT_TEST(test_math_average_odd_ints_truncate)
{
    int result;

    result = math_average(7, 8);
    FT_ASSERT_EQ(7, result);
    result = math_average(-5, 4);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_average_long_values)
{
    const int64_t positive_low = 2147483646LL;
    const int64_t positive_high = 2147483647LL;
    const int64_t negative_high = -2147483647LL;
    const int64_t negative_low = -2147483647LL - 1LL;
    int64_t result;

    result = math_average(positive_low, positive_high);
    FT_ASSERT_EQ(positive_low, result);
    result = math_average(negative_high, negative_low);
    FT_ASSERT_EQ(negative_high, result);
    return (1);
}

FT_TEST(test_math_average_double_values)
{
    double result;

    result = math_average(5.0, -3.0);
    FT_ASSERT_EQ(1.0, result);
    return (1);
}

FT_TEST(test_math_average_balances_large_opposites)
{
    int result_int;
    int64_t result_long;

    result_int = math_average(1000000000, -1000000000);
    result_long = math_average(static_cast<int64_t>(5000000000LL), static_cast<int64_t>(-5000000000LL));
    FT_ASSERT_EQ(0, result_int);
    FT_ASSERT_EQ(0LL, result_long);
    return (1);
}
