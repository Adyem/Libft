#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
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
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return (-1);
    Block* found = cma_find_block_for_pointer(ptr);
    if (!found)
    {
        int error_code;

        ft_errno = FT_ERR_INVALID_POINTER;
        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (-1);
    }
    cma_validate_block(found, "cma_checked_free", ptr);
    if (static_cast<void *>(found->payload) != ptr)
    {
        int error_code;

        ft_errno = FT_ERR_INVALID_POINTER;
        error_code = ft_errno;
        allocator_guard.unlock();
        ft_errno = error_code;
        return (-1);
    }
    ft_size_t freed_size = found->size;
    cma_mark_block_free(found);
    found = merge_block(found);
    Page *pg = find_page_of_block(found);
    free_page_if_empty(pg);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    ft_errno = ER_SUCCESS;
    allocator_guard.unlock();
    ft_errno = ER_SUCCESS;
    return (0);
}
