#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <limits>

#include "../../Modules/Basic/class_nullptr.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_system_size_max_matches_std_max)
{
    FT_ASSERT_EQ(static_cast<size_t>(FT_SYSTEM_SIZE_MAX), std::numeric_limits<size_t>::max());
    FT_ASSERT(FT_SYSTEM_SIZE_MAX >= 0);
    return (1);
}

FT_TEST(test_system_size_overflow_addition)
{
    constexpr size_t max_value = std::numeric_limits<size_t>::max();
    size_t wrapped = max_value + 1;
    FT_ASSERT_EQ(wrapped, 0);
    (void)wrapped;
    return (1);
}
