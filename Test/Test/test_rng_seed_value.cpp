#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_rng_seed_value_hashes_non_null_string)
{
    FT_ASSERT_EQ(ft_hash_string31("entropy"), rng_seed_value("entropy"));
    return (1);
}

FT_TEST(test_rng_seed_value_empty_string_uses_entropy_path)
{
    int64_t seed_value;

    seed_value = rng_seed_value("");
    FT_ASSERT(seed_value != 0);
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

FT_TEST(test_rng_seed_value_ft_string_matches_c_string)
{
    ft_string string_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.assign("entropy", 7));
    FT_ASSERT_EQ(rng_seed_value("entropy"), rng_seed_value(string_value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_rng_seed_value_ft_string_empty_uses_entropy_path)
{
    ft_string string_value;
    int64_t seed_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    seed_value = rng_seed_value(string_value);
    FT_ASSERT(seed_value != 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_rng_seed_value_empty_c_string_uses_entropy_path)
{
    int64_t first_value;
    int64_t second_value;

    first_value = rng_seed_value("");
    second_value = rng_seed_value("");
    FT_ASSERT(first_value != 0);
    FT_ASSERT(second_value != 0);
    return (1);
}
