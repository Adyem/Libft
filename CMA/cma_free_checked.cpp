#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include <cstdlib>

int cma_checked_free(void* ptr)
{
    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (!ptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (g_cma_thread_safe)
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
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    cma_validate_block(found, "cma_checked_free", ptr);
    char *expected_pointer = reinterpret_cast<char*>(found) + sizeof(Block);
    if (reinterpret_cast<char*>(ptr) != expected_pointer)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = FT_ERR_INVALID_POINTER;
        return (-1);
    }
    ft_size_t freed_size = found->size;
    found->free = true;
    found->magic = MAGIC_NUMBER;
    found = merge_block(found);
    Page *pg = find_page_of_block(found);
    free_page_if_empty(pg);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    ft_errno = ER_SUCCESS;
    return (0);
}
