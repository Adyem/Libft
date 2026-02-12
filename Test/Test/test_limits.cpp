#include "../test_internal.hpp"
#include "../../Basic/limits.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>

#ifndef LIBFT_TEST_BUILD
#endif

static_assert(ft_compute_system_size_max() == FT_SYSTEM_SIZE_MAX,
        "ft_compute_system_size_max matches FT_SYSTEM_SIZE_MAX");
static_assert(ft_compute_system_size_max() ==
        static_cast<unsigned long long>(std::numeric_limits<size_t>::max()),
        "ft_compute_system_size_max matches std::numeric_limits<size_t>::max()");

FT_TEST(test_ft_compute_system_size_max_matches_size_t_max,
        "ft_compute_system_size_max matches the platform size_t maximum")
{
    unsigned long long expected_value;
    unsigned long long computed_value;

    expected_value = static_cast<unsigned long long>(
            std::numeric_limits<size_t>::max());
    computed_value = ft_compute_system_size_max();
    FT_ASSERT_EQ(expected_value, computed_value);
    return (1);
}

FT_TEST(test_ft_system_size_max_macro_matches_function,
        "FT_SYSTEM_SIZE_MAX stays in sync with ft_compute_system_size_max")
{
    unsigned long long computed_value;

    computed_value = ft_compute_system_size_max();
    FT_ASSERT_EQ(computed_value, FT_SYSTEM_SIZE_MAX);
    return (1);
}

FT_TEST(test_ft_compute_system_size_max_uses_all_bytes,
        "ft_compute_system_size_max fills each byte with 0xFF")
{
    unsigned long long manual_value;
    size_t byte_index;

    manual_value = 0ULL;
    byte_index = 0;
    while (byte_index < sizeof(size_t))
    {
        manual_value = (manual_value << 8U) | 0xFFULL;
        byte_index++;
    }
    FT_ASSERT_EQ(manual_value, ft_compute_system_size_max());
    return (1);
}
