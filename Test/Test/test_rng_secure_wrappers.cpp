#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "compatebility_system_test_hooks.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cerrno>

FT_TEST(test_rng_secure_uint64_null_pointer_sets_error, "rng_secure_uint64 rejects null output")
{
    if (rng_secure_uint64(ft_nullptr) != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_uint32_success_returns_value, "rng_secure_uint32 succeeds and clears errno")
{
    uint32_t value = 0;
    if (rng_secure_uint32(&value) != FT_ERR_SUCCESS)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_with_fallback_zero_length_is_success, "rng_secure_bytes_with_fallback handles zero length")
{
    int32_t fallback_used = 42;
    if (rng_secure_bytes_with_fallback(ft_nullptr, 0, &fallback_used) != FT_ERR_SUCCESS)
        return (0);
    if (fallback_used != 0)
        return (0);
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_rng_secure_uint64_falls_back_when_system_rng_fails, "rng_secure_uint64 uses fallback on failure")
{
    uint64_t value = 0;
    int32_t fallback_used = 0;
    cmp_clear_force_rng_failures();
    cmp_force_rng_open_failure(EACCES);
    if (rng_secure_uint64(&value, &fallback_used) != FT_ERR_SUCCESS)
    {
        cmp_clear_force_rng_failures();
        return (0);
    }
    cmp_clear_force_rng_failures();
    if (fallback_used != 1)
        return (0);
    return (1);
}
#endif

FT_TEST(test_rng_secure_bytes_with_fallback_reports_success, "rng_secure_bytes_with_fallback marks fallback flag")
{
    unsigned char buffer[16];
    int32_t fallback_used = -1;
    if (rng_secure_bytes_with_fallback(buffer, sizeof(buffer), &fallback_used) != FT_ERR_SUCCESS)
        return (0);
    if (fallback_used != 0 && fallback_used != 1)
        return (0);
    return (1);
}
