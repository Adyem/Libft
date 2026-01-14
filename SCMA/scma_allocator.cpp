#include <cstdlib>
#include <cstring>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/limits.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int    scma_initialize(ft_size_t initial_capacity)
{
    int initialization_result;
    int error_code;
    unsigned char *&heap_data = scma_heap_data_ref();
    scma_block *&blocks_data = scma_blocks_data_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();
    ft_size_t &block_capacity = scma_block_capacity_ref();
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    int &initialized = scma_initialized_ref();

    initialization_result = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (initialized)
    {
        error_code = FT_ERR_ALREADY_INITIALIZED;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (initial_capacity == 0)
        initial_capacity = 1024;
    if (initial_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (heap_data)
    {
        std::free(heap_data);
        heap_data = ft_nullptr;
    }
    heap_capacity = 0;
    heap_data = static_cast<unsigned char *>(std::malloc(static_cast<size_t>(initial_capacity)));
    if (!heap_data)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    heap_capacity = initial_capacity;
    if (blocks_data)
    {
        std::free(blocks_data);
        blocks_data = ft_nullptr;
    }
    block_capacity = 0;
    block_count = 0;
    used_size = 0;
    initialized = 1;
    scma_reset_live_snapshot();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    initialization_result = 1;
    return (scma_unlock_and_return_int(initialization_result));
}

void    scma_shutdown(void)
{
    unsigned char *&heap_data = scma_heap_data_ref();
    scma_block *&blocks_data = scma_blocks_data_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();
    ft_size_t &block_capacity = scma_block_capacity_ref();
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    int &initialized = scma_initialized_ref();
    int error_code;

    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return ;
    }
    if (!initialized)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        scma_unlock_and_return_void();
        return ;
    }
    if (heap_data)
    {
        std::free(heap_data);
        heap_data = ft_nullptr;
    }
    if (blocks_data)
    {
        std::free(blocks_data);
        blocks_data = ft_nullptr;
    }
    heap_capacity = 0;
    block_capacity = 0;
    block_count = 0;
    used_size = 0;
    initialized = 0;
    scma_reset_live_snapshot();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    scma_unlock_and_return_void();
    return ;
}

