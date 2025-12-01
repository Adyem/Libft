#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_average_even_ints, "math_average handles even integers")
{
    int result;

    result = math_average(10, 4);
    FT_ASSERT_EQ(7, result);
    return (1);
}

FT_TEST(test_math_average_odd_ints_truncate, "math_average truncates fractional part toward zero")
{
    int result;

    result = math_average(7, 8);
    FT_ASSERT_EQ(7, result);
    result = math_average(-5, 4);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_average_long_values, "math_average works for long inputs")
{
    long result;

    result = math_average(2147483646L, 2147483647L);
    FT_ASSERT_EQ(2147483646L, result);
    result = math_average(-2147483647L, -2147483648L);
    FT_ASSERT_EQ(-2147483647L, result);
    return (1);
}

FT_TEST(test_math_average_double_values, "math_average averages doubles")
{
    double result;

    result = math_average(5.0, -3.0);
    FT_ASSERT_EQ(1.0, result);
    return (1);
}

FT_TEST(test_math_average_balances_large_opposites, "math_average cancels symmetric extremes")
{
    int result_int;
    long result_long;

    result_int = math_average(1000000000, -1000000000);
    result_long = math_average(5000000000L, -5000000000L);
    FT_ASSERT_EQ(0, result_int);
    FT_ASSERT_EQ(0L, result_long);
    return (1);
}
