#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"

void* cma_malloc(std::size_t size)
{
    if (OFFSWITCH == 1)
    {
        void *ptr = malloc(size);
        if (ptr)
            g_cma_allocation_count++;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_malloc %zu -> %p", size, ptr);
        return (ptr);
    }
    if (size <= 0)
        return (ft_nullptr);
    if (g_cma_alloc_limit != 0 && size > g_cma_alloc_limit)
        return (ft_nullptr);
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    size_t aligned_size = align16(size);
    Block *block = find_free_block(aligned_size);
    if (!block)
    {
        Page* page = create_page(aligned_size);
        if (!page)
        {
            if (g_cma_thread_safe)
                g_malloc_mutex.unlock(THREAD_ID);
            return (ft_nullptr);
        }
        block = page->blocks;
    }
    block = split_block(block, aligned_size);
    block->free = false;
    g_cma_allocation_count++;
    void *result = reinterpret_cast<char*>(block) + sizeof(Block);
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_malloc %zu -> %p", aligned_size, result);
    return (result);
}
