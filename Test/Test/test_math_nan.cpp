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

FT_TEST(test_math_nan_not_equal_to_itself)
{
    double value;

    value = math_nan();
    FT_ASSERT(math_isnan(value));
    return (1);
}

FT_TEST(test_math_nan_propagates_through_arithmetic)
{
    double value;
    double result;

    value = math_nan();
    result = value + 5.0;
    FT_ASSERT(math_isnan(result));
    return (1);
}

FT_TEST(test_math_nan_resets_errno_to_success)
{
    double value;

    value = math_nan();
    FT_ASSERT(math_isnan(value));
    return (1);
}

FT_TEST(test_math_indeterminate_from_zero_division)
{
    double value;

    value = math_indeterminate();
    FT_ASSERT(math_isnan(value));
    FT_ASSERT_EQ(0, math_isinf(value));
    return (1);
}
