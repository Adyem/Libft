#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

static Block    *cma_find_block_for_pointer(const void *memory_pointer)
{
    Page *current_page = page_list;

    while (current_page)
    {
        Block *current_block = current_page->blocks;

        while (current_block)
        {
            char *data_start = reinterpret_cast<char*>(current_block) + sizeof(Block);

            if (reinterpret_cast<const char*>(memory_pointer) == data_start)
                return (current_block);
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (ft_nullptr);
}

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
    bool lock_acquired;

    lock_acquired = false;
    if (cma_lock_allocator(&lock_acquired) != 0)
        return (-1);
    Block *block = cma_find_block_for_pointer(memory_pointer);

    if (block == ft_nullptr
        || block->magic != MAGIC_NUMBER_ALLOCATED
        || cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    *block_size = block->size;
    cma_unlock_allocator(lock_acquired);
    ft_errno = ER_SUCCESS;
    return (0);
}

ft_size_t cma_alloc_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;

    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
        return (0);
    return (block_size);
}
