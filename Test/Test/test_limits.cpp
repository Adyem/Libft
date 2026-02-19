#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_system_size_max_matches_std_max,
        "FT_SYSTEM_SIZE_MAX equals std::numeric_limits<size_t>::max")
{
    FT_ASSERT_EQ(static_cast<size_t>(FT_SYSTEM_SIZE_MAX), std::numeric_limits<size_t>::max());
    FT_ASSERT(FT_SYSTEM_SIZE_MAX >= 0);
    return (1);
}

FT_TEST(test_system_size_overflow_addition,
        "adding to FT_SYSTEM_SIZE_MAX wraps around when using modulo arithmetic")
{
    constexpr size_t max_value = std::numeric_limits<size_t>::max();
    size_t wrapped = max_value + 1;
    FT_ASSERT_EQ(wrapped, 0);
    (void)wrapped;
    return (1);
}
