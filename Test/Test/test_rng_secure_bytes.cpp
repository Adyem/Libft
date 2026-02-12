#include "../test_internal.hpp"
#include "../../RNG/rng.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#include <cerrno>

FT_TEST(test_rng_secure_bytes_null_buffer_sets_ft_einval, "rng_secure_bytes null buffer sets FT_ERR_INVALID_ARGUMENT")
{
    if (rng_secure_bytes(ft_nullptr, 16) != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_success_clears_errno, "rng_secure_bytes success clears ft_errno")
{
    unsigned char buffer[8];
    if (rng_secure_bytes(buffer, 8) != FT_ERR_SUCCESS)
        return (0);
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_rng_secure_bytes_open_failure_propagates_errno, "rng_secure_bytes propagates open failures")
{
    unsigned char buffer[4];
    cmp_clear_force_rng_failures();
    cmp_force_rng_open_failure(EACCES);
    int result = rng_secure_bytes(buffer, 4);
    cmp_clear_force_rng_failures();
    if (result != FT_ERR_INVALID_OPERATION)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_read_failure_propagates_errno, "rng_secure_bytes propagates read failures")
{
    unsigned char buffer[4];
    cmp_clear_force_rng_failures();
    cmp_force_rng_read_failure(EIO);
    int result = rng_secure_bytes(buffer, 4);
    cmp_clear_force_rng_failures();
    if (result != FT_ERR_IO)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_detects_unexpected_eof, "rng_secure_bytes treats unexpected EOF as an error")
{
    unsigned char buffer[4];

    cmp_clear_force_rng_failures();
    cmp_force_rng_read_eof();
    int result = rng_secure_bytes(buffer, 4);
    cmp_clear_force_rng_failures();
    if (result != FT_ERR_IO)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_close_failure_propagates_errno, "rng_secure_bytes propagates close failures")
{
    unsigned char buffer[4];
    cmp_clear_force_rng_failures();
    cmp_force_rng_close_failure(EBADF);
    int result = rng_secure_bytes(buffer, 4);
    cmp_clear_force_rng_failures();
    if (result != FT_ERR_INVALID_HANDLE)
        return (0);
    return (1);
}
#endif
