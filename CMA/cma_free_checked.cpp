#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>

int cma_checked_free(void* ptr)
{
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        return (0);
    }
    if (!ptr)
        return (0);
    bool lock_acquired = false;
    int lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
        return (-1);
    Block* found = cma_find_block_for_pointer(ptr);
    if (!found)
    {
        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (-1);
    }
    cma_validate_block(found, "cma_checked_free", ptr);
    if (static_cast<void *>(cma_block_user_pointer(found)) != ptr)
    {
        if (lock_acquired)
            cma_unlock_allocator(lock_acquired);
        return (-1);
    }
    ft_size_t freed_size = found->size;
    cma_debug_release_allocation(found, "cma_checked_free", ptr);
    cma_mark_block_free(found);
    found = merge_block(found);
    cma_debug_initialize_block(found);
    Page *pg = find_page_of_block(found);
    free_page_if_empty(pg);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    if (lock_acquired)
        cma_unlock_allocator(lock_acquired);
    return (0);
}
