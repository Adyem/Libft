#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>

int ft_strncmp(const char *string_1, const char *string_2, size_t maximum_length)
{
    ft_size_t current_index = 0;
    int error_code;

    if (maximum_length == 0)
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
           current_index < maximum_length)
    {
        unsigned char string_1_char = static_cast<unsigned char>(string_1[current_index]);
        unsigned char string_2_char = static_cast<unsigned char>(string_2[current_index]);
        if (string_1_char != string_2_char)
        {
            error_code = FT_ERR_SUCCESSS;
            ft_global_error_stack_push(error_code);
            return (string_1_char - string_2_char);
        }
        current_index++;
    }
    if (current_index == maximum_length)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (static_cast<unsigned char>(string_1[current_index]) -
        static_cast<unsigned char>(string_2[current_index]));
}
