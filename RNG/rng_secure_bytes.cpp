#include "rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int rng_secure_bytes(unsigned char *buffer, size_t length)
{
    if (buffer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int result = cmp_rng_secure_bytes(buffer, length);
    int error_code = ft_global_error_stack_pop_newest();
    if (result == 0 && error_code == FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INTERNAL;
    ft_global_error_stack_push(error_code);
    return (result);
}

uint32_t ft_random_uint32(void)
{
    unsigned char byte_buffer[4];
    if (rng_secure_bytes(byte_buffer, 4) != 0)
    {
        int error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    int error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(error_code);
        return (0);
    }
    uint32_t random_value = 0;
    size_t index = 0;
    while (index < 4)
    {
        random_value |= static_cast<uint32_t>(byte_buffer[index]) << (index * 8);
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (random_value);
}
