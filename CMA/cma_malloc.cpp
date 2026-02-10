#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "../Errno/errno.hpp"
#include "cma_internal.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"
#include "../Basic/limits.hpp"
#include "../System_utils/system_utils.hpp"

void* cma_malloc(ft_size_t size)
{
    void *result = ft_nullptr;
    bool lock_acquired = false;
    ft_size_t request_size = size;
    ft_size_t instrumented_size = 0;
    ft_size_t aligned_size = 0;
    Block *block = ft_nullptr;

    if (size > FT_SYSTEM_SIZE_MAX)
        return (ft_nullptr);
    if (size == 0)
        size = 1;
    if (g_cma_alloc_limit != 0 && size > g_cma_alloc_limit)
        return (ft_nullptr);
    if (cma_backend_is_enabled())
        return (cma_backend_allocate(size, ft_nullptr));
    if (OFFSWITCH == 1)
    {
        result = malloc(static_cast<size_t>(size));

        if (result)
        {
            g_cma_allocation_count++;
        }
        else
            return (ft_nullptr);
        if (ft_log_get_alloc_logging())
            ft_log_debug("cma_malloc %llu -> %p", size, result);
        return (result);
    }
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESSS)
        return (ft_nullptr);
    instrumented_size = cma_debug_allocation_size(size);
    aligned_size = align16(instrumented_size);
    block = find_free_block(aligned_size);
    if (!block)
    {
        Page* page = create_page(aligned_size);

        if (!page)
        {
            if (lock_acquired)
                cma_unlock_allocator(lock_acquired);
            return (ft_nullptr);
        }
        block = page->blocks;
    }
    cma_validate_block(block, "cma_malloc", ft_nullptr);
    if (!cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        lock_acquired = false;
        su_sigabrt();
    }
    block = split_block(block, aligned_size);
    cma_validate_block(block, "cma_malloc split", ft_nullptr);
    cma_mark_block_allocated(block);
    g_cma_allocation_count++;
    g_cma_current_bytes += block->size;
    if (g_cma_current_bytes > g_cma_peak_bytes)
        g_cma_peak_bytes = g_cma_current_bytes;
    cma_debug_prepare_allocation(block, size);
    result = static_cast<void *>(cma_block_user_pointer(block));
    cma_unlock_allocator(lock_acquired);
    lock_acquired = false;
    if (ft_log_get_alloc_logging())
    {
        if (request_size == size)
            ft_log_debug("cma_malloc %llu -> %p", aligned_size, result);
        else
            ft_log_debug("cma_malloc %llu (rounded to %llu) -> %p",
                request_size, aligned_size, result);
    }
    if (lock_acquired)
        cma_unlock_allocator(lock_acquired);
    return (result);
}
