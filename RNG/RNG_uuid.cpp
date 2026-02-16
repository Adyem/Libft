#include "rng.hpp"

void ft_generate_uuid(char out[37])
{
    if (out == ft_nullptr)
        return ;
    unsigned char uuid_bytes[16];
    if (rng_secure_bytes(uuid_bytes, 16) != 0)
    {
        out[0] = '\0';
        return ;
    }
    uuid_bytes[6] = static_cast<unsigned char>((uuid_bytes[6] & 0x0F) | 0x40);
    uuid_bytes[8] = static_cast<unsigned char>((uuid_bytes[8] & 0x3F) | 0x80);
    const char hex_characters[] = "0123456789abcdef";
    size_t byte_index = 0;
    size_t output_index = 0;
    while (byte_index < 16)
    {
        out[output_index] = hex_characters[(uuid_bytes[byte_index] >> 4) & 0x0F];
        output_index++;
        out[output_index] = hex_characters[uuid_bytes[byte_index] & 0x0F];
        output_index++;
        if (byte_index == 3 || byte_index == 5 || byte_index == 7 || byte_index == 9)
        {
            out[output_index] = '-';
            output_index++;
        }
        byte_index++;
    }
    out[output_index] = '\0';
    return ;
}
