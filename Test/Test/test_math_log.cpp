#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_log_zero_sets_errno, "math_log returns nan and sets errno for zero")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_log(0.0);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_math_log_negative_sets_errno, "math_log returns nan and sets errno for negative values")
{
    double result;

    ft_errno = ER_SUCCESS;
    result = math_log(-4.2);
    FT_ASSERT(math_isnan(result));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
