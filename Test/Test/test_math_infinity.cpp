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

FT_TEST(test_math_infinity_positive)
{
    double result;

    result = math_infinity();
    FT_ASSERT(math_isinf(result));
    FT_ASSERT(result > 0.0);
    return (1);
}

FT_TEST(test_math_negative_infinity_negative)
{
    double result;

    result = math_negative_infinity();
    FT_ASSERT(math_isinf(result));
    FT_ASSERT(result < 0.0);
    return (1);
}

FT_TEST(test_math_isinf_rejects_non_infinite)
{
    FT_ASSERT_EQ(0, math_isinf(0.0));
    FT_ASSERT_EQ(0, math_isinf(math_nan()));
    FT_ASSERT_EQ(0, math_isinf(123.5));
    return (1);
}
