#include "encryption_key_management.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../RNG/rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"

static int encryption_key_report(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

static unsigned char *encryption_key_pointer_report(int error_code, unsigned char *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

static int encryption_fill_secure_buffer_internal(unsigned char *buffer, size_t buffer_length)
{
    if (buffer == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (buffer_length == 0)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (rng_secure_bytes(buffer, buffer_length) != 0)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESSS);
}

int encryption_fill_secure_buffer(unsigned char *buffer, size_t buffer_length)
{
    int error_code;
    int return_value;

    error_code = encryption_fill_secure_buffer_internal(buffer, buffer_length);
    if (error_code == FT_ERR_SUCCESSS)
        return_value = 0;
    else
        return_value = -1;
    return (encryption_key_report(error_code, return_value));
}

static unsigned char *encryption_allocate_key(size_t key_length, int *error_code)
{
    unsigned char *key_buffer;

    key_buffer = static_cast<unsigned char *>(cma_malloc(key_length));
    if (key_buffer == ft_nullptr)
    {
        if (error_code)
            *error_code = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (key_buffer);
}

static void encryption_discard_key(unsigned char *key_buffer, size_t key_length)
{
    if (key_buffer == ft_nullptr)
        return ;
    if (key_length != 0)
        cmp_secure_memzero(key_buffer, key_length);
    cma_free(key_buffer);
    return ;
}

static unsigned char *encryption_generate_symmetric_key_internal(size_t key_length, int *error_code)
{
    unsigned char *key_buffer;
    int allocation_error;

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
    if (allocation_error != FT_ERR_SUCCESSS)
    {
        encryption_discard_key(key_buffer, key_length);
        if (error_code)
            *error_code = allocation_error;
        return (ft_nullptr);
    }
    if (error_code)
        *error_code = FT_ERR_SUCCESSS;
    return (key_buffer);
}

unsigned char *encryption_generate_symmetric_key(size_t key_length)
{
    unsigned char *key_buffer;
    int error_code;

    key_buffer = encryption_generate_symmetric_key_internal(key_length, &error_code);
    return (encryption_key_pointer_report(error_code, key_buffer));
}

unsigned char *encryption_generate_initialization_vector(size_t iv_length)
{
    unsigned char *key_buffer;
    int error_code;

    key_buffer = encryption_generate_symmetric_key_internal(iv_length, &error_code);
    return (encryption_key_pointer_report(error_code, key_buffer));
}
