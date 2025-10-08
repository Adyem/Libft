#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"
#include "../Libft/limits.hpp"

void* cma_malloc(ft_size_t size)
{
    if (size > FT_SYSTEM_SIZE_MAX)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (OFFSWITCH == 1)
    {
        void *ptr = malloc(static_cast<size_t>(size));
        if (ptr)
        {
            g_cma_allocation_count++;
            ft_errno = ER_SUCCESS;
        }
        else
            ft_errno = FT_ERR_NO_MEMORY;
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_malloc %llu -> %p",
                static_cast<unsigned long long>(size), ptr);
        return (ptr);
    }
    ft_size_t request_size = size;
    if (size == 0)
        size = 1;
    if (g_cma_alloc_limit != 0 && size > g_cma_alloc_limit)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    ft_size_t aligned_size = align16(size);
    Block *block = find_free_block(aligned_size);
    if (!block)
    {
        Page* page = create_page(aligned_size);
        if (!page)
        {
            int error_code;

            error_code = FT_ERR_NO_MEMORY;
            if (g_cma_thread_safe)
                g_malloc_mutex.unlock(THREAD_ID);
            ft_errno = error_code;
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
    ft_errno = ER_SUCCESS;
    if (ft_log_get_alloc_logging())
    {
        if (request_size == size)
            ft_log_debug("cma_malloc %llu -> %p",
                static_cast<unsigned long long>(aligned_size), result);
        else
            ft_log_debug("cma_malloc %llu (rounded to %llu) -> %p",
                static_cast<unsigned long long>(request_size),
                static_cast<unsigned long long>(aligned_size), result);
    }
    return (result);
}
