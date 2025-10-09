#include <cstddef>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

void *cma_aligned_alloc(ft_size_t alignment, ft_size_t size)
{
    if ((alignment & (alignment - 1)) != 0
        || alignment < static_cast<ft_size_t>(sizeof(void *)))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
    if (g_cma_alloc_limit != 0 && aligned_size > g_cma_alloc_limit)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (cma_backend_is_enabled())
        return (cma_backend_aligned_allocate(alignment, aligned_size));
    return (cma_malloc(aligned_size));
}
