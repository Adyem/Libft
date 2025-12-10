#include <stddef.h>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "CMA.hpp"

char    *cma_strdup(const char *string)
{
    size_t      measured_length_raw;
    ft_size_t   length;
    char        *new_string;
    ft_size_t   index;
    ft_size_t   allocation_size;

    if (!string)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = FT_ER_SUCCESSS;
    measured_length_raw = ft_strlen_size_t(string);
    if (ft_errno != FT_ER_SUCCESSS)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    if (measured_length_raw >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    length = static_cast<ft_size_t>(measured_length_raw);
    allocation_size = length + static_cast<ft_size_t>(1);
    new_string = static_cast<char *>(cma_malloc(allocation_size));
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
