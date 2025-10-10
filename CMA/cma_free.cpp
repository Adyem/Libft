#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <pthread.h>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../Printf/printf.hpp"
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
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    Block* block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
                - sizeof(Block)));
    ft_size_t freed_size = 0;
    if (block->magic != MAGIC_NUMBER)
    {
        unsigned char   *expected_pointer;
        long long        pointer_delta;

        pf_printf_fd(2, "Invalid block detected in cma_free. \n");
        print_block_info(block);
        expected_pointer = reinterpret_cast<unsigned char *>(block)
            + sizeof(Block);
        pointer_delta = reinterpret_cast<unsigned char *>(ptr)
            - expected_pointer;
        pf_printf_fd(2, "Pointer passed to cma_free: %p\n", ptr);
        pf_printf_fd(2, "Pointer offset from user start: %lld bytes\n",
            pointer_delta);
        dump_block_bytes(block);
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        su_sigabrt();
    }
    freed_size = block->size;
    block->free = true;
    block = merge_block(block);
    Page *page = find_page_of_block(block);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    g_cma_free_count++;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", ptr);
    return ;
}
