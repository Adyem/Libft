#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_isnan_nan)
{
    double value;

    value = math_nan();
    FT_ASSERT(math_isnan(value));
    return (1);
}

FT_TEST(test_math_isnan_numbers)
{
    FT_ASSERT(!math_isnan(0.0));
    FT_ASSERT(!math_isnan(42.0));
    return (1);
}
