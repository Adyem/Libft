#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* cma_memdup(const void* source, size_t size)
{
	int error_code = 0;
    if (size == 0)
    {
        void *memory_pointer;
        
        memory_pointer = cma_malloc(0);
        error_code = ft_global_error_stack_pop_newest();
        if (!memory_pointer)
        {
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (memory_pointer);
    }
    if (source == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    void *duplicate;

    duplicate = cma_malloc(size);
    error_code = ft_global_error_stack_pop_newest();
    if (duplicate == ft_nullptr)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    ft_memcpy(duplicate, source, size);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (duplicate);
}
