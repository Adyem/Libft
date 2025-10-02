#include "../../Math/math.hpp"
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
