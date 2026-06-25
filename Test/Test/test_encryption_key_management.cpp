#include "../test_internal.hpp"
#include "../../Modules/Encryption/encryption.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/RNG/rng.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "compatebility_system_test_hooks.hpp"

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

#include <cerrno>

FT_TEST(test_encryption_fill_secure_buffer_null_buffer_sets_errno)
{
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, encryption_fill_secure_buffer(ft_nullptr, 16));
    return (1);
}

FT_TEST(test_encryption_fill_secure_buffer_zero_length_sets_errno)
{
    unsigned char buffer[1];

    buffer[0] = 0;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, encryption_fill_secure_buffer(buffer, 0));
    return (1);
}

FT_TEST(test_encryption_fill_secure_buffer_populates_random_data)
{
    unsigned char first_buffer[32];
    unsigned char second_buffer[32];
    size_t index;
    int difference;

    if (encryption_fill_secure_buffer(first_buffer, 32) != 0)
        return (0);
    if (encryption_fill_secure_buffer(second_buffer, 32) != 0)
        return (0);
    difference = 0;
    index = 0;
    while (index < 32)
    {
        if (first_buffer[index] != second_buffer[index])
        {
            difference = 1;
            index = 32;
        }
        else
            index++;
    }
    if (difference == 0)
        return (0);
    return (1);
}

FT_TEST(test_encryption_generate_symmetric_key_allocates_random_bytes)
{
    unsigned char *first_key;
    unsigned char *second_key;
    size_t index;
    int difference;

    first_key = encryption_generate_symmetric_key(32);
    if (first_key == ft_nullptr)
        return (0);
    second_key = encryption_generate_symmetric_key(32);
    if (second_key == ft_nullptr)
    {
        encryption_secure_wipe(first_key, 32);
        cma_free(first_key);
        return (0);
    }
    difference = 0;
    index = 0;
    while (index < 32)
    {
        if (first_key[index] != second_key[index])
        {
            difference = 1;
            index = 32;
        }
        else
            index++;
    }
    encryption_secure_wipe(first_key, 32);
    encryption_secure_wipe(second_key, 32);
    cma_free(first_key);
    cma_free(second_key);
    if (difference == 0)
        return (0);
    return (1);
}

FT_TEST(test_encryption_generate_initialization_vector_delegates_to_key_generator)
{
    unsigned char *iv_buffer;
    size_t index;
    int non_zero;

    iv_buffer = encryption_generate_initialization_vector(16);
    if (iv_buffer == ft_nullptr)
        return (0);
    non_zero = 0;
    index = 0;
    while (index < 16)
    {
        if (iv_buffer[index] != 0)
        {
            non_zero = 1;
            index = 16;
        }
        else
            index++;
    }
    encryption_secure_wipe(iv_buffer, 16);
    cma_free(iv_buffer);
    if (non_zero == 0)
        return (0);
    return (1);
}

#if !defined(_WIN32) && !defined(_WIN64)
FT_TEST(test_encryption_generate_symmetric_key_propagates_rng_failures)
{
    unsigned char *key_buffer;

    cmp_clear_force_rng_failures();
    cmp_force_rng_read_failure(EIO);
    key_buffer = encryption_generate_symmetric_key(16);
    cmp_clear_force_rng_failures();
    if (key_buffer != ft_nullptr)
    {
        encryption_secure_wipe(key_buffer, 16);
        cma_free(key_buffer);
        return (0);
    }
    return (1);
}
#endif
