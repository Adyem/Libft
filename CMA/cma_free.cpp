#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <csignal>
#include <pthread.h>
#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../Printf/printf.hpp"

void cma_free(void* ptr)
{
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        return ;
    }
	if (!ptr)
        return ;
	g_malloc_mutex.lock(THREAD_ID);
    Block* block = reinterpret_cast<Block*>((static_cast<char*> (ptr)
				- sizeof(Block)));
    if (block->magic != MAGIC_NUMBER)
	{
		pf_printf_fd(2, "Invalid block detected in cma_free. \n");
		print_block_info(block);
        raise(SIGABRT);
	}
    block->free = true;
    merge_block(block);
	g_malloc_mutex.unlock(THREAD_ID);
	return ;
}
