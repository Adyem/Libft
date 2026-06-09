#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

int32_t ft_isascii(int32_t character)
{
    if (character >= 0 && character <= 0x7F)
        return (1);
    return (0);
}

int32_t ft_utf8_is_leading_byte(int32_t byte_value)
{
    unsigned char unsigned_byte;

    if (byte_value < 0 || byte_value > 0xFF)
        return (0);
    unsigned_byte = static_cast<unsigned char>(byte_value);
    if (unsigned_byte <= 0x7F)
        return (1);
    if (unsigned_byte >= 0xC2 && unsigned_byte <= 0xDF)
        return (1);
    if (unsigned_byte >= 0xE0 && unsigned_byte <= 0xEF)
        return (1);
    if (unsigned_byte >= 0xF0 && unsigned_byte <= 0xF4)
        return (1);
    return (0);
}

int32_t ft_utf8_is_trailing_byte(int32_t byte_value)
{
    unsigned char unsigned_byte;

    if (byte_value < 0 || byte_value > 0xFF)
        return (0);
    unsigned_byte = static_cast<unsigned char>(byte_value);
    if ((unsigned_byte & 0xC0) == 0x80)
        return (1);
    return (0);
}
