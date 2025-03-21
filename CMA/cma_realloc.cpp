#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <new>
#include <pthread.h>
#include <csignal>
#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../CPP_class/nullptr.hpp"

void* cma_realloc(void* ptr, size_t new_size)
{
    if (OFFSWITCH == 1)
    {
        void* new_ptr = ::operator new(new_size, std::align_val_t(8), std::nothrow);
        if (ptr && new_ptr)
        {
            Block* old_block = reinterpret_cast<Block*> ((static_cast<char*> (ptr)
						- sizeof(Block)));
            size_t copy_size = old_block->size < new_size ? old_block->size : new_size;
            memcpy(new_ptr, ptr, copy_size);
        }
        ::operator delete(ptr, std::align_val_t(8), std::nothrow);
        return (new_ptr);
    }
    if (!ptr)
        return (cma_malloc(new_size));
    if (new_size <= 0)
    {
        cma_free(ptr);
        return (ft_nullptr);
    }
    void* new_ptr = cma_malloc(new_size);
    if (!new_ptr)
    {
        cma_free(ptr);
        return (ft_nullptr);
    }
	g_malloc_mutex.lock(pthread_self());
    Block* old_block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
				- sizeof(Block)));
    if (old_block->magic != MAGIC_NUMBER)
	{
		g_malloc_mutex.unlock(pthread_self());
        return (ft_nullptr);
	}
    size_t copy_size = old_block->size < new_size ? old_block->size : new_size;
    memcpy(new_ptr, ptr, copy_size);
    cma_free(ptr);
	g_malloc_mutex.unlock(pthread_self());
    return (new_ptr);
}
