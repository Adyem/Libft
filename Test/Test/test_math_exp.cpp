#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_exp_after_log_failure)
{
    double result;
    double difference;
    double expected;

    result = math_log(0.0);
    FT_ASSERT(math_isnan(result));
    expected = 2.71828182845904523536;
    result = math_exp(1.0);
    difference = math_fabs(result - expected);
    FT_ASSERT(difference < 0.0000000001);
    return (1);
}

FT_TEST(test_math_exp_zero_input)
{
    double result;
    double difference;

    result = math_exp(0.0);
    difference = math_fabs(result - 1.0);
    FT_ASSERT(difference < 0.0000000001);
    return (1);
}

FT_TEST(test_math_exp_negative_input_produces_fraction)
{
    double result;
    double expected;
    double difference;

    result = math_exp(-2.0);
    expected = 0.13533528323661269189;
    difference = math_fabs(result - expected);
    FT_ASSERT(difference < 0.0000000001);
    return (1);
}
