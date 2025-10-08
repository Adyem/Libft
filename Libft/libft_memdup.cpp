#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

void *ft_memdup(const void *source, size_t size)
{
    ft_errno = ER_SUCCESS;
    if (size == 0)
    {
        void *duplicate_zero;

        duplicate_zero = cma_malloc(0);
        if (duplicate_zero == ft_nullptr)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        return (duplicate_zero);
    }
    if (source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    void *duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_memcpy(duplicate, source, size);
    return (duplicate);
}
