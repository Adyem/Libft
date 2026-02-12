#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"
#include "../System_utils/system_utils.hpp"

static int32_t  normalize_alignment_padding(ft_size_t *padding)
{
    ft_size_t    aligned_padding;

    if (padding == ft_nullptr)
        return (0);
    if (*padding == 0)
        return (1);
    aligned_padding = align16(*padding);
    if (aligned_padding < *padding)
        return (0);
    *padding = aligned_padding;
    return (1);
}

static ft_size_t    calculate_alignment_padding(Block *block, ft_size_t alignment)
{
    uintptr_t    payload_address;
    uintptr_t    alignment_value;
    uintptr_t    remainder;
    ft_size_t    padding;

    if (block == ft_nullptr)
        return (0);
    payload_address = reinterpret_cast<uintptr_t>(block->payload);
    payload_address += cma_debug_guard_size();
    alignment_value = alignment;
    if (alignment_value == 0)
        return (0);
    remainder = payload_address & (alignment_value - 1);
    if (remainder == 0)
        return (0);
    padding = alignment_value - remainder;
    return (padding);
}

static int32_t  block_supports_aligned_request(Block *block, ft_size_t aligned_size,
            ft_size_t alignment, ft_size_t *padding)
{
    ft_size_t    remaining_size;
    ft_size_t    local_padding;
    ft_size_t    minimum_payload;

    local_padding = calculate_alignment_padding(block, alignment);
    if (normalize_alignment_padding(&local_padding) == 0)
        return (0);
    minimum_payload = align16(1);
    if (minimum_payload < static_cast<ft_size_t>(16))
        minimum_payload = static_cast<ft_size_t>(16);
    if (local_padding > 0)
    {
        if (block->size <= local_padding)
            return (0);
        remaining_size = block->size - local_padding;
        if (remaining_size < minimum_payload)
            return (0);
    }
    else
        remaining_size = block->size;
    if (remaining_size < aligned_size)
        return (0);
    *padding = local_padding;
    return (1);
}

