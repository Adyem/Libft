#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>

int ft_strncmp(const char *string_1, const char *string_2, size_t max_len)
{
    ft_size_t current_index = 0;

    if (max_len == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (string_1 == ft_nullptr || string_2 == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    while (string_1[current_index] != '\0' &&
           string_2[current_index] != '\0' &&
           current_index < max_len)
    {
        unsigned char string_1_char = static_cast<unsigned char>(string_1[current_index]);
        unsigned char string_2_char = static_cast<unsigned char>(string_2[current_index]);
        if (string_1_char != string_2_char)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (string_1_char - string_2_char);
        }
        current_index++;
    }
    if (current_index == max_len)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (static_cast<unsigned char>(string_1[current_index]) -
           static_cast<unsigned char>(string_2[current_index]));
}
