#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <pthread.h>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"
#include "../System_utils/system_utils.hpp"

void cma_free(void* memory_pointer)
{
    ft_bool lock_acquired = FT_FALSE;

    if (OFFSWITCH == 1)
    {
        std::free(memory_pointer);
        g_cma_free_count++;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", memory_pointer);
        return ;
    }
    if (!memory_pointer)
        return ;
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(memory_pointer))
    {
        cma_backend_deallocate(memory_pointer);
        return ;
    }
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (cma_small_arena_owns_pointer_locked(memory_pointer) == FT_TRUE)
    {
        (void)cma_small_arena_deallocate_locked(memory_pointer);
        cma_unlock_allocator(lock_acquired);
        lock_acquired = FT_FALSE;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", memory_pointer);
        return ;
    }
    Block* block = cma_find_block_for_pointer(memory_pointer);
    if (!block)
    {
        cma_unlock_allocator(lock_acquired);
        lock_acquired = FT_FALSE;
        return ;
    }
    ft_size_t freed_size = 0;
    cma_validate_block(block, "cma_free", memory_pointer);
    if (cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        lock_acquired = FT_FALSE;
        su_sigabrt();
    }
    freed_size = block->size;
    cma_debug_release_allocation(block, "cma_free", memory_pointer);
    cma_mark_block_free(block);
#ifdef LIBFT_TEST_BUILD
    block->leak_ignored = FT_FALSE;
    block->leak_stack_frame_count = 0;
#endif
    block = merge_block(block);
    cma_debug_initialize_block(block);
    Page *page = find_page_of_block(block);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    cma_unlock_allocator(lock_acquired);
    lock_acquired = FT_FALSE;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", memory_pointer);
    return ;
}
