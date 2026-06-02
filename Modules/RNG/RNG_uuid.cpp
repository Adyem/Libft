#include "rng.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"

void ft_generate_uuid(char output_buffer[37])
{
    if (output_buffer == ft_nullptr)
        return ;
    unsigned char uuid_bytes[16];
    if (rng_secure_bytes(uuid_bytes, 16) != FT_ERR_SUCCESS)
    {
        output_buffer[0] = '\0';
        return ;
    }
    uuid_bytes[6] = static_cast<unsigned char>((uuid_bytes[6] & 0x0F) | 0x40);
    uuid_bytes[8] = static_cast<unsigned char>((uuid_bytes[8] & 0x3F) | 0x80);
    const char hex_characters[] = "0123456789abcdef";
    ft_size_t byte_index = 0;
    ft_size_t output_index = 0;
    while (byte_index < 16)
    {
        output_buffer[output_index] = hex_characters[(uuid_bytes[byte_index] >> 4) & 0x0F];
        output_index++;
        output_buffer[output_index] = hex_characters[uuid_bytes[byte_index] & 0x0F];
        output_index++;
        if (byte_index == 3 || byte_index == 5 || byte_index == 7 || byte_index == 9)
        {
            output_buffer[output_index] = '-';
            output_index++;
        }
        byte_index++;
    }
    output_buffer[output_index] = '\0';
    return ;
}
