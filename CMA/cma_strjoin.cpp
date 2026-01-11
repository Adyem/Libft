#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char    *allocate_new_string(const char *string_1, const char *string_2,
                int *error_code)
{
    size_t  total_length;
    char    *new_string;

    total_length = 0;
    if (string_1)
    {
        size_t  string_1_length;

        string_1_length = ft_strlen_size_t(string_1);
        *error_code = ft_global_error_stack_pop_newest();
        if (string_1_length > SIZE_MAX - 1 - total_length)
        {
            *error_code = FT_ERR_OUT_OF_RANGE;
            return (ft_nullptr);
        }
        total_length += string_1_length;
    }
    if (string_2)
    {
        size_t  string_2_length;

        string_2_length = ft_strlen_size_t(string_2);
        *error_code = ft_global_error_stack_pop_newest();
        if (string_2_length > SIZE_MAX - 1 - total_length)
        {
            *error_code = FT_ERR_OUT_OF_RANGE;
            return (ft_nullptr);
        }
        total_length += string_2_length;
    }
    new_string = static_cast<char *>(cma_malloc(total_length + 1));
    *error_code = ft_global_error_stack_pop_newest();
    if (!new_string)
    {
        return (ft_nullptr);
    }
    *error_code = FT_ERR_SUCCESSS;
    return (new_string);
}

char    *cma_strjoin(char const *string_1, char const *string_2)
{
    char    *result;
    size_t  write_index;
    int     error_code;

    if (!string_1 && !string_2)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    result = allocate_new_string(string_1, string_2, &error_code);
    if (!result)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    write_index = 0;
    if (string_1)
        while (string_1[0])
            result[write_index++] = *string_1++;
    if (string_2)
        while (string_2[0])
            result[write_index++] = *string_2++;
    result[write_index] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
