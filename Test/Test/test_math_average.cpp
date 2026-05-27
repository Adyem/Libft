#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

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
    int64_t result;

    result = math_average(static_cast<int64_t>(2147483646),
            static_cast<int64_t>(2147483647));
    FT_ASSERT_EQ(static_cast<int64_t>(2147483646), result);
    result = math_average(static_cast<int64_t>(-2147483647),
            -2147483648);
    FT_ASSERT_EQ(static_cast<int64_t>(-2147483647), result);
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
    result_long = math_average(static_cast<int64_t>(5000000000LL),
            static_cast<int64_t>(-5000000000LL));
    FT_ASSERT_EQ(0, result_int);
    FT_ASSERT_EQ(static_cast<int64_t>(0), result_long);
    return (1);
}
