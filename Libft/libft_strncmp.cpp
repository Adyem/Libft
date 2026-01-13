#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <unistd.h>

static int report_strncmp_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int ft_strncmp(const char *string_1, const char *string_2, size_t max_len)
{
    ft_size_t current_index = 0;
    int error_code;

    if (max_len == 0)
        return (report_strncmp_result(FT_ERR_SUCCESSS, 0));
    if (string_1 == ft_nullptr || string_2 == ft_nullptr)
        return (report_strncmp_result(FT_ERR_INVALID_ARGUMENT, -1));
    while (string_1[current_index] != '\0' &&
           string_2[current_index] != '\0' &&
           current_index < max_len)
    {
        unsigned char string_1_char = static_cast<unsigned char>(string_1[current_index]);
        unsigned char string_2_char = static_cast<unsigned char>(string_2[current_index]);
        if (string_1_char != string_2_char)
        {
            error_code = FT_ERR_SUCCESSS;
            return (report_strncmp_result(error_code,
                string_1_char - string_2_char));
        }
        current_index++;
    }
    if (current_index == max_len)
    {
        error_code = FT_ERR_SUCCESSS;
        return (report_strncmp_result(error_code, 0));
    }
    error_code = FT_ERR_SUCCESSS;
    return (report_strncmp_result(error_code,
        static_cast<unsigned char>(string_1[current_index]) -
        static_cast<unsigned char>(string_2[current_index])));
}
