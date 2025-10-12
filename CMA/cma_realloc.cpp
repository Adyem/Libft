#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <pthread.h>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Libft/limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread.hpp"
#include "../System_utils/system_utils.hpp"

static int reallocate_block(void *ptr, ft_size_t new_size)
{
    if (!ptr)
        return (-1);
    Block *block = cma_find_block_for_pointer(ptr);
    if (!block)
        return (-1);
    cma_validate_block(block, "cma_realloc resize", ptr);
    if (block->size >= new_size)
    {
        split_block(block, new_size);
        cma_validate_block(block, "cma_realloc split in place", ptr);
        return (0);
    }
    if (block->next && cma_block_is_free(block->next) &&
        (block->size + block->next->size) >= new_size)
    {
        cma_validate_block(block->next, "cma_realloc neighbor", ft_nullptr);
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next)
        {
            cma_validate_block(block->next, "cma_realloc relink", ft_nullptr);
            block->next->prev = block;
        }
        split_block(block, new_size);
        cma_validate_block(block, "cma_realloc split after merge", ptr);
        return (0);
    }
    return (-1);
}

static void *allocate_block_locked(ft_size_t aligned_size)
{
    Block *block;
    Page *page;

    block = find_free_block(aligned_size);
    if (!block)
    {
        page = create_page(aligned_size);
        if (!page)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        block = page->blocks;
    }
    cma_validate_block(block, "cma_realloc allocate", ft_nullptr);
    if (!cma_block_is_free(block))
    {
        ft_errno = FT_ERR_INVALID_STATE;
        su_sigabrt();
    }
    block = split_block(block, aligned_size);
    cma_validate_block(block, "cma_realloc allocate split", ft_nullptr);
    cma_mark_block_allocated(block);
    g_cma_allocation_count++;
    g_cma_current_bytes += block->size;
    if (g_cma_current_bytes > g_cma_peak_bytes)
        g_cma_peak_bytes = g_cma_current_bytes;
    ft_errno = ER_SUCCESS;
    void *result;

    result = block->payload;
    return (result);
}

static void release_block_locked(Block *block)
{
    ft_size_t freed_size;
    Page *page;

    cma_validate_block(block, "cma_realloc release", ft_nullptr);
    if (cma_block_is_free(block))
    {
        ft_errno = FT_ERR_INVALID_STATE;
        su_sigabrt();
    }
    freed_size = block->size;
    cma_mark_block_free(block);
    block = merge_block(block);
    page = find_page_of_block(block);
    free_page_if_empty(page);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    return ;
}

void *cma_realloc(void* ptr, ft_size_t new_size)
{
    if (new_size > FT_SYSTEM_SIZE_MAX)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    if (g_cma_alloc_limit != 0 && new_size > g_cma_alloc_limit)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    if (cma_backend_is_enabled())
    {
        if (!ptr || cma_backend_owns_pointer(ptr))
            return (cma_backend_reallocate(ptr, new_size));
    }
    if (OFFSWITCH == 1)
    {
        void *result = std::realloc(ptr, static_cast<size_t>(new_size));
        if (!ptr && result)
        {
            g_cma_allocation_count++;
            ft_errno = ER_SUCCESS;
        }
        else if (ptr && new_size == 0)
        {
            g_cma_free_count++;
            ft_errno = ER_SUCCESS;
        }
        else if (!result && new_size != 0)
            ft_errno = FT_ERR_NO_MEMORY;
        else
            ft_errno = ER_SUCCESS;
        return (result);
    }
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return (ft_nullptr);
    if (!ptr)
    {
        allocator_guard.unlock();
        return (cma_malloc(new_size));
    }
    if (new_size == 0)
    {
        allocator_guard.unlock();
        cma_free(ptr);
        ft_errno = ER_SUCCESS;
        return (ft_nullptr);
    }
    ft_size_t aligned_size = align16(new_size);
    Block *block = cma_find_block_for_pointer(ptr);
    if (!block)
    {
        int error_code;

        ft_errno = FT_ERR_INVALID_POINTER;
        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (ft_nullptr);
    }
    cma_validate_block(block, "cma_realloc header", ptr);
    ft_size_t previous_size = block->size;
    int error = reallocate_block(ptr, aligned_size);
    if (error == 0)
    {
        if (g_cma_current_bytes >= previous_size)
            g_cma_current_bytes -= previous_size;
        else
            g_cma_current_bytes = 0;
        g_cma_current_bytes += block->size;
        if (g_cma_current_bytes > g_cma_peak_bytes)
            g_cma_peak_bytes = g_cma_current_bytes;
        ft_errno = ER_SUCCESS;
        allocator_guard.unlock();
        ft_errno = ER_SUCCESS;
        return (ptr);
    }
    Block *old_block;
    ft_size_t copy_size;

    old_block = block;
    cma_validate_block(old_block, "cma_realloc copy source", ptr);
    if (old_block->size < aligned_size)
        copy_size = old_block->size;
    else
        copy_size = aligned_size;
    void *new_ptr = allocate_block_locked(aligned_size);
    if (!new_ptr)
    {
        int error_code;

        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (ft_nullptr);
    }
    ft_memcpy(new_ptr, ptr, static_cast<size_t>(copy_size));
    release_block_locked(old_block);
    ft_errno = ER_SUCCESS;
    allocator_guard.unlock();
    ft_errno = ER_SUCCESS;
    return (new_ptr);
}
