#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Logger/logger.hpp"
#include "../System_utils/system_utils.hpp"

static int  normalize_alignment_padding(ft_size_t *padding)
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
    alignment_value = static_cast<uintptr_t>(alignment);
    if (alignment_value == 0)
        return (0);
    remainder = payload_address & (alignment_value - 1);
    if (remainder == 0)
        return (0);
    padding = static_cast<ft_size_t>(alignment_value - remainder);
    return (padding);
}

static int  block_supports_aligned_request(Block *block, ft_size_t aligned_size,
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

static void    *aligned_alloc_offswitch(ft_size_t alignment, ft_size_t request_size)
{
    size_t  alignment_value;
    size_t  allocation_size;
    size_t  remainder;
    void    *pointer;
    int     error_code;

    alignment_value = static_cast<size_t>(alignment);
    allocation_size = static_cast<size_t>(request_size);
    remainder = 0;
    if (alignment_value != 0)
        remainder = allocation_size % alignment_value;
    if (remainder != 0)
        allocation_size += alignment_value - remainder;
    if (allocation_size == 0)
        allocation_size = alignment_value;
    pointer = ft_nullptr;
#ifdef _WIN32
    (void)error_code;
    pointer = std::aligned_alloc(alignment_value, allocation_size);
    if (pointer)
    {
        g_cma_allocation_count++;
        ft_errno = ER_SUCCESS;
    }
    else
        ft_errno = FT_ERR_NO_MEMORY;
#else
    error_code = posix_memalign(&pointer, alignment_value, allocation_size);
    if (error_code == 0 && pointer)
    {
        g_cma_allocation_count++;
        ft_errno = ER_SUCCESS;
    }
    else
        ft_errno = FT_ERR_NO_MEMORY;
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
    ft_size_t   request_size;
    ft_size_t   aligned_size;
    ft_size_t   padding;
    Block       *block;
    void        *result;

    if ((alignment & (alignment - 1)) != 0
        || alignment < sizeof(void *))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (alignment > FT_SYSTEM_SIZE_MAX || size > FT_SYSTEM_SIZE_MAX)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    request_size = size;
    if (request_size == 0)
        request_size = 1;
    aligned_size = align16(request_size);
    if (aligned_size < request_size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (ft_nullptr);
    }
    if (g_cma_alloc_limit != 0 && request_size > g_cma_alloc_limit)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (cma_backend_is_enabled())
        return (cma_backend_aligned_allocate(alignment, aligned_size));
    if (OFFSWITCH == 1)
        return (aligned_alloc_offswitch(alignment, request_size));
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return (ft_nullptr);
    padding = 0;
    block = find_aligned_free_block(aligned_size, alignment, &padding);
    if (!block)
    {
        ft_size_t   page_request;
        Page        *page;

        page_request = compute_extended_page_request(aligned_size, alignment);
        if (page_request == 0)
        {
            int error_code;

            ft_errno = FT_ERR_OUT_OF_RANGE;
            error_code = ft_errno;
            allocator_guard.unlock();
            ft_errno = error_code;
            return (ft_nullptr);
        }
        page = create_page(page_request);
        if (!page)
        {
            int error_code;

            ft_errno = FT_ERR_NO_MEMORY;
            error_code = ft_errno;
            allocator_guard.unlock();
            ft_errno = error_code;
            return (ft_nullptr);
        }
        block = page->blocks;
        cma_validate_block(block, "cma_aligned_alloc new page", ft_nullptr);
        padding = calculate_alignment_padding(block, alignment);
    }
    if (normalize_alignment_padding(&padding) == 0)
    {
        int error_code;

        ft_errno = FT_ERR_OUT_OF_RANGE;
        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (ft_nullptr);
    }
    if (padding > 0)
    {
        Block   *prefix_block;

        prefix_block = split_block(block, padding);
        cma_validate_block(prefix_block, "cma_aligned_alloc prefix", ft_nullptr);
        block = prefix_block->next;
        if (!block)
        {
            int error_code;

            ft_errno = FT_ERR_NO_MEMORY;
            error_code = ft_errno;
            allocator_guard.unlock();
            ft_errno = error_code;
            return (ft_nullptr);
        }
        cma_validate_block(block, "cma_aligned_alloc aligned", ft_nullptr);
    }
    block = split_block(block, aligned_size);
    cma_validate_block(block, "cma_aligned_alloc split", ft_nullptr);
    if (!cma_block_is_free(block))
    {
        allocator_guard.unlock();
        ft_errno = FT_ERR_INVALID_STATE;
        su_sigabrt();
    }
    cma_mark_block_allocated(block);
    g_cma_allocation_count++;
    g_cma_current_bytes += block->size;
    if (g_cma_current_bytes > g_cma_peak_bytes)
        g_cma_peak_bytes = g_cma_current_bytes;
    result = static_cast<void *>(block->payload);
    allocator_guard.unlock();
    ft_errno = ER_SUCCESS;
    if (ft_log_get_alloc_logging())
        ft_log_debug("cma_aligned_alloc %llu (alignment %llu) -> %p",
            request_size, alignment, result);
    return (result);
}
