#include <stddef.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "CMA.hpp"

char    *cma_strdup(const char *string)
{
    size_t  length;
    char    *new_string;
    size_t  index;
    size_t  allocation_size;

    if (!string)
        return (ft_nullptr);
    ft_errno = ER_SUCCESS;
    length = ft_strlen_size_t(string);
    if (ft_errno != ER_SUCCESS)
    {
        ft_errno = FT_ERANGE;
        return (ft_nullptr);
    }
    if (length > SIZE_MAX - 1)
    {
        ft_errno = FT_ERANGE;
        return (ft_nullptr);
    }
    allocation_size = length + 1;
    new_string = static_cast<char *>(cma_malloc(static_cast<ft_size_t>(allocation_size)));
    if (!new_string)
        return (ft_nullptr);
    index = 0;
    while (index < length)
    {
        new_string[index] = string[index];
        index++;
    }
    new_string[index] = '\0';
    return (new_string);
}
