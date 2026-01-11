#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdlib>

int cma_checked_free(void* ptr)
{
    int error_code;

    if (OFFSWITCH == 1)
    {
        std::free(ptr);
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!ptr)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
    {
        error_code = allocator_guard.get_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    Block* found = cma_find_block_for_pointer(ptr);
    if (!found)
    {
        error_code = FT_ERR_INVALID_POINTER;
        allocator_guard.unlock();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    cma_validate_block(found, "cma_checked_free", ptr);
    if (static_cast<void *>(cma_block_user_pointer(found)) != ptr)
    {
        error_code = FT_ERR_INVALID_POINTER;
        allocator_guard.unlock();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_size_t freed_size = found->size;
    cma_debug_release_allocation(found, "cma_checked_free", ptr);
    cma_mark_block_free(found);
    found = merge_block(found);
    cma_debug_initialize_block(found);
    Page *pg = find_page_of_block(found);
    free_page_if_empty(pg);
    if (g_cma_current_bytes >= freed_size)
        g_cma_current_bytes -= freed_size;
    else
        g_cma_current_bytes = 0;
    g_cma_free_count++;
    allocator_guard.unlock();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
