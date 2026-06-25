#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include "CMA.hpp"
#include "cma_internal.hpp"

#include "../System_utils/system_utils.hpp"
#include "../Sink/sink.hpp"
#include <cstdarg>
#include <cstdio>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void cma_set_alloc_logging(ft_bool enable)
{
    g_cma_alloc_logging = enable;
    return ;
}

ft_bool cma_get_alloc_logging(void)
{
    return (g_cma_alloc_logging);
}

void cma_record_allocation_log(const char *format_string, ...)
{
    char message_buffer[256];
    va_list argument_list;
    int32_t formatted_length;

    if (format_string == nullptr || g_cma_alloc_logging == FT_FALSE)
        return ;
    va_start(argument_list, format_string);
    formatted_length = std::vsnprintf(message_buffer, sizeof(message_buffer),
            format_string, argument_list);
    va_end(argument_list);
    if (formatted_length < 0)
        return ;
    (void)sink_record_message(0, message_buffer);
    return ;
}
#include "../Errno/errno.hpp"

static ft_size_t determine_page_size(ft_size_t size)
{
    if (size < SMALL_SIZE)
        return (SMALL_ALLOC);
    else if (size < MEDIUM_SIZE)
        return (MEDIUM_ALLOC);
    return (size);
}

static void determine_page_use(Page *page)
{
    if (page->heap == FT_FALSE)
        page->alloc_size_type = 0;
    else if (page->size == SMALL_ALLOC)
        page->alloc_size_type = 0;
    else if (page->size == MEDIUM_ALLOC)
        page->alloc_size_type = 1;
    else
        page->alloc_size_type = 2;
    return ;
}

static int8_t determine_which_block_to_use(ft_size_t size)
{
    if (size < SMALL_SIZE)
        return (0);
    else if (size < MEDIUM_SIZE)
        return (1);
    return (2);
}

static void *create_stack_block(void)
{
    static char memory_block[PAGE_SIZE];

    return (memory_block);
}

static void report_corrupted_block(Block *block, const char *context,
        void *user_pointer)
{
    (void)block;
    (void)user_pointer;
    (void)context;
    su_sigabrt();
    return ;
}

static ft_bool are_blocks_adjacent(Block *left_block, Block *right_block)
{
    unsigned char   *expected_address;
    unsigned char   *actual_address;

    if (left_block == nullptr || right_block == nullptr)
        return (FT_FALSE);
    if (left_block->payload == nullptr || right_block->payload == nullptr)
        return (FT_FALSE);
    expected_address = left_block->payload + left_block->size;
    actual_address = right_block->payload;
    if (expected_address == actual_address)
        return (FT_TRUE);
    return (FT_FALSE);
}

static void verify_forward_link(Block *block, Block *next_block)
{
    if (next_block->prev != block)
        report_corrupted_block(next_block, "merge_block inconsistent next link",
            nullptr);
    if (are_blocks_adjacent(block, next_block) == FT_FALSE)
        report_corrupted_block(next_block, "merge_block detached next neighbor",
            nullptr);
    return ;
}

static void verify_backward_link(Block *block, Block *previous_block)
{
    if (previous_block->next != block)
        report_corrupted_block(previous_block,
            "merge_block inconsistent prev link", nullptr);
    if (are_blocks_adjacent(previous_block, block) == FT_FALSE)
        report_corrupted_block(previous_block,
            "merge_block detached prev neighbor", nullptr);
    return ;
}

static void verify_traversal_link(Block *current_block, Block *next_block,
        const char *context)
{
    if (current_block == nullptr || next_block == nullptr)
        return ;
    if (current_block == next_block)
        report_corrupted_block(current_block, context, nullptr);
    if (next_block->prev != current_block)
        report_corrupted_block(next_block, context, nullptr);
    if (are_blocks_adjacent(current_block, next_block) == FT_FALSE)
        report_corrupted_block(next_block, context, nullptr);
    return ;
}

