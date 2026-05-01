#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rng_seed_value_hashes_non_null_string)
{
    FT_ASSERT_EQ(ft_hash_string31("entropy"), rng_seed_value("entropy"));
    return (1);
}

FT_TEST(test_rng_seed_value_empty_string_uses_hash_path)
{
    FT_ASSERT_EQ(ft_hash_string31(""), rng_seed_value(""));
    return (1);
}

FT_TEST(test_rng_seed_value_distinguishes_different_strings)
{
    FT_ASSERT(rng_seed_value("alpha") != rng_seed_value("alphb"));
    return (1);
}

FT_TEST(test_rng_seed_value_null_uses_entropy_path)
{
    (void)rng_seed_value(ft_nullptr);
    (void)rng_seed_value(ft_nullptr);
    return (1);
}

FT_TEST(test_rng_seed_value_string_path_is_deterministic)
{
    int64_t first_value;
    int64_t second_value;

    first_value = rng_seed_value("world_seed");
    second_value = rng_seed_value("world_seed");
    FT_ASSERT_EQ(first_value, second_value);
    return (1);
}
