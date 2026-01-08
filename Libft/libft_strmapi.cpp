#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

char *ft_strmapi(const char *string, char (*function)(unsigned int, char))
{
    int length_error;

    if (string == ft_nullptr || function == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    size_t length = ft_strlen_size_t(string);
    length_error = ft_global_error_stack_pop_newest();
    if (length_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(length_error);
        return (ft_nullptr);
    }
    char *result = static_cast<char*>(cma_malloc(length + 1));
    if (result == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    unsigned int index = 0;
    while (index < length)
    {
        result[index] = function(index, string[index]);
        index++;
    }
    result[length] = '\0';
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