void cma_validate_block(Block *block, const char *context, void *user_pointer)
{
    const char    *location;
    ft_bool            sentinel_free;
    ft_bool            sentinel_allocated;

    location = context;
    if (block == nullptr)
    {
        if (location == nullptr)
            location = "unknown";
        su_sigabrt();
    }
    sentinel_free = (block->magic == MAGIC_NUMBER_FREE);
    sentinel_allocated = (block->magic == MAGIC_NUMBER_ALLOCATED);
    if (!sentinel_free && !sentinel_allocated)
        report_corrupted_block(block, location, user_pointer);
    if (sentinel_free && block->free == FT_FALSE)
        cma_mark_block_free(block);
    if (sentinel_allocated && block->free == FT_TRUE)
        cma_mark_block_allocated(block);
    if (block->payload == nullptr)
        report_corrupted_block(block, location, user_pointer);
    return ;
}

static ft_size_t    minimum_split_payload(void)
{
    ft_size_t    minimum_payload;

    minimum_payload = align16(1);
    if (minimum_payload < static_cast<ft_size_t>(16))
        minimum_payload = static_cast<ft_size_t>(16);
    return (minimum_payload);
}

Block* split_block(Block* block, ft_size_t size)
{
    Block       *new_block;
    ft_size_t    available_size;
    ft_size_t    remaining_size;
    ft_size_t    minimum_payload;
    Block       *result_block;
    ft_bool         metadata_guarded;

    result_block = block;
    metadata_guarded = FT_FALSE;
    cma_validate_block(block, "split_block", nullptr);
    metadata_guarded = cma_metadata_guard_increment();
    if (!metadata_guarded)
        goto split_block_cleanup;
    if (cma_metadata_make_writable() != FT_ERR_SUCCESS)
        goto split_block_cleanup;
    available_size = block->size;
    if (size >= available_size)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        goto split_block_cleanup;
    }
    remaining_size = available_size - size;
    minimum_payload = minimum_split_payload();
    if (remaining_size <= minimum_payload)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        goto split_block_cleanup;
    }
    new_block = cma_metadata_allocate_block();
    if (new_block == nullptr)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        goto split_block_cleanup;
    }
    new_block->size = remaining_size;
    new_block->payload = block->payload + size;
    cma_debug_initialize_block(new_block);
    cma_mark_block_free(new_block);
    new_block->next = block->next;
    new_block->prev = block;
    if (new_block->next)
    {
        cma_validate_block(new_block->next, "split_block relink next", nullptr);
        new_block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
    if (cma_block_is_free(block))
        cma_mark_block_free(block);
    else
        cma_mark_block_allocated(block);
    cma_debug_initialize_block(block);
    result_block = block;
split_block_cleanup:
    if (metadata_guarded)
        cma_metadata_guard_decrement();
    return (result_block);
}

Page *create_page(ft_size_t size)
{
    ft_size_t page_size = determine_page_size(size);
    ft_bool use_heap = FT_TRUE;

    if (page_list == nullptr && page_size <= PAGE_SIZE)
    {
        page_size = PAGE_SIZE;
        use_heap = FT_FALSE;
    }
    else
    {
        if (size > determine_page_size(size))
            page_size = size;
    }
    void* memory_pointer;
    if (use_heap)
    {
        memory_pointer = std::malloc(page_size);
        if (!memory_pointer)
            return (nullptr);
    }
    else
    {
        memory_pointer = create_stack_block();
        if (!memory_pointer)
            return (nullptr);
    }
    Page* page = static_cast<Page*>(std::malloc(sizeof(Page)));
    if (!page)
    {
        if (use_heap)
            std::free(memory_pointer);
        return (nullptr);
    }
    std::memset(page, 0, sizeof(Page));
    page->heap = use_heap;
    page->start = memory_pointer;
    page->size = page_size;
    page->next = nullptr;
    page->prev = nullptr;
    page->blocks = cma_metadata_allocate_block();
    if (page->blocks == nullptr)
    {
        if (use_heap)
            std::free(memory_pointer);
        std::free(page);
        return (nullptr);
    }
    page->blocks->size = page_size;
    page->blocks->payload = static_cast<unsigned char *>(memory_pointer);
    cma_debug_initialize_block(page->blocks);
    cma_mark_block_free(page->blocks);
    page->blocks->next = nullptr;
    page->blocks->prev = nullptr;
    cma_validate_block(page->blocks, "create_page", nullptr);
    determine_page_use(page);
    if (!page_list)
    {
        page_list = page;
    }
    else
    {
        page->next = page_list;
        page_list->prev = page;
        page_list = page;
    }
    return (page);
}

