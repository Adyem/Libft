#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Logger/logger.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/system_utils.hpp"
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
    if (page->heap == false)
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
    ft_errno = FT_ERR_INVALID_STATE;
    su_sigabrt();
    return ;
}

static int are_blocks_adjacent(Block *left_block, Block *right_block)
{
    unsigned char   *expected_address;
    unsigned char   *actual_address;

    if (left_block == ft_nullptr || right_block == ft_nullptr)
        return (0);
    if (left_block->payload == ft_nullptr || right_block->payload == ft_nullptr)
        return (0);
    expected_address = left_block->payload + left_block->size;
    actual_address = right_block->payload;
    if (expected_address == actual_address)
        return (1);
    return (0);
}

static void verify_forward_link(Block *block, Block *next_block)
{
    if (next_block->prev != block)
        report_corrupted_block(next_block, "merge_block inconsistent next link",
            ft_nullptr);
    if (are_blocks_adjacent(block, next_block) == 0)
        report_corrupted_block(next_block, "merge_block detached next neighbor",
            ft_nullptr);
    return ;
}

static void verify_backward_link(Block *block, Block *previous_block)
{
    if (previous_block->next != block)
        report_corrupted_block(previous_block,
            "merge_block inconsistent prev link", ft_nullptr);
    if (are_blocks_adjacent(previous_block, block) == 0)
        report_corrupted_block(previous_block,
            "merge_block detached prev neighbor", ft_nullptr);
    return ;
}

void cma_validate_block(Block *block, const char *context, void *user_pointer)
{
    const char    *location;
    bool            sentinel_free;
    bool            sentinel_allocated;

    location = context;
    if (block == ft_nullptr)
    {
        if (location == ft_nullptr)
            location = "unknown";
        su_sigabrt();
    }
    sentinel_free = (block->magic == MAGIC_NUMBER_FREE);
    sentinel_allocated = (block->magic == MAGIC_NUMBER_ALLOCATED);
    if (!sentinel_free && !sentinel_allocated)
        report_corrupted_block(block, location, user_pointer);
    if (sentinel_free && block->free == false)
        cma_mark_block_free(block);
    if (sentinel_allocated && block->free == true)
        cma_mark_block_allocated(block);
    if (block->payload == ft_nullptr)
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

    cma_validate_block(block, "split_block", ft_nullptr);
    available_size = block->size;
    if (size >= available_size)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    remaining_size = available_size - size;
    minimum_payload = minimum_split_payload();
    if (remaining_size <= minimum_payload)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    new_block = cma_metadata_allocate_block();
    if (new_block == ft_nullptr)
    {
        if (cma_block_is_free(block))
            cma_mark_block_free(block);
        else
            cma_mark_block_allocated(block);
        return (block);
    }
    new_block->size = remaining_size;
    new_block->payload = block->payload + size;
    cma_debug_initialize_block(new_block);
    cma_mark_block_free(new_block);
    new_block->next = block->next;
    new_block->prev = block;
    if (new_block->next)
    {
        cma_validate_block(new_block->next, "split_block relink next", ft_nullptr);
        new_block->next->prev = new_block;
    }
    block->next = new_block;
    block->size = size;
    if (cma_block_is_free(block))
        cma_mark_block_free(block);
    else
        cma_mark_block_allocated(block);
    cma_debug_initialize_block(block);
    return (block);
}

Page *create_page(ft_size_t size)
{
    ft_size_t page_size = determine_page_size(size);
    bool use_heap = true;

    if (page_list == ft_nullptr)
    {
        page_size = PAGE_SIZE;
        use_heap = false;
    }
    else
    {
        if (size > determine_page_size(size))
            page_size = size;
    }
    void* ptr;
    if (use_heap)
    {
        ptr = std::malloc(static_cast<size_t>(page_size));
        if (!ptr)
            return (ft_nullptr);
    }
    else
    {
        ptr = create_stack_block();
        if (!ptr)
            return (ft_nullptr);
    }
    Page* page = static_cast<Page*>(std::malloc(sizeof(Page)));
    if (!page)
    {
        if (use_heap)
            std::free(ptr);
        return (ft_nullptr);
    }
    std::memset(page, 0, sizeof(Page));
    if (cma_page_prepare_thread_safety(page) != 0)
    {
        if (use_heap)
            std::free(ptr);
        std::free(page);
        return (ft_nullptr);
    }
    page->heap = use_heap;
    page->start = ptr;
    page->size = page_size;
    page->next = ft_nullptr;
    page->prev = ft_nullptr;
    page->blocks = cma_metadata_allocate_block();
    if (page->blocks == ft_nullptr)
    {
        cma_page_teardown_thread_safety(page);
        if (use_heap)
            std::free(ptr);
        std::free(page);
        return (ft_nullptr);
    }
    page->blocks->size = page_size;
    page->blocks->payload = static_cast<unsigned char *>(ptr);
    cma_debug_initialize_block(page->blocks);
    cma_mark_block_free(page->blocks);
    page->blocks->next = ft_nullptr;
    page->blocks->prev = ft_nullptr;
    cma_validate_block(page->blocks, "create_page", ft_nullptr);
    determine_page_use(page);
    if (!page_list) {
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
            cma_validate_block(cur_block, "find_free_block", ft_nullptr);
            if (cma_block_is_free(cur_block) && cur_block->size >= size)
                return (cur_block);
            cur_block = cur_block->next;
        }
        cur_page = cur_page->next;
    }
    return (ft_nullptr);
}

