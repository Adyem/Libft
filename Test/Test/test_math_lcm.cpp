#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_lcm_basic_values, "math_lcm computes least common multiple")
{
    int result;

    ft_errno = FT_EINVAL;
    result = math_lcm(21, 6);
    FT_ASSERT_EQ(42, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_with_zero, "math_lcm returns zero when one argument is zero")
{
    long result;

    ft_errno = FT_EINVAL;
    result = math_lcm(0L, 12L);
    FT_ASSERT_EQ(0L, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_lcm_negative_inputs, "math_lcm returns positive result for negative inputs")
{
    long long result;

    ft_errno = FT_EINVAL;
    result = math_lcm(-15LL, 20LL);
    FT_ASSERT_EQ(60LL, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
