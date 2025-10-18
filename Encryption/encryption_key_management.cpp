#include "encryption_key_management.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../RNG/rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int encryption_fill_secure_buffer(unsigned char *buffer, size_t buffer_length)
{
    if (buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (buffer_length == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (rng_secure_bytes(buffer, buffer_length) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}

static unsigned char *encryption_allocate_key(size_t key_length)
{
    unsigned char *key_buffer;

    key_buffer = static_cast<unsigned char *>(cma_malloc(key_length));
    if (key_buffer == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    return (key_buffer);
}

static void encryption_discard_key(unsigned char *key_buffer, size_t key_length, int original_error)
{
    if (key_buffer == ft_nullptr)
        return ;
    if (key_length != 0)
        cmp_secure_memzero(key_buffer, key_length);
    cma_free(key_buffer);
    ft_errno = original_error;
    return ;
}

unsigned char *encryption_generate_symmetric_key(size_t key_length)
{
    unsigned char *key_buffer;

    if (key_length == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    key_buffer = encryption_allocate_key(key_length);
    if (key_buffer == ft_nullptr)
        return (ft_nullptr);
    if (encryption_fill_secure_buffer(key_buffer, key_length) != 0)
    {
        int generation_error;

        generation_error = ft_errno;
        encryption_discard_key(key_buffer, key_length, generation_error);
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (key_buffer);
}

unsigned char *encryption_generate_initialization_vector(size_t iv_length)
{
    return (encryption_generate_symmetric_key(iv_length));
}
