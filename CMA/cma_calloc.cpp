#include <cstdint>
#include <stdbool.h>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "CMA.hpp"

void    *cma_calloc(ft_size_t count, ft_size_t size)
{
    void            *memory_pointer;
    ft_size_t        total_size;
    char            *character_pointer;
    ft_size_t        index;

    if (count == 0 || size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (count != 0 && size > SIZE_MAX / count)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    total_size = count * size;
    memory_pointer = cma_malloc(total_size);
    if (!memory_pointer)
        return (ft_nullptr);
    character_pointer = static_cast<char*>(memory_pointer);
    index = 0;
    while (index < total_size)
    {
        character_pointer[index] = 0;
        index++;
    }
    ft_errno = FT_ER_SUCCESSS;
    return (memory_pointer);
}
