#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_exp_after_log_failure, "math_exp remains correct after prior log failure")
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

FT_TEST(test_math_exp_zero_input, "math_exp returns one for zero input")
{
    double result;
    double difference;

    result = math_exp(0.0);
    difference = math_fabs(result - 1.0);
    FT_ASSERT(difference < 0.0000000001);
    return (1);
}

FT_TEST(test_math_exp_negative_input_produces_fraction, "math_exp handles negative exponents")
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