int    scma_is_initialized(void)
{
    int initialized;
    int error_code;

    initialized = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (scma_initialized_ref())
    {
        initialized = 1;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (scma_unlock_and_return_int(initialized));
}

scma_handle    scma_allocate(ft_size_t size)
{
    scma_handle result_handle;
    ft_size_t required_size;
    scma_block_span span;
    ft_size_t index;
    int found_slot;
    scma_block *block;
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &block_count = scma_block_count_ref();
    int error_code;

    result_handle = scma_invalid_handle();
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (result_handle);
    }
    if (!scma_initialized_ref())
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (size == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    scma_compact();
    if (size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    required_size = used_size + size;
    if (required_size < used_size)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (required_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (!scma_ensure_capacity(required_size))
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    span = scma_get_block_span();
    index = 0;
    found_slot = 0;
    while (index < span.count)
    {
        scma_block *candidate;

        candidate = &span.data[static_cast<size_t>(index)];
        if (!candidate->in_use)
        {
            found_slot = 1;
            break ;
        }
        index++;
    }
    if (!found_slot)
    {
        scma_block *&blocks_data = scma_blocks_data_ref();
        ft_size_t new_index;

        if (!scma_ensure_block_capacity(block_count + 1))
        {
            error_code = ft_global_error_stack_pop_newest();
            ft_global_error_stack_push(error_code);
            return (scma_unlock_and_return_handle(result_handle));
        }
        new_index = block_count;
        block = &blocks_data[static_cast<size_t>(new_index)];
        block->offset = used_size;
        block->size = size;
        block->in_use = 1;
        block->generation = 1;
        block_count = block_count + 1;
        used_size += size;
        result_handle.index = new_index;
        result_handle.generation = block->generation;
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_handle(result_handle));
    }
    block = &span.data[static_cast<size_t>(index)];
    block->offset = used_size;
    block->size = size;
    block->in_use = 1;
    block->generation = scma_next_generation(block->generation);
    result_handle.index = index;
    result_handle.generation = block->generation;
    used_size += size;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (scma_unlock_and_return_handle(result_handle));
}

int    scma_free(scma_handle handle)
{
    int free_result;
    scma_block *block;
    scma_live_snapshot &snapshot = scma_live_snapshot_ref();
    int error_code;

    free_result = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    block->in_use = 0;
    block->size = 0;
    block->generation = scma_next_generation(block->generation);
    if (snapshot.active && snapshot.handle.index == handle.index
        && snapshot.handle.generation == handle.generation)
        scma_reset_live_snapshot();
    scma_compact();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    free_result = 1;
    return (scma_unlock_and_return_int(free_result));
}

int    scma_resize(scma_handle handle, ft_size_t new_size)
{
    int resize_result;
    scma_block *block;
    ft_size_t old_size;
    unsigned char *temp_buffer;
    ft_size_t base_size;
    ft_size_t required_size;
    ft_size_t &used_size = scma_used_size_ref();
    scma_live_snapshot &snapshot = scma_live_snapshot_ref();
    int error_code;

    resize_result = 0;
    temp_buffer = ft_nullptr;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (new_size == 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (!scma_validate_handle(handle, &block))
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    old_size = block->size;
    if (old_size > 0)
        temp_buffer = static_cast<unsigned char *>(std::malloc(static_cast<size_t>(old_size)));
    if (old_size != 0 && !temp_buffer)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(0));
    }
    if (old_size != 0)
    {
        unsigned char *heap_data;

        heap_data = scma_get_heap_data();
        std::memcpy(temp_buffer,
            heap_data + static_cast<size_t>(block->offset),
            static_cast<size_t>(old_size));
    }
    if (new_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (scma_unlock_and_return_int(0));
    }
    if (used_size < old_size)
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_global_error_stack_push(error_code);
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (scma_unlock_and_return_int(0));
    }
    base_size = used_size - old_size;
    if (base_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - new_size)
    {
        error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (scma_unlock_and_return_int(0));
    }
    required_size = base_size + new_size;
    if (!scma_ensure_capacity(required_size))
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (scma_unlock_and_return_int(0));
    }
    block->in_use = 0;
    scma_compact();
    block->offset = used_size;
    block->size = new_size;
    block->in_use = 1;
    used_size += new_size;
    if (snapshot.active && snapshot.handle.index == handle.index
        && snapshot.handle.generation == handle.generation)
        scma_reset_live_snapshot();
    if (temp_buffer)
    {
        ft_size_t copy_size;
        unsigned char *heap_data;

        copy_size = old_size;
        if (copy_size > new_size)
            copy_size = new_size;
        if (copy_size > 0)
        {
            heap_data = scma_get_heap_data();
            std::memcpy(heap_data + static_cast<size_t>(block->offset),
                temp_buffer,
                static_cast<size_t>(copy_size));
        }
        std::free(temp_buffer);
        temp_buffer = ft_nullptr;
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    resize_result = 1;
    return (scma_unlock_and_return_int(resize_result));
}

ft_size_t    scma_get_size(scma_handle handle)
{
    ft_size_t size_result;
    scma_block *block;
    int error_code;

    size_result = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_size(0));
    }
    size_result = block->size;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (scma_unlock_and_return_size(size_result));
}

int    scma_handle_is_valid(scma_handle handle)
{
    int valid;
    int error_code;

    valid = 0;
    if (scma_mutex_lock() != 0)
    {
        error_code = ft_global_error_stack_pop_newest();
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (scma_validate_handle(handle, ft_nullptr))
    {
        valid = 1;
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (scma_unlock_and_return_int(valid));
    }
    error_code = ft_global_error_stack_pop_newest();
    ft_global_error_stack_push(error_code);
    return (scma_unlock_and_return_int(valid));
}
