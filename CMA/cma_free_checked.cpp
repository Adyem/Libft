#include "CMA.hpp"
#include "CMA_internal.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <cstdlib>

int cma_checked_free(void* ptr)
{
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        return (0);
    }
    if (!ptr)
        return (0);
    g_malloc_mutex.lock(THREAD_ID);
    Page* page = page_list;
    Block* found = ft_nullptr;
    while (page && !found)
    {
        Block* block = page->blocks;
        while (block)
        {
            char* data_start = reinterpret_cast<char*>(block) + sizeof(Block);
            char* data_end = data_start + block->size;
            if (reinterpret_cast<char*>(ptr) >= data_start &&
                reinterpret_cast<char*>(ptr) < data_end)
            {
                found = block;
                break;
            }
            block = block->next;
        }
        page = page->next;
    }
    if (!found)
    {
        g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = CMA_INVALID_PTR;
        return (-1);
    }
    found->free = true;
    merge_block(found);
    g_malloc_mutex.unlock(THREAD_ID);
    return (0);
}
