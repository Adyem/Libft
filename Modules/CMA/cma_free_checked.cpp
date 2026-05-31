#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include <cstdlib>

int32_t cma_checked_free(void* memory_pointer)
{
    if (OFFSWITCH == 1)
    {
        std::free(memory_pointer);
        return (FT_ERR_SUCCESS);
    }
    if (!memory_pointer)
        return (FT_ERR_SUCCESS);
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (cma_small_arena_owns_pointer_locked(memory_pointer) == FT_TRUE)
    {
        int32_t arena_error = cma_small_arena_deallocate_locked(memory_pointer);

        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (arena_error);
    }
    Block* found = cma_find_block_for_pointer(memory_pointer);
    if (!found)
    {
        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    cma_validate_block(found, "cma_checked_free", memory_pointer);
    if (static_cast<void *>(cma_block_user_pointer(found)) != memory_pointer)
    {
        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_size_t freed_size = found->size;
    cma_debug_release_allocation(found, "cma_checked_free", memory_pointer);
    cma_mark_block_free(found);
    found = merge_block(found);
    cma_debug_initialize_block(found);
    Page *page = find_page_of_block(found);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    if (lock_acquired)
        cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}
