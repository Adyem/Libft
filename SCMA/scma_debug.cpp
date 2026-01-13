#include <cstdio>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int    scma_get_stats(scma_stats *out_stats)
{
    scma_stats stats;
    scma_live_snapshot &snapshot = scma_live_snapshot_ref();
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();
    int error_code;

    if (scma_mutex_lock() != 0)
    {
        error_code = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!scma_initialized_ref())
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (!out_stats)
    {
        error_code = FT_ERR_INVALID_POINTER;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    stats.block_count = block_count;
    stats.used_size = used_size;
    stats.heap_capacity = heap_capacity;
    if (snapshot.active)
        stats.snapshot_active = 1;
    else
        stats.snapshot_active = 0;
    *out_stats = stats;
    error_code = FT_ERR_SUCCESSS;
    ft_errno = error_code;
    ft_global_error_stack_push(error_code);
    return (scma_unlock_and_return_int(1));
}

void    scma_debug_dump(void)
{
    scma_block_span span;
    ft_size_t index;
    size_t heap_capacity;
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &heap_capacity_ref_value = scma_heap_capacity_ref();
    int error_code;

    if (scma_mutex_lock() != 0)
    {
        error_code = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (!scma_initialized_ref())
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        std::printf("[scma] not initialized\n");
        scma_unlock_and_return_void();
        return ;
    }
    span = scma_get_block_span();
    heap_capacity = static_cast<size_t>(heap_capacity_ref_value);
    std::printf("[scma] blocks=%llu used=%llu capacity=%zu\n",
        static_cast<unsigned long long>(span.count),
        static_cast<unsigned long long>(used_size),
        heap_capacity);
    index = 0;
    while (index < span.count)
    {
        scma_block *block;

        block = &span.data[static_cast<size_t>(index)];
        std::printf("  [%llu] offset=%llu size=%llu in_use=%d generation=%llu\n",
            static_cast<unsigned long long>(index),
            static_cast<unsigned long long>(block->offset),
            static_cast<unsigned long long>(block->size),
            block->in_use,
            static_cast<unsigned long long>(block->generation));
        index++;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_errno = error_code;
    ft_global_error_stack_push(error_code);
    scma_unlock_and_return_void();
    return ;
}
