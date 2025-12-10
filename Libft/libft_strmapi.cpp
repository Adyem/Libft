#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

char *ft_strmapi(const char *string, char (*function)(unsigned int, char))
{
    ft_errno = FT_ER_SUCCESSS;
    if (string == ft_nullptr || function == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    size_t length = ft_strlen_size_t(string);
    if (ft_errno != FT_ER_SUCCESSS)
        return (ft_nullptr);
    char *result = static_cast<char*>(cma_malloc(length + 1));
    if (result == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    unsigned int index = 0;
    while (index < length)
    {
        result[index] = function(index, string[index]);
        index++;
    }
    result[length] = '\0';
    return (result);
}
