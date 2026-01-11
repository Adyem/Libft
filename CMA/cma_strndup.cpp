#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char    *cma_strndup(const char *string, size_t maximum_length)
{
    size_t  copy_length;
    char    *duplicate;
    size_t  index;
    int     error_code;

    if (string == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    copy_length = ft_strnlen(string, maximum_length);
    duplicate = static_cast<char *>(cma_malloc(copy_length + 1));
    error_code = ft_global_error_stack_pop_newest();
    if (duplicate == ft_nullptr)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    index = 0;
    while (index < copy_length)
    {
        duplicate[index] = string[index];
        index++;
    }
    duplicate[copy_length] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (duplicate);
}
