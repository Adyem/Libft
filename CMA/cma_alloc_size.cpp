#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

static Block    *cma_get_block_from_pointer(const void *memory_pointer)
{
    return (reinterpret_cast<Block*>(const_cast<char*>(
        static_cast<const char*>(memory_pointer)) - sizeof(Block)));
}

static Block    *cma_find_block_for_pointer(const void *memory_pointer)
{
    Page *current_page = page_list;

    while (current_page)
    {
        Block *current_block = current_page->blocks;

        while (current_block)
        {
            char *data_start = reinterpret_cast<char*>(current_block) + sizeof(Block);

            if (reinterpret_cast<const char*>(memory_pointer) == data_start)
                return (current_block);
            current_block = current_block->next;
        }
        current_page = current_page->next;
    }
    return (ft_nullptr);
}

ft_size_t cma_block_size(const void *memory_pointer)
{
    if (memory_pointer == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    Block *block = cma_get_block_from_pointer(memory_pointer);

    if (block->magic != MAGIC_NUMBER)
    {
        pf_printf_fd(2, "Invalid block detected in cma_block_size.\n");
        print_block_info(block);
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        su_sigabrt();
    }
    ft_size_t block_size = block->size;

    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    return (block_size);
}

int cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size)
{
    if (block_size == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    *block_size = 0;
    if (memory_pointer == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    if (g_cma_thread_safe)
        g_malloc_mutex.lock(THREAD_ID);
    Block *block = cma_find_block_for_pointer(memory_pointer);

    if (block == ft_nullptr || block->magic != MAGIC_NUMBER)
    {
        if (g_cma_thread_safe)
            g_malloc_mutex.unlock(THREAD_ID);
        ft_errno = CMA_INVALID_PTR;
        return (-1);
    }
    *block_size = block->size;
    if (g_cma_thread_safe)
        g_malloc_mutex.unlock(THREAD_ID);
    ft_errno = ER_SUCCESS;
    return (0);
}

ft_size_t cma_alloc_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;

    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
        return (0);
    return (block_size);
}
