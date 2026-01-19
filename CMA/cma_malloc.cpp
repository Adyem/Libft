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
#include "../Libft/limits.hpp"
#include "../System_utils/system_utils.hpp"

void* cma_malloc(ft_size_t size)
{
    int error_code = 0;

    if (size > FT_SYSTEM_SIZE_MAX)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        cma_record_operation_error(error_code);
        return (ft_nullptr);
    }
    ft_size_t request_size = size;
    if (size == 0)
        size = 1;
    if (g_cma_alloc_limit != 0 && size > g_cma_alloc_limit)
    {
        error_code = FT_ERR_NO_MEMORY;
        cma_record_operation_error(error_code);
        return (ft_nullptr);
    }
    if (cma_backend_is_enabled())
    {
        void *memory_pointer;

        memory_pointer = cma_backend_allocate(size, &error_code);
        cma_record_operation_error(error_code);
        return (memory_pointer);
    }
    if (OFFSWITCH == 1)
    {
        void *memory_pointer = malloc(static_cast<size_t>(size));

        if (memory_pointer)
        {
            g_cma_allocation_count++;
            error_code = FT_ERR_SUCCESSS;
        }
        else
            error_code = FT_ERR_NO_MEMORY;
        if (ft_log_get_alloc_logging())
        ft_log_debug("cma_malloc %llu -> %p", size, memory_pointer);
        cma_record_operation_error(error_code);
        return (memory_pointer);
    }
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
    {
        error_code = allocator_guard.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_STATE;
        cma_record_operation_error(error_code);
        return (ft_nullptr);
    }
    ft_size_t instrumented_size = cma_debug_allocation_size(size);
    ft_size_t aligned_size = align16(instrumented_size);
    Block *block = find_free_block(aligned_size);
    if (!block)
    {
        Page* page = create_page(aligned_size);

        if (!page)
        {
            error_code = FT_ERR_NO_MEMORY;
            allocator_guard.unlock();
        cma_record_operation_error(error_code);
        return (ft_nullptr);
    }
        block = page->blocks;
    }
    cma_validate_block(block, "cma_malloc", ft_nullptr);
    if (!cma_block_is_free(block))
    {
        allocator_guard.unlock();
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
    void *result = static_cast<void *>(cma_block_user_pointer(block));
    cma_leak_tracker_record_allocation(result, cma_block_user_size(block));
    allocator_guard.unlock();
    error_code = FT_ERR_SUCCESSS;
    if (ft_log_get_alloc_logging())
    {
        if (request_size == size)
            ft_log_debug("cma_malloc %llu -> %p", aligned_size, result);
        else
            ft_log_debug("cma_malloc %llu (rounded to %llu) -> %p",
                request_size, aligned_size, result);
    }
    cma_record_operation_error(error_code);
    return (result);
}
