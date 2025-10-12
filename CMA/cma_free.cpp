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
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        g_cma_free_count++;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", ptr);
        return ;
    }
    if (!ptr)
        return ;
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(ptr))
    {
        cma_backend_deallocate(ptr);
        return ;
    }
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    Block* block = cma_find_block_for_pointer(ptr);
    if (!block)
    {
        int error_code;

        error_code = FT_ERR_INVALID_POINTER;
        ft_errno = FT_ERR_INVALID_POINTER;
        allocator_guard.unlock();
        ft_errno = error_code;
        return ;
    }
    ft_size_t freed_size = 0;
    cma_validate_block(block, "cma_free", ptr);
    if (cma_block_is_free(block))
    {
        allocator_guard.unlock();
        ft_errno = FT_ERR_INVALID_STATE;
        su_sigabrt();
    }
    freed_size = block->size;
    cma_mark_block_free(block);
    block = merge_block(block);
    Page *page = find_page_of_block(block);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    allocator_guard.unlock();
    ft_errno = ER_SUCCESS;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", ptr);
    return ;
}
