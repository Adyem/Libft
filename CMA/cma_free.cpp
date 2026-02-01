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
    int error_code = FT_ERR_SUCCESSS;
    bool lock_acquired = false;

    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        g_cma_free_count++;
        error_code = FT_ERR_SUCCESSS;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", ptr);
        cma_record_operation_error(error_code);
        return ;
    }
    if (!ptr)
    {
        cma_record_operation_error(error_code);
        return ;
    }
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(ptr))
    {
        error_code = cma_backend_deallocate(ptr);
        cma_record_operation_error(error_code);
        return ;
    }
    error_code = cma_lock_allocator(&lock_acquired);
    if (error_code != FT_ERR_SUCCESSS)
    {
        cma_record_operation_error(error_code);
        return ;
    }
    Block* block = cma_find_block_for_pointer(ptr);
    if (!block)
    {
        error_code = FT_ERR_INVALID_POINTER;
        cma_unlock_allocator(lock_acquired);
        lock_acquired = false;
        cma_record_operation_error(error_code);
        return ;
    }
    ft_size_t freed_size = 0;
    cma_validate_block(block, "cma_free", ptr);
    if (cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        lock_acquired = false;
        su_sigabrt();
    }
    freed_size = block->size;
    cma_debug_release_allocation(block, "cma_free", ptr);
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
    cma_unlock_allocator(lock_acquired);
    lock_acquired = false;
    error_code = FT_ERR_SUCCESSS;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", ptr);
    cma_record_operation_error(error_code);
    return ;
}
