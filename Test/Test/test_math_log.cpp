#include "../../Math/math.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cmath>
#include <limits>

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

FT_TEST(test_math_log_subnormal_positive, "math_log matches std::log for subnormal values")
{
    double input;
    double result;
    double expected;
    double difference;
    double tolerance;

    input = std::numeric_limits<double>::denorm_min();
    ft_errno = FT_ETERM;
    result = math_log(input);
    expected = std::log(input);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    difference = math_fabs(result - expected);
    tolerance = 0.000000001;
    FT_ASSERT(difference < tolerance);
    return (1);
}