Block    *cma_find_block_for_pointer(const void *memory_pointer)
{
    Page    *current_page;

    if (memory_pointer == ft_nullptr)
        return (ft_nullptr);
    current_page = page_list;
    while (current_page)
    {
        Block    *current_block;

        current_block = current_page->blocks;
        while (current_block)
        {
            if (cma_block_user_pointer(current_block) == memory_pointer)
                return (current_block);
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (ft_nullptr);
}

Block *merge_block(Block *block)
{
    Block       *current;
    Block       *next_block;
    Block       *previous_block;

    cma_validate_block(block, "merge_block", ft_nullptr);
    current = block;
    previous_block = current->prev;
    while (previous_block && cma_block_is_free(previous_block))
    {
        cma_validate_block(previous_block, "merge_block prev", ft_nullptr);
        verify_backward_link(current, previous_block);
#ifdef DEBUG
#endif
        previous_block->size += current->size;
#ifdef DEBUG
#endif
        previous_block->next = current->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink prev", ft_nullptr);
            current->next->prev = previous_block;
        }
        current->next = ft_nullptr;
        current->prev = ft_nullptr;
        cma_mark_block_free(current);
        cma_metadata_release_block(current);
        current = previous_block;
        previous_block = current->prev;
    }
    next_block = current->next;
    while (next_block && cma_block_is_free(next_block))
    {
        cma_validate_block(next_block, "merge_block next", ft_nullptr);
        verify_forward_link(current, next_block);
#ifdef DEBUG
#endif
        current->size += next_block->size;
#ifdef DEBUG
#endif
        current->next = next_block->next;
        if (current->next)
        {
            cma_validate_block(current->next, "merge_block relink next", ft_nullptr);
            current->next->prev = current;
        }
        next_block->next = ft_nullptr;
        next_block->prev = ft_nullptr;
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
        end = start + static_cast<size_t>(page->size);
        payload = block->payload;
        if (payload >= start && payload < end)
            return (page);
        page = page->next;
    }
    return (ft_nullptr);
}

void free_page_if_empty(Page *page)
{
    bool lock_acquired;
    int entry_errno;

    if (!page || page->heap == false)
        return ;
    entry_errno = ft_errno;
    if (cma_page_lock(page, &lock_acquired) != 0)
        return ;
    if (page->blocks && cma_block_is_free(page->blocks) &&
        page->blocks->next == ft_nullptr &&
        page->blocks->prev == ft_nullptr)
    {
        if (page->prev)
            page->prev->next = page->next;
        if (page->next)
            page->next->prev = page->prev;
        if (page_list == page)
            page_list = page->next;
        if (lock_acquired)
            cma_page_unlock(page, lock_acquired);
        std::free(page->start);
        cma_metadata_release_block(page->blocks);
        cma_page_teardown_thread_safety(page);
        std::free(page);
        ft_errno = entry_errno;
        return ;
    }
    if (lock_acquired)
        cma_page_unlock(page, lock_acquired);
    ft_errno = entry_errno;
    return ;
}









void cma_get_extended_stats(ft_size_t *allocation_count,
        ft_size_t *free_count,
        ft_size_t *current_bytes,
        ft_size_t *peak_bytes)
{
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    if (allocation_count != ft_nullptr)
        *allocation_count = g_cma_allocation_count;
    if (free_count != ft_nullptr)
        *free_count = g_cma_free_count;
    if (current_bytes != ft_nullptr)
        *current_bytes = g_cma_current_bytes;
    if (peak_bytes != ft_nullptr)
        *peak_bytes = g_cma_peak_bytes;
    return ;
}

void cma_get_stats(ft_size_t *allocation_count, ft_size_t *free_count)
{
    cma_get_extended_stats(allocation_count, free_count, ft_nullptr, ft_nullptr);
    return ;
}
