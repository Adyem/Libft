#include <cstdlib>
#include <cstring>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/limits.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

static unsigned char *g_scma_heap_data = ft_nullptr;
static ft_size_t g_scma_heap_capacity = 0;
static scma_block *g_scma_blocks_data = ft_nullptr;
static ft_size_t g_scma_block_capacity = 0;
static ft_size_t g_scma_block_count = 0;
static ft_size_t g_scma_used_size = 0;
static int32_t g_scma_initialized = 0;

int32_t    &scma_initialized_ref(void)
{
    return (g_scma_initialized);
}

static scma_handle    scma_create_invalid_handle(void)
{
    scma_handle handle;

    handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    return (handle);
}

unsigned char    *&scma_heap_data_ref(void)
{
    return (g_scma_heap_data);
}

ft_size_t    &scma_heap_capacity_ref(void)
{
    return (g_scma_heap_capacity);
}

scma_block    *&scma_blocks_data_ref(void)
{
    return (g_scma_blocks_data);
}

ft_size_t    &scma_block_capacity_ref(void)
{
    return (g_scma_block_capacity);
}

ft_size_t    &scma_block_count_ref(void)
{
    return (g_scma_block_count);
}

ft_size_t    &scma_used_size_ref(void)
{
    return (g_scma_used_size);
}

scma_block_span    scma_get_block_span(void)
{
    scma_block_span span;
    ft_size_t block_count;

    block_count = scma_block_count_ref();
    span.count = block_count;
    if (span.count == 0)
    {
        span.data = ft_nullptr;
        return (span);
    }
    span.data = scma_blocks_data_ref();
    return (span);
}

unsigned char    *scma_get_heap_data(void)
{
    unsigned char *heap_data;

    heap_data = scma_heap_data_ref();
    return (heap_data);
}

scma_handle    scma_invalid_handle(void)
{
    return (scma_create_invalid_handle());
}

int32_t    scma_handle_is_invalid(scma_handle handle)
{
    if (handle.index == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (1);
    if (handle.generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (1);
    return (0);
}

void    scma_compact(void)
{
    scma_block_span span;
    unsigned char *heap_data;
    ft_size_t new_offset;
    ft_size_t index;

    if (scma_mutex_lock() != 0)
        return ;
    if (!scma_initialized_ref())
    {
        scma_unlock_and_return_void();
        return ;
    }
    span = scma_get_block_span();
    if (span.count == 0)
    {
        scma_used_size_ref() = 0;
        scma_unlock_and_return_void();
        return ;
    }
    heap_data = scma_get_heap_data();
    new_offset = 0;
    index = 0;
    while (index < span.count)
    {
        scma_block *block;

        block = &span.data[index];
        if (block->in_use)
        {
            if (block->offset != new_offset)
            {
                std::memmove(heap_data + new_offset,
                    heap_data + block->offset,
                    block->size);
                block->offset = new_offset;
            }
            new_offset += block->size;
        }
        index++;
    }
    scma_used_size_ref() = new_offset;
    scma_unlock_and_return_void();
    return ;
}

int32_t    scma_validate_handle(scma_handle handle, scma_block **out_block)
{
    int32_t validation_result;
    scma_block_span span;
    scma_block *block;

    validation_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_initialized_ref())
    {
        return (scma_unlock_and_return_int(0));
    }
    if (scma_handle_is_invalid(handle))
    {
        return (scma_unlock_and_return_int(0));
    }
    span = scma_get_block_span();
    if (handle.index >= span.count)
    {
        return (scma_unlock_and_return_int(0));
    }
    block = &span.data[handle.index];
    if (!block->in_use)
    {
        return (scma_unlock_and_return_int(0));
    }
    if (block->generation != handle.generation)
    {
        return (scma_unlock_and_return_int(0));
    }
    if (out_block)
        *out_block = block;
    validation_result = 1;
    return (scma_unlock_and_return_int(validation_result));
}

int32_t    scma_ensure_block_capacity(ft_size_t required_count)
{
    ft_size_t new_capacity;
    ft_size_t allocation_size;
    void *new_data;
    scma_block *&blocks_data = scma_blocks_data_ref();
    ft_size_t &block_capacity = scma_block_capacity_ref();

    new_capacity = block_capacity;
    if (new_capacity >= required_count)
        return (1);
    if (new_capacity == 0)
        new_capacity = required_count;
    while (new_capacity < required_count)
    {
        if (new_capacity > required_count / 2)
            new_capacity = required_count;
        else
        {
            if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / 2)
                new_capacity = required_count;
            else
                new_capacity *= 2;
        }
    }
    if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (0);
    }
    if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / sizeof(scma_block))
    {
        return (0);
    }
    allocation_size = new_capacity * sizeof(scma_block);
    new_data = std::realloc(blocks_data, allocation_size);
    if (!new_data)
    {
        return (0);
    }
    blocks_data = static_cast<scma_block *>(new_data);
    block_capacity = new_capacity;
    return (1);
}

int32_t    scma_ensure_capacity(ft_size_t required_size)
{
    ft_size_t new_capacity;
    void *new_data;
    unsigned char *&heap_data = scma_heap_data_ref();
    ft_size_t &heap_capacity = scma_heap_capacity_ref();

    new_capacity = heap_capacity;
    if (new_capacity >= required_size)
        return (1);
    if (new_capacity == 0)
        new_capacity = required_size;
    while (new_capacity < required_size)
    {
        if (new_capacity > required_size / 2)
            new_capacity = required_size;
        else
        {
            if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / 2)
                new_capacity = required_size;
            else
                new_capacity *= 2;
        }
    }
    if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (0);
    }
    new_data = std::realloc(heap_data, new_capacity);
    if (!new_data)
    {
        return (0);
    }
    heap_data = static_cast<unsigned char *>(new_data);
    heap_capacity = new_capacity;
    return (1);
}

ft_size_t    scma_next_generation(ft_size_t generation)
{
    ft_size_t next_generation;

    next_generation = generation + 1;
    if (next_generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        next_generation = 1;
    if (next_generation == 0)
        next_generation = 1;
    return (next_generation);
}

int32_t    scma_unlock_and_return_int(int32_t value)
{
    (void)scma_mutex_unlock();
    return (value);
}

ft_size_t    scma_unlock_and_return_size(ft_size_t value)
{
    (void)scma_mutex_unlock();
    return (value);
}

scma_handle    scma_unlock_and_return_handle(scma_handle value)
{
    (void)scma_mutex_unlock();
    return (value);
}

void    *scma_unlock_and_return_pointer(void *value)
{
    (void)scma_mutex_unlock();
    return (value);
}

void    scma_unlock_and_return_void(void)
{
    (void)scma_mutex_unlock();
    return ;
}
