#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

static char *report_strmapi_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return (ft_nullptr);
}

char *ft_strmapi(const char *string, char (*function)(unsigned int, char))
{
    int length_error;

    if (string == ft_nullptr || function == ft_nullptr)
        return (report_strmapi_error(FT_ERR_INVALID_ARGUMENT));
    size_t length = ft_strlen_size_t(string);
    length_error = ft_global_error_stack_pop_newest();
    if (length_error != FT_ERR_SUCCESSS)
    {
        return (report_strmapi_error(length_error));
    }
    char *result = static_cast<char*>(cma_malloc(length + 1));
    if (result == ft_nullptr)
        return (report_strmapi_error(FT_ERR_NO_MEMORY));
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
