#include "CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

void* cma_memdup(const void* source, size_t size)
{
    if (size == 0)
    {
        return (cma_malloc(0));
    }
    if (source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    void *duplicate;

    duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
    {
        return (ft_nullptr);
    }
    ft_memcpy(duplicate, source, size);
    return (duplicate);
}
