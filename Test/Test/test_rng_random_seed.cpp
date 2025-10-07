#include "../../RNG/rng.hpp"
#include "../../RNG/rng_internal.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#if !defined(_WIN32) && !defined(_WIN64)
#include "../Compatebility/compatebility_system_test_hooks.hpp"
#endif
#include <atomic>
#include <cstdint>

FT_TEST(test_ft_random_seed_hashes_string, "ft_random_seed hashes deterministic strings")
{
    const char *seed_string = "entropy";
    uint32_t expected_hash = 2166136261u;
    int index = 0;

    while (seed_string[index] != '\0')
    {
        expected_hash ^= static_cast<unsigned char>(seed_string[index]);
        expected_hash *= 16777619u;
        index = index + 1;
    }
    ft_errno = FT_EINVAL;
    uint32_t computed_hash = ft_random_seed(seed_string);
    FT_ASSERT_EQ(expected_hash, computed_hash);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_random_seed_empty_string_returns_offset_basis, "ft_random_seed returns offset basis for empty input")
{
    const char *seed_string = "";
    ft_errno = FT_EINVAL;
    uint32_t computed_hash = ft_random_seed(seed_string);
    FT_ASSERT_EQ(2166136261u, computed_hash);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_random_seed_null_seed_uses_random_device, "ft_random_seed clears errno when using random_device")
{
    ft_errno = FT_EINVAL;
    (void)ft_random_seed(ft_nullptr);
    if (ft_errno != ER_SUCCESS)
        return (0);
    ft_errno = FT_EINVAL;
    (void)ft_random_seed(ft_nullptr);
    if (ft_errno != ER_SUCCESS)
        return (0);
    return (1);
}

FT_TEST(test_ft_seed_random_engine_with_entropy_sets_flag, "ft_seed_random_engine_with_entropy seeds when needed")
{
    g_random_engine_seeded.store(false, std::memory_order_release);
    ft_errno = FT_EINVAL;
    ft_seed_random_engine_with_entropy();
    if (g_random_engine_seeded.load(std::memory_order_acquire) != true)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    g_random_engine_seeded.store(false, std::memory_order_release);
    return (1);
}

FT_TEST(test_ft_seed_random_engine_with_entropy_skips_when_seeded, "ft_seed_random_engine_with_entropy respects existing seed")
{
    g_random_engine_seeded.store(true, std::memory_order_release);
    ft_errno = FT_EINVAL;
    ft_seed_random_engine_with_entropy();
    if (g_random_engine_seeded.load(std::memory_order_acquire) != true)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    g_random_engine_seeded.store(false, std::memory_order_release);
    return (1);
}

FT_TEST(test_ft_random_uint32_success_clears_errno, "ft_random_uint32 propagates rng_secure_bytes success")
{
    ft_errno = FT_EINVAL;
    (void)ft_random_uint32();
    if (ft_errno != ER_SUCCESS)
        return (0);
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_ft_random_uint32_failure_returns_zero, "ft_random_uint32 returns zero when secure bytes fail")
{
    cmp_clear_force_rng_failures();
    ft_errno = ER_SUCCESS;
    cmp_force_rng_open_failure(EACCES);
    uint32_t random_value = ft_random_uint32();
    cmp_clear_force_rng_failures();
    if (ft_errno != EACCES + ERRNO_OFFSET)
        return (0);
    if (random_value != 0u)
        return (0);
    return (1);
}

FT_TEST(test_ft_random_uint32_eof_returns_zero, "ft_random_uint32 treats unexpected EOF as an error")
{
    cmp_clear_force_rng_failures();
    ft_errno = ER_SUCCESS;
    cmp_force_rng_read_eof();
    uint32_t random_value = ft_random_uint32();
    cmp_clear_force_rng_failures();
    if (ft_errno != FT_EIO)
        return (0);
    if (random_value != 0u)
        return (0);
    return (1);
}
#endif
