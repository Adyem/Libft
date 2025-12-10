#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_nan_not_equal_to_itself, "math_nan returns a NaN value")
{
    double value;

    value = math_nan();
    FT_ASSERT(math_isnan(value));
    FT_ASSERT(value != value);
    return (1);
}

FT_TEST(test_math_nan_propagates_through_arithmetic, "math_nan propagates through arithmetic operations")
{
    double value;
    double result;

    value = math_nan();
    result = value + 5.0;
    FT_ASSERT(math_isnan(result));
    FT_ASSERT(value != value);
    return (1);
}

FT_TEST(test_math_nan_resets_errno_to_success, "math_nan resets errno to success")
{
    double value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    value = math_nan();
    FT_ASSERT(math_isnan(value));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_math_indeterminate_from_zero_division, "math_indeterminate creates a NaN from 0 / 0")
{
    double value;

    value = math_indeterminate();
    FT_ASSERT(math_isnan(value));
    FT_ASSERT_EQ(0, math_isinf(value));
    return (1);
}
