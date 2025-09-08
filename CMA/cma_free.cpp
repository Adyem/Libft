#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../PThread/pthread_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../Logger/logger.hpp"

void cma_free(void* ptr)
{
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_free %p", ptr);
        return ;
    }
    if (!ptr)
        return ;
    g_malloc_mutex.lock(THREAD_ID);
    Block* block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
                - sizeof(Block)));
    if (block->magic != MAGIC_NUMBER)
    {
        pf_printf_fd(2, "Invalid block detected in cma_free. \n");
        print_block_info(block);
        raise(SIGABRT);
    }
    block->free = true;
    block = merge_block(block);
    Page *page = find_page_of_block(block);
    free_page_if_empty(page);
    g_malloc_mutex.unlock(THREAD_ID);
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_free %p", ptr);
    return ;
}
