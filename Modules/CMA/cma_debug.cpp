#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <limits>
#include "cma_internal.hpp"

#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#if DEBUG

static const unsigned char g_cma_guard_pattern = 0xA5;
static const ft_size_t g_cma_guard_size = 32;

ft_size_t cma_debug_guard_size(void)
{
    return (g_cma_guard_size);
}

ft_size_t cma_debug_allocation_size(ft_size_t requested_size)
{
    ft_size_t guard_bytes;
    ft_size_t maximum_value;

    guard_bytes = g_cma_guard_size * 2;
    maximum_value = std::numeric_limits<ft_size_t>::max();
    if (requested_size > maximum_value - guard_bytes)
        return (maximum_value);
    return (requested_size + guard_bytes);
}

void cma_debug_initialize_block(Block *block)
{
    if (!block)
        return ;
#ifdef LIBFT_TEST_BUILD
    block->leak_ignored = FT_FALSE;
    block->leak_stack_frame_count = 0;
#endif
    block->debug_base_pointer = block->payload;
    block->debug_user_size = 0;
    return ;
}

static void cma_debug_abort(void)
{
    su_sigabrt();
    return ;
}

static unsigned char *cma_debug_base_pointer(Block *block)
{
    unsigned char   *base_pointer;

    if (!block)
        return (nullptr);
    base_pointer = block->debug_base_pointer;
    if (!base_pointer)
        base_pointer = block->payload;
    return (base_pointer);
}

static unsigned char *cma_debug_const_base_pointer(const Block *block)
{
    unsigned char   *base_pointer;

    if (!block)
        return (nullptr);
    base_pointer = block->debug_base_pointer;
    if (!base_pointer)
        base_pointer = block->payload;
    return (base_pointer);
}

void cma_debug_prepare_allocation(Block *block, ft_size_t user_size)
{
    unsigned char   *base_pointer;
    unsigned char   *user_pointer;
    unsigned char   *rear_guard_pointer;
    ft_size_t        guard_size;
    ft_size_t        required_size;
    ft_size_t        index;

    if (!block)
        return ;
    base_pointer = cma_debug_base_pointer(block);
    if (!base_pointer)
        cma_debug_abort();
    guard_size = cma_debug_guard_size();
    required_size = user_size;
    required_size += guard_size * 2;
    if (block->size < required_size)
        cma_debug_abort();
    user_pointer = base_pointer + guard_size;
    rear_guard_pointer = user_pointer + user_size;
    block->debug_base_pointer = base_pointer;
    block->debug_user_size = user_size;
    index = 0;
    while (index < guard_size)
    {
        base_pointer[index] = g_cma_guard_pattern;
        index++;
    }
    index = 0;
    while (index < guard_size)
    {
        rear_guard_pointer[index] = g_cma_guard_pattern;
        index++;
    }
    return ;
}

void cma_debug_release_allocation(Block *block, const char *context,
        const void *user_pointer)
{
    unsigned char   *base_pointer;
    unsigned char   *user_area_pointer;
    unsigned char   *rear_guard_pointer;
    ft_size_t        guard_size;
    ft_size_t        index;
    ft_size_t        wipe_index;

    (void)context;
    (void)user_pointer;
    if (!block)
        return ;
    guard_size = cma_debug_guard_size();
    base_pointer = cma_debug_base_pointer(block);
    if (!base_pointer)
        return ;
    user_area_pointer = base_pointer + guard_size;
    rear_guard_pointer = user_area_pointer + block->debug_user_size;
    index = 0;
    while (index < guard_size)
    {
        if (base_pointer[index] != g_cma_guard_pattern)
            cma_debug_abort();
        if (rear_guard_pointer[index] != g_cma_guard_pattern)
            cma_debug_abort();
        index++;
    }
    wipe_index = 0;
    while (wipe_index < block->debug_user_size)
    {
        user_area_pointer[wipe_index] = 0xDD;
        wipe_index++;
    }
    block->debug_user_size = 0;
    index = 0;
    while (index < guard_size)
    {
        base_pointer[index] = 0xDE;
        rear_guard_pointer[index] = 0xDE;
        index++;
    }
    return ;
}

unsigned char *cma_debug_user_pointer(const Block *block)
{
    unsigned char   *base_pointer;

    base_pointer = cma_debug_const_base_pointer(block);
    if (!base_pointer)
        return (nullptr);
    return (base_pointer + cma_debug_guard_size());
}

ft_size_t cma_debug_user_size(const Block *block)
{
    if (!block)
        return (0);
    return (block->debug_user_size);
}

#endif

#ifdef LIBFT_TEST_BUILD
void    cma_capture_leak_stack(Block *block, ft_size_t skip_count)
{
    if (block == nullptr)
        return ;
    block->leak_stack_frame_count = cmp_stack_trace_capture(
            block->leak_stack_frames,
            CMP_STACK_TRACE_MAX_FRAMES,
            skip_count);
    return ;
}

