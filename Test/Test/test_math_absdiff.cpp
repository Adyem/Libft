#include "../../Math/math.hpp"
#include "../../Basic/limits.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_absdiff_int_symmetry, "math_absdiff handles int inputs symmetrically")
{
    int forward_difference;
    int reverse_difference;

    forward_difference = math_absdiff(12, -5);
    reverse_difference = math_absdiff(-5, 12);
    FT_ASSERT_EQ(forward_difference, reverse_difference);
    FT_ASSERT_EQ(17, forward_difference);
    return (1);
}

FT_TEST(test_math_absdiff_long_precision, "math_absdiff handles long range values")
{
    long first_number;
    long second_number;
    long difference;

    first_number = 123456789L;
    second_number = -98765432L;
    difference = math_absdiff(first_number, second_number);
    FT_ASSERT_EQ(222222221L, difference);
    return (1);
}

FT_TEST(test_math_absdiff_long_long_large_values, "math_absdiff handles long long inputs")
{
    long long first_number;
    long long second_number;
    long long difference;

    first_number = 922337203685477000LL;
    second_number = 922337203685470000LL;
    difference = math_absdiff(first_number, second_number);
    FT_ASSERT_EQ(7000LL, difference);
    return (1);
}

FT_TEST(test_math_absdiff_double_precision, "math_absdiff handles double inputs")
{
    double difference;

    difference = math_absdiff(-12.5, 3.25);
    FT_ASSERT(math_fabs(difference - 15.75) < 0.000001);
    return (1);
}

FT_TEST(test_math_absdiff_extreme_integers, "math_absdiff prevents overflow for signed integers")
{
    int int_difference;
    long long_difference;
    long long long_long_difference;

    int_difference = math_absdiff(FT_INT_MIN, FT_INT_MAX);
    long_difference = math_absdiff(FT_LONG_MIN, FT_LONG_MAX);
    long_long_difference = math_absdiff(FT_LLONG_MIN, FT_LLONG_MAX);
    FT_ASSERT_EQ(FT_INT_MAX, int_difference);
    FT_ASSERT_EQ(FT_LONG_MAX, long_difference);
    FT_ASSERT_EQ(FT_LLONG_MAX, long_long_difference);
    return (1);
}

FT_TEST(test_math_absdiff_zero_inputs, "math_absdiff returns zero for identical values")
{
    int int_difference;
    long long_difference;

    int_difference = math_absdiff(0, 0);
    long_difference = math_absdiff(-12345L, -12345L);
    FT_ASSERT_EQ(0, int_difference);
    FT_ASSERT_EQ(0L, long_difference);
    return (1);
}
