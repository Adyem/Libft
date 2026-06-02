#include "encryption.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../RNG/rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

static int32_t encryption_fill_secure_buffer_internal(uint8_t *buffer,
    ft_size_t buffer_length)
{
    if (buffer == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (buffer_length == 0)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (rng_secure_bytes(buffer, buffer_length) != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

int32_t encryption_fill_secure_buffer(uint8_t *buffer, ft_size_t buffer_length)
{
    return (encryption_fill_secure_buffer_internal(buffer, buffer_length));
}

static uint8_t *encryption_allocate_key(ft_size_t key_length,
    int32_t *error_code)
{
    uint8_t *key_buffer;

    key_buffer = static_cast<uint8_t *>(cma_malloc(key_length));
    if (key_buffer == ft_nullptr)
    {
        if (error_code)
            *error_code = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESS;
    return (key_buffer);
}

static void encryption_discard_key(uint8_t *key_buffer, ft_size_t key_length)
{
    if (key_buffer == ft_nullptr)
        return ;
    if (key_length != 0)
        cmp_secure_memzero(key_buffer, key_length);
    cma_free(key_buffer);
    return ;
}

static uint8_t *encryption_generate_symmetric_key_internal(ft_size_t key_length,
    int32_t *error_code)
{
    uint8_t *key_buffer;
    int32_t allocation_error;

    if (key_length == 0)
    {
        if (error_code)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    key_buffer = encryption_allocate_key(key_length, &allocation_error);
    if (key_buffer == ft_nullptr)
    {
        if (error_code)
            *error_code = allocation_error;
        return (ft_nullptr);
    }
    allocation_error = encryption_fill_secure_buffer_internal(key_buffer, key_length);
    if (allocation_error != FT_ERR_SUCCESS)
    {
        encryption_discard_key(key_buffer, key_length);
        if (error_code)
            *error_code = allocation_error;
        return (ft_nullptr);
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESS;
    return (key_buffer);
}

uint8_t *encryption_generate_symmetric_key(ft_size_t key_length)
{
    uint8_t *key_buffer;

    key_buffer = encryption_generate_symmetric_key_internal(key_length, ft_nullptr);
    return (key_buffer);
}

uint8_t *encryption_generate_initialization_vector(
    ft_size_t initialization_vector_length)
{
    uint8_t *key_buffer;

    key_buffer = encryption_generate_symmetric_key_internal(
            initialization_vector_length, ft_nullptr);
    return (key_buffer);
}
