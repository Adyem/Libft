#include <cstdlib>
#include <cstddef>
#include <cstdio>
#include <cassert>
#include <pthread.h>
#include <csignal>
#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

static int reallocate_block(void *ptr, size_t new_size)
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

void *cma_realloc(void* ptr, size_t new_size)
{
    if (OFFSWITCH == 1)
    {
        return std::realloc(ptr, new_size);
    }
	g_malloc_mutex.lock(THREAD_ID);
    if (!ptr)
        return (cma_malloc(new_size));
    if (new_size == 0)
    {
		g_malloc_mutex.unlock(THREAD_ID);
        cma_free(ptr);
        return (ft_nullptr);
    }
	new_size = align16(new_size);
	int error = reallocate_block(ptr, new_size);
	if (error == 0)
		return (ptr);
    void* new_ptr = cma_malloc(new_size);
    if (!new_ptr)
    {
		g_malloc_mutex.unlock(THREAD_ID);
        cma_free(ptr);
        return (ft_nullptr);
    }
    Block* old_block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
				- sizeof(Block)));
    if (old_block->magic != MAGIC_NUMBER)
	{
		g_malloc_mutex.unlock(THREAD_ID);
		cma_free(ptr);
        return (ft_nullptr);
	}
    size_t copy_size = old_block->size < new_size ? old_block->size : new_size;
    ft_memcpy(new_ptr, ptr, copy_size);
    cma_free(ptr);
	g_malloc_mutex.unlock(THREAD_ID);
    return (new_ptr);
}
