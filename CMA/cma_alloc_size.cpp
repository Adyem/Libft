#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

ft_size_t cma_block_size(const void *memory_pointer)
{
    ft_size_t block_size;

    if (memory_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (cma_backend_is_enabled())
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (0);
    }
    block_size = 0;
    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
        return (0);
    return (block_size);
}

int cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size)
{
    if (block_size == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *block_size = 0;
    if (memory_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(memory_pointer))
        return (cma_backend_checked_block_size(memory_pointer, block_size));
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return (-1);
    Block *block = cma_find_block_for_pointer(memory_pointer);

    if (block == ft_nullptr
        || block->magic != MAGIC_NUMBER_ALLOCATED
        || cma_block_is_free(block))
    {
        int error_code;

        ft_errno = FT_ERR_INVALID_POINTER;
        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (-1);
    }
    *block_size = cma_block_user_size(block);
    ft_errno = FT_ERR_SUCCESSS;
    allocator_guard.unlock();
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

ft_size_t cma_alloc_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;

    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
        return (0);
    return (block_size);
}
