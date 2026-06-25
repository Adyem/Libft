#include "../test_internal.hpp"
#include "../../Modules/RNG/rng.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "compatebility_system_test_hooks.hpp"

#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

#include <cerrno>

FT_TEST(test_rng_secure_bytes_null_buffer_sets_ft_einval)
{
    if (rng_secure_bytes(ft_nullptr, 16) != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_rng_secure_bytes_success)
{
    unsigned char buffer[8];
    if (rng_secure_bytes(buffer, 8) != FT_ERR_SUCCESS)
        return (0);
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_rng_secure_bytes_open_failure_propagates_errno)
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

FT_TEST(test_rng_secure_bytes_read_failure_propagates_errno)
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

FT_TEST(test_rng_secure_bytes_detects_unexpected_eof)
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

FT_TEST(test_rng_secure_bytes_close_failure_propagates_errno)
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
