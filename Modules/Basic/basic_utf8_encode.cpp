#include <cstddef>
#include "basic.hpp"
#include "utf8.hpp"
#include "../Errno/errno.hpp"

int32_t ft_utf8_encode(uint32_t code_point, char *buffer, ft_size_t buffer_size,
        ft_size_t *encoded_length_pointer)
{
    ft_size_t required_length;

    if (!buffer && buffer_size != 0)
        return (FT_ERR_INVALID_POINTER);
    if (code_point > 0x10FFFF)
        return (FT_ERR_OUT_OF_RANGE);
    if (code_point >= 0xD800 && code_point <= 0xDFFF)
        return (FT_ERR_INVALID_ARGUMENT);
    if (code_point <= 0x7F)
        required_length = 1;
    else if (code_point <= 0x7FF)
        required_length = 2;
    else if (code_point <= 0xFFFF)
        required_length = 3;
    else
        required_length = 4;
    if (buffer_size <= required_length)
        return (FT_ERR_OUT_OF_RANGE);
    if (required_length == 1)
        buffer[0] = static_cast<char>(code_point);
    else if (required_length == 2)
    {
        buffer[0] = static_cast<char>(0xC0 | ((code_point >> 6) & 0x1F));
        buffer[1] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    else if (required_length == 3)
    {
        buffer[0] = static_cast<char>(0xE0 | ((code_point >> 12) & 0x0F));
        buffer[1] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    else
    {
        buffer[0] = static_cast<char>(0xF0 | ((code_point >> 18) & 0x07));
        buffer[1] = static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (code_point & 0x3F));
    }
    buffer[required_length] = '\0';
    if (encoded_length_pointer)
        *encoded_length_pointer = required_length;
    return (FT_ERR_SUCCESS);
}