static Block   *find_aligned_free_block(ft_size_t aligned_size, ft_size_t alignment,
            ft_size_t *padding)
{
    Page    *current_page;
    Block   *current_block;

    current_page = page_list;
    while (current_page)
    {
        current_block = current_page->blocks;
        while (current_block)
        {
            cma_validate_block(current_block, "cma_aligned_alloc search", ft_nullptr);
            if (cma_block_is_free(current_block))
            {
                ft_size_t   local_padding;

                if (block_supports_aligned_request(current_block, aligned_size,
                        alignment, &local_padding))
                {
                    *padding = local_padding;
                    return (current_block);
                }
            }
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (ft_nullptr);
}

static void    *aligned_alloc_offswitch(ft_size_t alignment, ft_size_t request_size,
            int32_t *error_code)
{
    ft_size_t  alignment_value;
    ft_size_t  allocation_size;
    ft_size_t  remainder;
    int32_t    local_error_code;
    int32_t    *error_target;
    void    *pointer;

    local_error_code = FT_ERR_SUCCESS;
    error_target = error_code;
    if (error_target == ft_nullptr)
        error_target = &local_error_code;
    alignment_value = alignment;
    allocation_size = request_size;
    remainder = 0;
    if (alignment_value != 0)
        remainder = allocation_size % alignment_value;
    if (remainder != 0)
        allocation_size += alignment_value - remainder;
    if (allocation_size == 0)
        allocation_size = alignment_value;
    pointer = ft_nullptr;
#ifdef _WIN32
    pointer = std::aligned_alloc(alignment_value, allocation_size);
    if (pointer)
    {
        g_cma_allocation_count++;
        *error_target = FT_ERR_SUCCESS;
    }
    else
        *error_target = FT_ERR_NO_MEMORY;
#else
    *error_target = posix_memalign(&pointer, alignment_value, allocation_size);
    if (*error_target == 0 && pointer)
    {
        g_cma_allocation_count++;
        *error_target = FT_ERR_SUCCESS;
    }
    else
        *error_target = FT_ERR_NO_MEMORY;
#endif
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_aligned_alloc %llu (alignment %llu) -> %p",
            allocation_size, alignment, pointer);
    return (pointer);
}

static ft_size_t    compute_extended_page_request(ft_size_t aligned_size,
            ft_size_t alignment)
{
    ft_size_t    extended_size;

    if (aligned_size > FT_SYSTEM_SIZE_MAX - alignment)
        return (0);
    extended_size = aligned_size + alignment;
    return (extended_size);
}


void    *cma_aligned_alloc(ft_size_t alignment, ft_size_t size)
{
    if ((alignment & (alignment - 1)) != 0
        || alignment < sizeof(void *))
        return (ft_nullptr);
    if (alignment > FT_SYSTEM_SIZE_MAX || size > FT_SYSTEM_SIZE_MAX)
        return (ft_nullptr);
    ft_size_t request_size = (size == 0) ? 1 : size;
    ft_size_t backend_aligned_size = align16(request_size);
    if (backend_aligned_size < request_size)
        return (ft_nullptr);
    ft_size_t limit_check_size = backend_aligned_size;
    if (alignment > limit_check_size)
        limit_check_size = alignment;
    if (g_cma_alloc_limit != 0 && limit_check_size > g_cma_alloc_limit)
        return (ft_nullptr);
    if (cma_backend_is_enabled())
        return (cma_backend_aligned_allocate(alignment,
                backend_aligned_size, ft_nullptr));
    ft_size_t instrumented_size = cma_debug_allocation_size(request_size);
    if (instrumented_size < request_size)
        return (ft_nullptr);
    ft_size_t aligned_size = align16(instrumented_size);
    if (aligned_size < instrumented_size)
        return (ft_nullptr);
    if (OFFSWITCH == 1)
        return (aligned_alloc_offswitch(alignment, request_size, ft_nullptr));
    bool lock_acquired = false;
    int32_t lock_error = cma_lock_allocator(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    ft_size_t padding = 0;
    Block *block = find_aligned_free_block(aligned_size, alignment, &padding);
    if (!block)
    {
        ft_size_t page_request = compute_extended_page_request(aligned_size,
                alignment);
        if (page_request == 0)
        {
            cma_unlock_allocator(lock_acquired);
            return (ft_nullptr);
        }
        Page *page = create_page(page_request);
        if (!page)
        {
            cma_unlock_allocator(lock_acquired);
            return (ft_nullptr);
        }
        block = page->blocks;
        cma_validate_block(block, "cma_aligned_alloc new page", ft_nullptr);
        padding = calculate_alignment_padding(block, alignment);
    }
    if (normalize_alignment_padding(&padding) == 0)
    {
        cma_unlock_allocator(lock_acquired);
        return (ft_nullptr);
    }
    if (padding > 0)
    {
        Block *prefix_block = split_block(block, padding);
        cma_validate_block(prefix_block, "cma_aligned_alloc prefix", ft_nullptr);
        block = prefix_block->next;
        if (!block)
        {
            cma_unlock_allocator(lock_acquired);
            return (ft_nullptr);
        }
        cma_validate_block(block, "cma_aligned_alloc aligned", ft_nullptr);
    }
    block = split_block(block, aligned_size);
    cma_validate_block(block, "cma_aligned_alloc split", ft_nullptr);
    if (!cma_block_is_free(block))
    {
        cma_unlock_allocator(lock_acquired);
        su_sigabrt();
    }
    cma_mark_block_allocated(block);
    g_cma_allocation_count++;
    g_cma_current_bytes += block->size;
    if (g_cma_current_bytes > g_cma_peak_bytes)
        g_cma_peak_bytes = g_cma_current_bytes;
    cma_debug_prepare_allocation(block, request_size);
    void *result = static_cast<void *>(cma_block_user_pointer(block));
    cma_unlock_allocator(lock_acquired);
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_aligned_alloc %llu (alignment %llu) -> %p",
                request_size, alignment, result);
    return (result);
}
