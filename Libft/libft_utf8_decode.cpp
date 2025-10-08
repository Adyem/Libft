#include "libft.hpp"
#include "libft_utf8.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int ft_utf8_is_trailing_byte(unsigned char byte_value)
{
    if ((byte_value & 0xC0) == 0x80)
        return (1);
    return (0);
}

static int ft_utf8_detect_sequence(unsigned char first_byte, size_t *expected_length,
        uint32_t *initial_value, uint32_t *minimum_value)
{
    if (first_byte <= 0x7F)
    {
        *expected_length = 1;
        *initial_value = first_byte;
        *minimum_value = 0;
        return (FT_SUCCESS);
    }
    if ((first_byte & 0xE0) == 0xC0)
    {
        *expected_length = 2;
        *initial_value = first_byte & 0x1F;
        *minimum_value = 0x80;
        return (FT_SUCCESS);
    }
    if ((first_byte & 0xF0) == 0xE0)
    {
        *expected_length = 3;
        *initial_value = first_byte & 0x0F;
        *minimum_value = 0x800;
        return (FT_SUCCESS);
    }
    if ((first_byte & 0xF8) == 0xF0)
    {
        *expected_length = 4;
        *initial_value = first_byte & 0x07;
        *minimum_value = 0x10000;
        return (FT_SUCCESS);
    }
    return (FT_FAILURE);
}

int ft_utf8_next(const char *string, size_t string_length,
        size_t *index_pointer, uint32_t *code_point_pointer,
        size_t *sequence_length_pointer)
{
    size_t current_index;
    unsigned char first_byte;
    size_t expected_length;
    uint32_t decoded_value;
    size_t processed_bytes;
    uint32_t minimum_value;

    ft_errno = ER_SUCCESS;
    if (string == ft_nullptr || index_pointer == ft_nullptr
        || code_point_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    if (*index_pointer >= string_length)
        return (FT_FAILURE);
    current_index = *index_pointer;
    first_byte = static_cast<unsigned char>(string[current_index]);
    expected_length = 0;
    decoded_value = 0;
    minimum_value = 0;
    if (ft_utf8_detect_sequence(first_byte, &expected_length,
            &decoded_value, &minimum_value) != FT_SUCCESS)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    if (current_index + expected_length > string_length)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    processed_bytes = 1;
    while (processed_bytes < expected_length)
    {
        unsigned char continuation_byte;

        continuation_byte = static_cast<unsigned char>(string[current_index + processed_bytes]);
        if (!ft_utf8_is_trailing_byte(continuation_byte))
        {
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (FT_FAILURE);
        }
        decoded_value = (decoded_value << 6) | (continuation_byte & 0x3F);
        processed_bytes++;
    }
    if (decoded_value < minimum_value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    if (decoded_value > 0x10FFFF)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    if (decoded_value >= 0xD800 && decoded_value <= 0xDFFF)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    *code_point_pointer = decoded_value;
    if (sequence_length_pointer != ft_nullptr)
        *sequence_length_pointer = expected_length;
    *index_pointer = current_index + expected_length;
    return (FT_SUCCESS);
}