static Block *cma_next_live_block(Block *block)
{
    while (block != nullptr)
    {
        if (cma_block_is_free(block) == FT_FALSE)
            return (block);
        block = block->next;
    }
    return (nullptr);
}

int32_t cma_get_leak_summary(cma_leak_summary *out_summary)
{
    ft_bool lock_acquired;
    Page *page;
    cma_leak_summary summary;

    if (out_summary == nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    summary.live_block_count = 0;
    summary.live_bytes = 0;
    summary.ignored_block_count = 0;
    summary.ignored_bytes = 0;
    page = page_list;
    while (page != nullptr)
    {
        Block *block;

        block = cma_next_live_block(page->blocks);
        while (block != nullptr)
        {
            if (block->leak_ignored == FT_TRUE)
            {
                summary.ignored_block_count += 1;
                summary.ignored_bytes += cma_block_user_size(block);
            }
            else
            {
                summary.live_block_count += 1;
                summary.live_bytes += cma_block_user_size(block);
            }
            block = cma_next_live_block(block->next);
        }
        page = page->next;
    }
    *out_summary = summary;
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t cma_get_leak_entries(cma_leak_entry *entries, ft_size_t capacity,
    ft_size_t *entry_count)
{
    ft_bool lock_acquired;
    ft_size_t leak_count;
    ft_size_t output_index;
    Page *page;

    if (entry_count == nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (entries == nullptr && capacity != 0)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    leak_count = 0;
    page = page_list;
    while (page != nullptr)
    {
        Block *block;

        block = cma_next_live_block(page->blocks);
        while (block != nullptr)
        {
            if (block->leak_ignored == FT_FALSE)
                leak_count += 1;
            block = cma_next_live_block(block->next);
        }
        page = page->next;
    }
    *entry_count = leak_count;
    if (capacity < leak_count)
    {
        cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_OPERATION);
    }
    output_index = 0;
    page = page_list;
    while (page != nullptr)
    {
        Block *block;

        block = cma_next_live_block(page->blocks);
        while (block != nullptr)
        {
            if (block->leak_ignored == FT_FALSE)
            {
                entries[output_index].memory_pointer = cma_block_user_pointer(block);
                entries[output_index].size = cma_block_user_size(block);
                output_index += 1;
            }
            block = cma_next_live_block(block->next);
        }
        page = page->next;
    }
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t cma_report_leaks(void)
{
    ft_bool lock_acquired;
    Page *page;
    cma_leak_summary summary;

    lock_acquired = FT_FALSE;
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    summary.live_block_count = 0;
    summary.live_bytes = 0;
    summary.ignored_block_count = 0;
    summary.ignored_bytes = 0;
    page = page_list;
    while (page != nullptr)
    {
        Block *block;

        block = cma_next_live_block(page->blocks);
        while (block != nullptr)
        {
            if (block->leak_ignored == FT_TRUE)
            {
                summary.ignored_block_count += 1;
                summary.ignored_bytes += cma_block_user_size(block);
            }
            else
            {
                summary.live_block_count += 1;
                summary.live_bytes += cma_block_user_size(block);
            }
            block = cma_next_live_block(block->next);
        }
        page = page->next;
    }
    std::printf("[cma] leak summary: live_blocks=%" PRIu64 " live_bytes=%" PRIu64
        " ignored_blocks=%" PRIu64 " ignored_bytes=%" PRIu64 "\n",
        summary.live_block_count, summary.live_bytes,
        summary.ignored_block_count, summary.ignored_bytes);
    page = page_list;
    while (page != nullptr)
    {
        Block *block;

        block = cma_next_live_block(page->blocks);
        while (block != nullptr)
        {
            if (block->leak_ignored == FT_FALSE)
            {
                std::printf("  [leak] pointer=%p size=%" PRIu64 "\n",
                    cma_block_user_pointer(block), cma_block_user_size(block));
                cmp_stack_trace_print(stdout, block->leak_stack_frames,
                    block->leak_stack_frame_count);
            }
            block = cma_next_live_block(block->next);
        }
        page = page->next;
    }
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t cma_untrack_leak(void *memory_pointer)
{
    ft_bool lock_acquired;
    Block *block;

    if (memory_pointer == nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    block = cma_find_block_for_pointer(memory_pointer);
    if (block == nullptr || cma_block_is_free(block) == FT_TRUE)
    {
        cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_HANDLE);
    }
    block->leak_ignored = FT_TRUE;
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t cma_track_leak(void *memory_pointer)
{
    ft_bool lock_acquired;
    Block *block;

    if (memory_pointer == nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_acquired = FT_FALSE;
    if (cma_lock_allocator(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    block = cma_find_block_for_pointer(memory_pointer);
    if (block == nullptr || cma_block_is_free(block) == FT_TRUE)
    {
        cma_unlock_allocator(lock_acquired);
        return (FT_ERR_INVALID_HANDLE);
    }
    block->leak_ignored = FT_FALSE;
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}
#endif
