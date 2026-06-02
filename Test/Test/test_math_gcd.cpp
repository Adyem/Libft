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

FT_TEST(test_math_gcd_basic_values)
{
    int result;

    result = math_gcd(48, 18);
    FT_ASSERT_EQ(6, result);
    return (1);
}

FT_TEST(test_math_gcd_negative_inputs)
{
    long result;

    result = math_gcd(-42L, -56L);
    FT_ASSERT_EQ(14L, result);
    return (1);
}

FT_TEST(test_math_gcd_zero_arguments)
{
    long long result;

    result = math_gcd(static_cast<int64_t>(0), static_cast<int64_t>(0));
    FT_ASSERT_EQ(0LL, result);
    return (1);
}

FT_TEST(test_math_gcd_one_zero_argument)
{
    FT_ASSERT_EQ(25, math_gcd(0, -25));
    FT_ASSERT_EQ(9L, math_gcd(9L, 0L));
    return (1);
}

FT_TEST(test_math_gcd_mixed_signs_match_positive_result)
{
    long result_first_sign;
    long result_second_sign;

    result_first_sign = math_gcd(-84L, 30L);
    result_second_sign = math_gcd(84L, -30L);
    FT_ASSERT_EQ(result_first_sign, result_second_sign);
    FT_ASSERT_EQ(6L, result_first_sign);
    return (1);
}
