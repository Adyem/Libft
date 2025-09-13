#include "rng.hpp"
#include "../Compatebility/compatebility_internal.hpp"

int rng_secure_bytes(unsigned char *buffer, size_t length)
{
    if (buffer == ft_nullptr)
        return (-1);
    return (cmp_rng_secure_bytes(buffer, length));
}

uint32_t ft_random_uint32(void)
{
    unsigned char byte_buffer[4];
    if (rng_secure_bytes(byte_buffer, 4) != 0)
        return (0);
    uint32_t random_value = 0;
    size_t index = 0;
    while (index < 4)
    {
        random_value |= static_cast<uint32_t>(byte_buffer[index]) << (index * 8);
        index++;
    }
    return (random_value);
}
