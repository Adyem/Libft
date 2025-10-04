#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cassert>
#include <pthread.h>
#include <csignal>
#include "../Errno/errno.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Libft/limits.hpp"
#include "../CPP_class/class_nullptr.hpp"

static int reallocate_block(void *ptr, ft_size_t new_size)
{
    if (!ptr)
        return (-1);
    Block* block = reinterpret_cast<Block*>((static_cast<char*>(ptr) - sizeof(Block)));
    if (block->size >= new_size)
    {
        split_block(block, new_size);
        return (0);
    }
    if (block->next && block->next->free &&
        (block->size + sizeof(Block) + block->next->size) >= new_size)
    {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
        split_block(block, new_size);
        return (0);
    }
    return (-1);
}

void *cma_realloc(void* ptr, ft_size_t new_size)
{
    if (new_size > FT_SYSTEM_SIZE_MAX)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
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
            ft_errno = FT_EALLOC;
        else
            ft_errno = ER_SUCCESS;
        return (result);
    }
    if (g_cma_alloc_limit != 0 && new_size > g_cma_alloc_limit)
    {
        ft_errno = FT_EALLOC;
        return (ft_nullptr);
    }
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    if (!ptr)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        return (cma_malloc(new_size));
    }
    if (new_size == 0)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        cma_free(ptr);
        ft_errno = ER_SUCCESS;
        return (ft_nullptr);
    }
    ft_size_t requested_size = new_size;
    ft_size_t aligned_size = align16(new_size);
    int error = reallocate_block(ptr, aligned_size);
    if (error == 0)
    {
        Block* resized_block = reinterpret_cast<Block*>(
            static_cast<char*>(ptr) - sizeof(Block));
        resized_block->requested_size = requested_size;
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = ER_SUCCESS;
        return (ptr);
    }
    void* new_ptr = cma_malloc(requested_size);
    if (!new_ptr)
    {
        int error_code;

        error_code = ft_errno;
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = error_code;
        return (ft_nullptr);
    }
    Block* old_block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
                - sizeof(Block)));
    if (old_block->magic != MAGIC_NUMBER)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        cma_free(ptr);
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    ft_size_t copy_size;
    if (old_block->size < aligned_size)
        copy_size = old_block->size;
    else
        copy_size = aligned_size;
    ft_memcpy(new_ptr, ptr, static_cast<size_t>(copy_size));
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    cma_free(ptr);
    ft_errno = ER_SUCCESS;
    return (new_ptr);
}
