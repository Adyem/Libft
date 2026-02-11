#include "rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"

int32_t rng_secure_bytes(unsigned char *buffer, ft_size_t length)
{
    if (buffer == ft_nullptr && length > 0)
        return (FT_ERR_INVALID_ARGUMENT);
    return (cmp_rng_secure_bytes(buffer, length));
}

uint32_t ft_random_uint32(void)
{
    unsigned char byte_buffer[4];
    int32_t error_code;
    uint32_t random_value;
    ft_size_t index;

    error_code = rng_secure_bytes(byte_buffer, 4);
    if (error_code != FT_ERR_SUCCESSS)
        return (0);
    random_value = 0;
    index = 0;
    while (index < 4)
    {
        random_value |= static_cast<uint32_t>(byte_buffer[index]) << (index * 8);
        index++;
    }
    return (random_value);
}
