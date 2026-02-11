#include <cstdio>
#include <cinttypes>
#include "../Errno/errno.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int32_t    scma_get_stats(scma_stats *out_stats)
{
    scma_stats stats;
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();

    if (scma_mutex_lock() != 0)
    {
        return (0);
    }
    if (!scma_initialized_ref())
    {
        return (scma_unlock_and_return_int(0));
    }
    if (!out_stats)
    {
        return (scma_unlock_and_return_int(0));
    }
    stats.block_count = block_count;
    stats.used_size = used_size;
    stats.heap_capacity = heap_capacity;
    *out_stats = stats;
    return (scma_unlock_and_return_int(1));
}

void    scma_debug_dump(void)
{
    scma_block_span span;
    ft_size_t index;
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &heap_capacity_ref_value = scma_heap_capacity_ref();
    uint64_t block_count_print;
    uint64_t used_size_print;
    uint64_t heap_capacity_print;

    if (scma_mutex_lock() != 0)
    {
        return ;
    }
    if (!scma_initialized_ref())
    {
        std::printf("[scma] not initialized\n");
        scma_unlock_and_return_void();
        return ;
    }
    span = scma_get_block_span();
    block_count_print = span.count;
    used_size_print = used_size;
    heap_capacity_print = heap_capacity_ref_value;
    std::printf("[scma] blocks=%" PRIu64 " used=%" PRIu64 " capacity=%" PRIu64 "\n",
        block_count_print,
        used_size_print,
        heap_capacity_print);
    index = 0;
    while (index < span.count)
    {
        scma_block *block;
        uint64_t index_print;
        uint64_t offset_print;
        uint64_t size_print;
        uint64_t generation_print;

        block = &span.data[index];
        index_print = index;
        offset_print = block->offset;
        size_print = block->size;
        generation_print = block->generation;
        std::printf("  [%" PRIu64 "] offset=%" PRIu64 " size=%" PRIu64 " in_use=%d generation=%" PRIu64 "\n",
            index_print, offset_print, size_print, block->in_use, generation_print);
        index++;
    }
    scma_unlock_and_return_void();
    return ;
}
