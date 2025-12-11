#include "libft.hpp"
#include "libft_utf8.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int ft_utf8_count(const char *string, size_t *code_point_count_pointer)
{
    size_t string_length;
    size_t byte_index;
    size_t decoded_count;

    ft_errno = FT_ERR_SUCCESSS;
    if (string == ft_nullptr || code_point_count_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_FAILURE);
    }
    string_length = ft_strlen_size_t(string);
    byte_index = 0;
    decoded_count = 0;
    while (byte_index < string_length)
    {
        size_t working_index;
        uint32_t code_point_value;
        size_t sequence_length;

        working_index = byte_index;
        code_point_value = 0;
        sequence_length = 0;
        if (ft_utf8_next(string, string_length, &working_index,
                &code_point_value, &sequence_length) != FT_SUCCESS)
            return (FT_FAILURE);
        byte_index = working_index;
        decoded_count++;
    }
    *code_point_count_pointer = decoded_count;
    return (FT_SUCCESS);
}
