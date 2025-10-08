#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* cma_memdup(const void* src, size_t size)
{
    void *new_mem;

    if (size == 0)
    {
        void *duplicate_zero;

        duplicate_zero = cma_malloc(0);
        return (duplicate_zero);
    }
    if (!src)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    new_mem = cma_malloc(size);
    if (!new_mem)
        return (ft_nullptr);
    ft_memcpy(new_mem, src, size);
    return (new_mem);
}
