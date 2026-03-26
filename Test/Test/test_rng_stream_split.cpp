#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rng_stream_seed_rejects_null_output)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, rng_stream_seed(42, 7, ft_nullptr));
    return (1);
}

FT_TEST(test_rng_stream_seed_is_deterministic)
{
    uint64_t first_value = 0;
    uint64_t second_value = 0;
    uint64_t different_stream_value = 0;
    if (rng_stream_seed(123456789ULL, 99, &first_value) != 0)
        return (0);
    if (rng_stream_seed(123456789ULL, 99, &second_value) != 0)
        return (0);
    if (rng_stream_seed(123456789ULL, 100, &different_stream_value) != 0)
        return (0);
    if (first_value != second_value)
        return (0);
    if (different_stream_value == first_value)
        return (0);
    return (1);
}

FT_TEST(test_rng_stream_seed_sequence_reproducible)
{
    uint32_t buffer_one[5];
    uint32_t buffer_two[5];
    size_t index = 0;
    if (rng_stream_seed_sequence(777, 21, buffer_one, 5) != 0)
        return (0);
    if (rng_stream_seed_sequence(777, 21, buffer_two, 5) != 0)
        return (0);
    while (index < 5)
    {
        if (buffer_one[index] != buffer_two[index])
            return (0);
        index++;
    }
    return (1);
}

FT_TEST(test_rng_stream_seed_from_string_matches_manual_seed)
{
    uint64_t from_string = 0;
    uint64_t manual_value = 0;
    if (rng_stream_seed_from_string("example-seed", 33, &from_string) != 0)
        return (0);
    uint32_t base_seed = ft_random_seed("example-seed");
    if (rng_stream_seed(static_cast<uint64_t>(base_seed), 33, &manual_value) != 0)
        return (0);
    if (from_string != manual_value)
        return (0);
    return (1);
}

FT_TEST(test_rng_stream_seed_sequence_from_string_rejects_null)
{
    uint32_t buffer[2];
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT,
        rng_stream_seed_sequence_from_string(ft_nullptr, 3, buffer, 2));
    return (1);
}
