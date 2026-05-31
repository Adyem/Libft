#include <cstdlib>
#include <cstring>
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

int32_t    scma_initialize(ft_size_t initial_capacity)
{
    int32_t thread_safety_result;
    unsigned char *&heap_data = scma_heap_data_ref();
    scma_block *&blocks_data = scma_blocks_data_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();
    ft_size_t &block_capacity = scma_block_capacity_ref();
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    int32_t &initialised = scma_initialised_ref();

    thread_safety_result = scma_enable_thread_safety();
    if (thread_safety_result != FT_ERR_SUCCESS)
        return (static_cast<uint32_t>(thread_safety_result));
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (initialised)
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_ALREADY_INITIALISED)));
    }
    if (initial_capacity == 0)
        initial_capacity = 1024;
    if (initial_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    }
    if (heap_data)
    {
        std::free(heap_data);
        heap_data = ft_nullptr;
    }
    heap_capacity = 0;
    heap_data = static_cast<unsigned char *>(std::malloc(initial_capacity));
    if (!heap_data)
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_NO_MEMORY)));
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
    initialised = 1;
    return (static_cast<uint32_t>(scma_unlock_and_return_int(FT_ERR_SUCCESS)));
}

void    scma_shutdown(void)
{
    unsigned char *&heap_data = scma_heap_data_ref();
    scma_block *&blocks_data = scma_blocks_data_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();
    ft_size_t &block_capacity = scma_block_capacity_ref();
    ft_size_t &block_count = scma_block_count_ref();
    ft_size_t &used_size = scma_used_size_ref();
    int32_t &initialised = scma_initialised_ref();

    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return ;
    }
    if (!initialised)
    {
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
    initialised = 0;
    scma_unlock_and_return_void();
    return ;
}

int32_t    scma_is_initialised(void)
{
    int32_t initialised;

    initialised = 0;
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (0);
    }
    if (scma_initialised_ref())
    {
        initialised = 1;
    }
    return (scma_unlock_and_return_int(initialised));
}

scma_handle    scma_allocate(ft_size_t size)
{
    scma_handle result_handle;
    ft_size_t required_size;
    scma_block_span span;
    ft_size_t index;
    int32_t found_slot;
    scma_block *block;
    ft_size_t &used_size = scma_used_size_ref();
    ft_size_t &block_count = scma_block_count_ref();

    result_handle = scma_invalid_handle();
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (result_handle);
    }
    if (!scma_initialised_ref())
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (size == 0)
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    scma_compact();
    if (size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    required_size = used_size + size;
    if (required_size < used_size)
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (required_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    if (!scma_ensure_capacity(required_size))
    {
        return (scma_unlock_and_return_handle(result_handle));
    }
    span = scma_get_block_span();
    index = 0;
    found_slot = 0;
    while (index < span.count)
    {
        scma_block *candidate;

        candidate = &span.data[index];
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
            return (scma_unlock_and_return_handle(result_handle));
        }
        new_index = block_count;
        block = &blocks_data[new_index];
        block->offset = used_size;
        block->size = size;
        block->in_use = 1;
        block->generation = 1;
#ifdef LIBFT_TEST_BUILD
        block->leak_ignored = FT_FALSE;
        scma_capture_leak_stack(block, 2);
#endif
        block_count = block_count + 1;
        used_size += size;
        result_handle.index = new_index;
        result_handle.generation = block->generation;
        return (scma_unlock_and_return_handle(result_handle));
    }
    block = &span.data[index];
    block->offset = used_size;
    block->size = size;
    block->in_use = 1;
    block->generation = scma_next_generation(block->generation);
#ifdef LIBFT_TEST_BUILD
    block->leak_ignored = FT_FALSE;
    scma_capture_leak_stack(block, 2);
#endif
    result_handle.index = index;
    result_handle.generation = block->generation;
    used_size += size;
    return (scma_unlock_and_return_handle(result_handle));
}

int32_t    scma_free(scma_handle handle)
{
    scma_block *block;

    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (!scma_validate_handle(handle, &block))
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_HANDLE)));
    }
    block->in_use = 0;
    block->size = 0;
    block->generation = scma_next_generation(block->generation);
#ifdef LIBFT_TEST_BUILD
    block->leak_ignored = FT_FALSE;
    block->leak_stack_frame_count = 0;
#endif
    scma_compact();
    return (static_cast<uint32_t>(scma_unlock_and_return_int(FT_ERR_SUCCESS)));
}

int32_t    scma_resize(scma_handle handle, ft_size_t new_size)
{
    scma_block *block;
    ft_size_t old_size;
    unsigned char *temp_buffer;
    ft_size_t base_size;
    ft_size_t required_size;
    ft_size_t &used_size = scma_used_size_ref();

    temp_buffer = ft_nullptr;
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (new_size == 0)
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_ARGUMENT)));
    }
    if (!scma_validate_handle(handle, &block))
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_HANDLE)));
    }
    old_size = block->size;
    if (old_size > 0)
        temp_buffer = static_cast<unsigned char *>(std::malloc(old_size));
    if (old_size != 0 && !temp_buffer)
    {
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_NO_MEMORY)));
    }
    if (old_size != 0)
    {
        unsigned char *heap_data;

        heap_data = scma_get_heap_data();
        std::memcpy(temp_buffer,
            heap_data + block->offset,
            old_size);
    }
    if (new_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    }
    if (used_size < old_size)
    {
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_INVALID_STATE)));
    }
    base_size = used_size - old_size;
    if (base_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - new_size)
    {
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_OUT_OF_RANGE)));
    }
    required_size = base_size + new_size;
    if (!scma_ensure_capacity(required_size))
    {
        if (temp_buffer)
        {
            std::free(temp_buffer);
            temp_buffer = ft_nullptr;
        }
        return (static_cast<uint32_t>(scma_unlock_and_return_int(
                    FT_ERR_NO_MEMORY)));
    }
    block->in_use = 0;
    scma_compact();
    block->offset = used_size;
    block->size = new_size;
    block->in_use = 1;
    used_size += new_size;
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
            std::memcpy(heap_data + block->offset,
                temp_buffer, copy_size);
        }
        std::free(temp_buffer);
        temp_buffer = ft_nullptr;
    }
    return (static_cast<uint32_t>(scma_unlock_and_return_int(FT_ERR_SUCCESS)));
}

ft_size_t    scma_get_size(scma_handle handle)
{
    ft_size_t size_result;
    scma_block *block;

    size_result = 0;
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
    {
        return (scma_unlock_and_return_size(0));
    }
    size_result = block->size;
    return (scma_unlock_and_return_size(size_result));
}

int32_t    scma_handle_is_valid(scma_handle handle)
{
    int32_t valid;

    valid = 0;
    if (scma_mutex_lock() != FT_ERR_SUCCESS)
    {
        return (0);
    }
    if (scma_validate_handle(handle, ft_nullptr))
    {
        valid = 1;
        return (scma_unlock_and_return_int(valid));
    }
    return (scma_unlock_and_return_int(valid));
}
