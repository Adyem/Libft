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

FT_TEST(test_math_nan_leaves_errno_unchanged, "math_nan does not alter errno state")
{
    double value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    value = math_nan();
    FT_ASSERT(math_isnan(value));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
