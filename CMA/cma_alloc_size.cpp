#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

ft_size_t cma_block_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;

    if (memory_pointer == ft_nullptr)
        return (0);
    if (cma_backend_is_enabled())
        return (0);
    if (cma_checked_block_size(memory_pointer, &block_size) != FT_ERR_SUCCESS)
        return (0);
    return (block_size);
}

int32_t cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size)
{
    if (block_size == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    *block_size = 0;
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(memory_pointer))
        return (cma_backend_checked_block_size(memory_pointer, block_size));
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (lock_error);
    }
    Block *block = cma_find_block_for_pointer(memory_pointer);
    if (block == ft_nullptr
        || block->magic != MAGIC_NUMBER_ALLOCATED
        || cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    *block_size = cma_block_user_size(block);
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

ft_size_t cma_alloc_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;

    if (cma_checked_block_size(memory_pointer, &block_size) != FT_ERR_SUCCESS)
        return (0);
    return (block_size);
}
