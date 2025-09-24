#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

void *ft_memdup(const void *source, size_t size)
{
    ft_errno = ER_SUCCESS;
    if (source == ft_nullptr || size == 0)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    void *duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
    {
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    ft_memcpy(duplicate, source, size);
    return (duplicate);
}
