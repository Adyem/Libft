#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../CPP_class/nullptr.hpp"

void* cma_malloc(std::size_t size)
{
	if (OFFSWITCH == 1)
        return (malloc(size));
	if (size <= 0)
        return (ft_nullptr);
	g_malloc_mutex.lock(THREAD_ID);
    size_t aligned_size = align16(size);
    Block *block = find_free_block(aligned_size);
    if (!block)
    {
        Page* page = create_page(aligned_size);
        if (!page)
		{
			g_malloc_mutex.unlock(THREAD_ID);
            return (ft_nullptr);
		}
        block = page->blocks;
    }
    block = split_block(block, aligned_size);
    block->free = false;
	g_malloc_mutex.unlock(THREAD_ID);
    return (reinterpret_cast<char*>(block) + sizeof(Block));
}
