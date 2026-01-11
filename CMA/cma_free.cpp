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

void cma_free(void* ptr)
{
    int error_code;

    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        g_cma_free_count++;
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", ptr);
        return ;
    }
    if (!ptr)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(ptr))
    {
        cma_backend_deallocate(ptr);
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return ;
    }
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
    {
        error_code = allocator_guard.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return ;
    }
    Block* block = cma_find_block_for_pointer(ptr);
    if (!block)
    {
        error_code = FT_ERR_INVALID_POINTER;
        allocator_guard.unlock();
        ft_global_error_stack_push(error_code);
        return ;
    }
    ft_size_t freed_size = 0;
    cma_validate_block(block, "cma_free", ptr);
    if (cma_block_is_free(block))
    {
        allocator_guard.unlock();
        su_sigabrt();
    }
    freed_size = block->size;
    cma_debug_release_allocation(block, "cma_free", ptr);
    cma_leak_tracker_record_free(ptr);
    cma_mark_block_free(block);
    block = merge_block(block);
    cma_debug_initialize_block(block);
    Page *page = find_page_of_block(block);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    allocator_guard.unlock();
    error_code = FT_ERR_SUCCESSS;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", ptr);
    ft_global_error_stack_push(error_code);
    return ;
}