Block *find_free_block(ft_size_t size)
{
    Page* cur_page = page_list;
    int8_t alloc_size_type = determine_which_block_to_use(size);
    while (cur_page)
    {
        if (cur_page->alloc_size_type != alloc_size_type)
        {
            cur_page = cur_page->next;
            continue ;
        }
        Block* cur_block = cur_page->blocks;
        while (cur_block)
        {
            cma_validate_block(cur_block, "find_free_block", nullptr);
            if (cma_block_is_free(cur_block) && cur_block->size >= size)
                return (cur_block);
            verify_traversal_link(cur_block, cur_block->next,
                "find_free_block corrupted traversal link");
            cur_block = cur_block->next;
        }
        cur_page = cur_page->next;
    }
    return (nullptr);
}

Block    *cma_find_block_for_pointer(const void *memory_pointer)
{
    Page    *current_page;

    if (memory_pointer == nullptr)
        return (nullptr);
    current_page = page_list;
    while (current_page)
    {
        Block    *current_block;

        current_block = current_page->blocks;
        while (current_block)
        {
            if (cma_block_user_pointer(current_block) == memory_pointer)
                return (current_block);
            verify_traversal_link(current_block, current_block->next,
                "cma_find_block_for_pointer corrupted traversal link");
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (nullptr);
}

Block *merge_block(Block *block)
{
    Block       *current;
    Block       *next_block;
    Block       *previous_block;

    cma_validate_block(block, "merge_block", nullptr);
    current = block;
    previous_block = current->prev;
    while (previous_block && cma_block_is_free(previous_block))
    {
        cma_validate_block(previous_block, "merge_block prev", nullptr);
        verify_backward_link(current, previous_block);
#ifdef DEBUG
#endif
        previous_block->size += current->size;
#ifdef DEBUG
#endif
        previous_block->next = current->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink prev", nullptr);
            current->next->prev = previous_block;
        }
        current->next = nullptr;
        current->prev = nullptr;
        cma_mark_block_free(current);
        cma_metadata_release_block(current);
        current = previous_block;
        previous_block = current->prev;
    }
    next_block = current->next;
    while (next_block && cma_block_is_free(next_block))
    {
        cma_validate_block(next_block, "merge_block next", nullptr);
        verify_forward_link(current, next_block);
#ifdef DEBUG
#endif
        current->size += next_block->size;
#ifdef DEBUG
#endif
        current->next = next_block->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink next", nullptr);
            current->next->prev = current;
        }
        next_block->next = nullptr;
        next_block->prev = nullptr;
        cma_mark_block_free(next_block);
        cma_metadata_release_block(next_block);
        next_block = current->next;
    }
    cma_mark_block_free(current);
#ifdef DEBUG
#endif
    return (current);
}

Page *find_page_of_block(Block *block)
{
    Page *page = page_list;
    while (page)
    {
        unsigned char    *start;
        unsigned char    *end;
        unsigned char    *payload;

        start = static_cast<unsigned char *>(page->start);
        end = start + page->size;
        payload = block->payload;
        if (payload >= start && payload < end)
            return (page);
        page = page->next;
    }
    return (nullptr);
}

void free_page_if_empty(Page *page)
{
    if (!page || page->heap == FT_FALSE)
        return ;
    if (page->blocks && cma_block_is_free(page->blocks) &&
        page->blocks->next == nullptr &&
        page->blocks->prev == nullptr)
    {
        if (page->prev)
            page->prev->next = page->next;
        if (page->next)
            page->next->prev = page->prev;
        if (page_list == page)
            page_list = page->next;
        std::free(page->start);
        cma_metadata_release_block(page->blocks);
        std::free(page);
        return ;
    }
    return ;
}

int32_t cma_get_extended_stats(ft_size_t *allocation_count,
        ft_size_t *free_count,
        ft_size_t *current_bytes,
        ft_size_t *peak_bytes)
{
    ft_bool lock_acquired = FT_FALSE;
    int32_t lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (allocation_count != nullptr)
        *allocation_count = g_cma_allocation_count;
    if (free_count != nullptr)
        *free_count = g_cma_free_count;
    if (current_bytes != nullptr)
        *current_bytes = g_cma_current_bytes;
    if (peak_bytes != nullptr)
        *peak_bytes = g_cma_peak_bytes;
    cma_unlock_allocator(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count)
{
    return (cma_get_extended_stats(allocation_count, free_count,
            nullptr, nullptr));
}
