#include <cstddef>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"
#include "CMA.hpp"
#include "cma_internal.hpp"

ft_size_t cma_block_size(const void *memory_pointer)
{
    ft_size_t block_size;
    int error_code;

    if (memory_pointer == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (cma_backend_is_enabled())
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    block_size = 0;
    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    ft_global_error_stack_pop_newest();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (block_size);
}

int cma_checked_block_size(const void *memory_pointer, ft_size_t *block_size)
{
    int error_code;
    int result_code;

    if (block_size == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    *block_size = 0;
    if (memory_pointer == ft_nullptr)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (cma_backend_is_enabled() && cma_backend_owns_pointer(memory_pointer))
    {
        result_code = cma_backend_checked_block_size(memory_pointer, block_size);
        error_code = ft_global_error_stack_pop_newest();
        if (result_code != 0 && error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (result_code);
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
    Block *block = cma_find_block_for_pointer(memory_pointer);

    if (block == ft_nullptr
        || block->magic != MAGIC_NUMBER_ALLOCATED
        || cma_block_is_free(block))
    {
        int error_code;

        error_code = FT_ERR_INVALID_POINTER;
        allocator_guard.unlock();
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    *block_size = cma_block_user_size(block);
    allocator_guard.unlock();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

ft_size_t cma_alloc_size(const void *memory_pointer)
{
    ft_size_t block_size = 0;
    int error_code;

    if (cma_checked_block_size(memory_pointer, &block_size) != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    ft_global_error_stack_pop_newest();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (block_size);
}
