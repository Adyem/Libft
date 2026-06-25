#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <limits>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_signbit_positive_values)
{
    int result;

    result = math_signbit(3.5);
    FT_ASSERT_EQ(0, result);
    result = math_signbit(0.0);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_signbit_negative_zero)
{
    int result;
    double negative_zero;

    negative_zero = -0.0;
    result = math_signbit(negative_zero);
    FT_ASSERT_EQ(1, result);
    return (1);
}

FT_TEST(test_math_signbit_negative_numbers)
{
    int result;
    double negative_infinity;

    negative_infinity = -std::numeric_limits<double>::infinity();
    result = math_signbit(-2.75);
    FT_ASSERT_EQ(1, result);
    result = math_signbit(negative_infinity);
    FT_ASSERT_EQ(1, result);
    return (1);
}

FT_TEST(test_math_signbit_positive_infinity)
{
    int result;
    double positive_infinity;

    positive_infinity = std::numeric_limits<double>::infinity();
    result = math_signbit(positive_infinity);
    FT_ASSERT_EQ(0, result);
    return (1);
}

FT_TEST(test_math_signbit_nan_reports_non_negative)
{
    int result;
    double not_a_number;

    not_a_number = math_nan();
    result = math_signbit(not_a_number);
    FT_ASSERT_EQ(0, result);
    return (1);
}
