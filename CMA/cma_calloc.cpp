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
    int             error_code;

    if (count == 0 || size == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    if (count != 0 && size > SIZE_MAX / count)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    total_size = count * size;
    memory_pointer = cma_malloc(total_size);
    error_code = ft_global_error_stack_drop_last_error();
    if (!memory_pointer)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    character_pointer = static_cast<char*>(memory_pointer);
    index = 0;
    while (index < total_size)
    {
        character_pointer[index] = 0;
        index++;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (memory_pointer);
}
