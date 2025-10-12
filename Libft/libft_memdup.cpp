#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static void *allocate_duplicate(size_t size)
{
    void *duplicate;

    duplicate = cma_malloc(size);
    if (duplicate == ft_nullptr)
    {
        ft_errno = FT_ERR_NO_MEMORY;
    }
    return (duplicate);
}

void *ft_memdup(const void *source, size_t size)
{
    void *duplicate;

    ft_errno = ER_SUCCESS;
    if (size == 0)
    {
        return (allocate_duplicate(0));
    }
    if (source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    duplicate = allocate_duplicate(size);
    if (duplicate == ft_nullptr)
    {
        return (ft_nullptr);
    }
    ft_memcpy(duplicate, source, size);
    return (duplicate);
}
