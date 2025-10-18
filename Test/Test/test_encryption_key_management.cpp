#include "../../Encryption/encryption_key_management.hpp"
#include "../../Encryption/encryption_secure_wipe.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../RNG/rng.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../Compatebility/compatebility_system_test_hooks.hpp"

#include <cerrno>

FT_TEST(test_encryption_fill_secure_buffer_null_buffer_sets_errno,
    "encryption_fill_secure_buffer rejects null pointers")
{
    ft_errno = ER_SUCCESS;
    if (encryption_fill_secure_buffer(ft_nullptr, 16) != -1)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_encryption_fill_secure_buffer_zero_length_sets_errno,
    "encryption_fill_secure_buffer rejects zero length")
{
    unsigned char buffer[1];

    buffer[0] = 0;
    ft_errno = ER_SUCCESS;
    if (encryption_fill_secure_buffer(buffer, 0) != -1)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_encryption_fill_secure_buffer_populates_random_data,
    "encryption_fill_secure_buffer fills buffers with random bytes")
{
    unsigned char first_buffer[32];
    unsigned char second_buffer[32];
    size_t index;
    int difference;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    if (encryption_fill_secure_buffer(first_buffer, 32) != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    if (encryption_fill_secure_buffer(second_buffer, 32) != 0)
        return (0);
    if (ft_errno != ER_SUCCESS)
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

FT_TEST(test_encryption_generate_symmetric_key_allocates_random_bytes,
    "encryption_generate_symmetric_key allocates securely populated buffers")
{
    unsigned char *first_key;
    unsigned char *second_key;
    size_t index;
    int difference;

    ft_errno = FT_ERR_IO;
    first_key = encryption_generate_symmetric_key(32);
    if (first_key == ft_nullptr)
        return (0);
    if (ft_errno != ER_SUCCESS)
    {
        encryption_secure_wipe(first_key, 32);
        cma_free(first_key);
        return (0);
    }
    ft_errno = FT_ERR_IO;
    second_key = encryption_generate_symmetric_key(32);
    if (second_key == ft_nullptr)
    {
        encryption_secure_wipe(first_key, 32);
        cma_free(first_key);
        return (0);
    }
    if (ft_errno != ER_SUCCESS)
    {
        encryption_secure_wipe(first_key, 32);
        encryption_secure_wipe(second_key, 32);
        cma_free(first_key);
        cma_free(second_key);
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

FT_TEST(test_encryption_generate_initialization_vector_delegates_to_key_generator,
    "encryption_generate_initialization_vector produces random buffers")
{
    unsigned char *iv_buffer;
    size_t index;
    int non_zero;

    ft_errno = FT_ERR_NO_MEMORY;
    iv_buffer = encryption_generate_initialization_vector(16);
    if (iv_buffer == ft_nullptr)
        return (0);
    if (ft_errno != ER_SUCCESS)
    {
        encryption_secure_wipe(iv_buffer, 16);
        cma_free(iv_buffer);
        return (0);
    }
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
FT_TEST(test_encryption_generate_symmetric_key_propagates_rng_failures,
    "encryption_generate_symmetric_key cleans up when secure bytes fail")
{
    unsigned char *key_buffer;

    cmp_clear_force_rng_failures();
    ft_errno = ER_SUCCESS;
    cmp_force_rng_read_failure(EIO);
    key_buffer = encryption_generate_symmetric_key(16);
    cmp_clear_force_rng_failures();
    if (key_buffer != ft_nullptr)
    {
        encryption_secure_wipe(key_buffer, 16);
        cma_free(key_buffer);
        return (0);
    }
    if (ft_errno != FT_ERR_IO)
        return (0);
    return (1);
}
#endif
