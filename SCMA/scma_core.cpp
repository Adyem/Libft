#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/limits.hpp"
#include "../PThread/pthread.hpp"
#include "SCMA.hpp"

struct scma_block
{
    ft_size_t    offset;
    ft_size_t    size;
    int          in_use;
    ft_size_t    generation;
};

static unsigned char *g_scma_heap_data = ft_nullptr;
static ft_size_t g_scma_heap_capacity = 0;
static scma_block *g_scma_blocks_data = ft_nullptr;
static ft_size_t g_scma_block_capacity = 0;
static ft_size_t g_scma_block_count = 0;
static ft_size_t g_scma_used_size = 0;
static int g_scma_initialized = 0;
static pt_mutex g_scma_mutex;
static thread_local ft_size_t g_scma_lock_depth = 0;

pt_mutex    &scma_runtime_mutex(void)
{
    return (g_scma_mutex);
}

static ft_size_t    &scma_runtime_lock_depth(void)
{
    return (g_scma_lock_depth);
}

int     scma_mutex_lock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    if (lock_depth == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (-1);
    }
    if (lock_depth == 0)
    {
        pt_mutex &mutex = scma_runtime_mutex();
        mutex.lock(THREAD_ID);
        if (mutex.get_error() != ER_SUCCESS)
        {
            ft_errno = mutex.get_error();
            return (-1);
        }
    }
    lock_depth = lock_depth + 1;
    ft_errno = ER_SUCCESS;
    return (0);
}

int     scma_mutex_unlock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    if (lock_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    int previous_errno;

    previous_errno = ft_errno;
    lock_depth = lock_depth - 1;
    if (lock_depth == 0)
    {
        pt_mutex &mutex = scma_runtime_mutex();
        mutex.unlock(THREAD_ID);
        if (mutex.get_error() != ER_SUCCESS)
        {
            ft_errno = mutex.get_error();
            return (-1);
        }
    }
    ft_errno = previous_errno;
    return (0);
}

ft_size_t    scma_mutex_lock_count(void)
{
    ft_size_t lock_depth;

    lock_depth = scma_runtime_lock_depth();
    return (lock_depth);
}

struct scma_block_span
{
    scma_block *data;
    ft_size_t count;
};

static inline scma_block_span    scma_get_block_span(void)
{
    scma_block_span span;

    span.count = g_scma_block_count;
    if (span.count == 0)
    {
        span.data = ft_nullptr;
        return (span);
    }
    span.data = g_scma_blocks_data;
    return (span);
}

static inline unsigned char    *scma_get_heap_data(void)
{
    return (g_scma_heap_data);
}

static inline scma_handle    scma_invalid_handle(void)
{
    scma_handle handle;

    handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    return (handle);
}

struct scma_live_snapshot
{
    unsigned char *data;
    ft_size_t size;
    scma_handle handle;
    int active;
};

static scma_live_snapshot g_scma_live_snapshot = { ft_nullptr, 0, scma_invalid_handle(), 0 };

static void    scma_reset_live_snapshot(void)
{
    g_scma_live_snapshot.data = ft_nullptr;
    g_scma_live_snapshot.size = 0;
    g_scma_live_snapshot.handle = scma_invalid_handle();
    g_scma_live_snapshot.active = 0;
    return ;
}

static void    scma_track_live_snapshot(scma_handle handle, unsigned char *data, ft_size_t size, int active)
{
    if (!active)
    {
        scma_reset_live_snapshot();
        return ;
    }
    g_scma_live_snapshot.data = data;
    g_scma_live_snapshot.size = size;
    g_scma_live_snapshot.handle = handle;
    g_scma_live_snapshot.active = 1;
    return ;
}

static void    scma_update_tracked_snapshot(scma_handle handle, ft_size_t offset, const void *source, ft_size_t size)
{
    if (!g_scma_live_snapshot.active)
        return ;
    if (!g_scma_live_snapshot.data)
        return ;
    if (handle.index != g_scma_live_snapshot.handle.index)
        return ;
    if (handle.generation != g_scma_live_snapshot.handle.generation)
        return ;
    if (offset > g_scma_live_snapshot.size)
        return ;
    if (size > g_scma_live_snapshot.size - offset)
        return ;
    std::memcpy(g_scma_live_snapshot.data + static_cast<size_t>(offset),
        source,
        static_cast<size_t>(size));
    return ;
}

static inline int    scma_handle_is_invalid(scma_handle handle)
{
    if (handle.index == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (1);
    if (handle.generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (1);
    return (0);
}

static void    scma_compact(void)
{
    scma_block_span span;
    unsigned char *heap_data;
    ft_size_t new_offset;
    ft_size_t index;

    if (scma_mutex_lock() != 0)
        return ;
    if (!g_scma_initialized)
        goto cleanup;
    span = scma_get_block_span();
    if (span.count == 0)
    {
        g_scma_used_size = 0;
        goto cleanup;
    }
    heap_data = scma_get_heap_data();
    new_offset = 0;
    index = 0;
    while (index < span.count)
    {
        scma_block *block;

        block = &span.data[static_cast<size_t>(index)];
        if (block->in_use)
        {
            if (block->offset != new_offset)
            {
                std::memmove(heap_data + static_cast<size_t>(new_offset),
                    heap_data + static_cast<size_t>(block->offset),
                    static_cast<size_t>(block->size));
                block->offset = new_offset;
            }
            new_offset += block->size;
        }
        index++;
    }
    g_scma_used_size = new_offset;

cleanup:
    if (scma_mutex_unlock() != 0)
        return ;
    return ;
}

static int    scma_validate_handle(scma_handle handle, scma_block **out_block)
{
    int validation_result;
    scma_block_span span;
    scma_block *block;

    validation_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!g_scma_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        goto cleanup;
    }
    if (scma_handle_is_invalid(handle))
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    span = scma_get_block_span();
    if (handle.index >= span.count)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    block = &span.data[static_cast<size_t>(handle.index)];
    if (!block->in_use)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    if (block->generation != handle.generation)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    if (out_block)
        *out_block = block;
    ft_errno = ER_SUCCESS;
    validation_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
        validation_result = 0;
    return (validation_result);
}

static int    scma_ensure_block_capacity(ft_size_t required_count)
{
    ft_size_t new_capacity;
    size_t allocation_size;
    void *new_data;

    new_capacity = g_scma_block_capacity;
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
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    if (new_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / static_cast<ft_size_t>(sizeof(scma_block)))
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    allocation_size = static_cast<size_t>(new_capacity) * sizeof(scma_block);
    new_data = std::realloc(g_scma_blocks_data, allocation_size);
    if (!new_data)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    g_scma_blocks_data = static_cast<scma_block *>(new_data);
    g_scma_block_capacity = new_capacity;
    return (1);
}

static int    scma_ensure_capacity(ft_size_t required_size)
{
    ft_size_t new_capacity;
    void *new_data;

    new_capacity = g_scma_heap_capacity;
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
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    new_data = std::realloc(g_scma_heap_data, static_cast<size_t>(new_capacity));
    if (!new_data)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    g_scma_heap_data = static_cast<unsigned char *>(new_data);
    g_scma_heap_capacity = new_capacity;
    return (1);
}

int    scma_initialize(ft_size_t initial_capacity)
{
    int initialization_result;

    initialization_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (g_scma_initialized)
    {
        ft_errno = FT_ERR_ALREADY_INITIALIZED;
        goto cleanup;
    }
    if (initial_capacity == 0)
        initial_capacity = 1024;
    if (initial_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (g_scma_heap_data)
    {
        std::free(g_scma_heap_data);
        g_scma_heap_data = ft_nullptr;
    }
    g_scma_heap_capacity = 0;
    g_scma_heap_data = static_cast<unsigned char *>(std::malloc(static_cast<size_t>(initial_capacity)));
    if (!g_scma_heap_data)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        goto cleanup;
    }
    g_scma_heap_capacity = initial_capacity;
    if (g_scma_blocks_data)
    {
        std::free(g_scma_blocks_data);
        g_scma_blocks_data = ft_nullptr;
    }
    g_scma_block_capacity = 0;
    g_scma_block_count = 0;
    g_scma_used_size = 0;
    g_scma_initialized = 1;
    scma_reset_live_snapshot();
    ft_errno = ER_SUCCESS;
    initialization_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
        initialization_result = 0;
    return (initialization_result);
}

void    scma_shutdown(void)
{
    if (scma_mutex_lock() != 0)
        return ;
    if (!g_scma_initialized)
        goto cleanup;
    if (g_scma_heap_data)
    {
        std::free(g_scma_heap_data);
        g_scma_heap_data = ft_nullptr;
    }
    if (g_scma_blocks_data)
    {
        std::free(g_scma_blocks_data);
        g_scma_blocks_data = ft_nullptr;
    }
    g_scma_heap_capacity = 0;
    g_scma_block_capacity = 0;
    g_scma_block_count = 0;
    g_scma_used_size = 0;
    g_scma_initialized = 0;
    scma_reset_live_snapshot();
    ft_errno = ER_SUCCESS;

cleanup:
    if (scma_mutex_unlock() != 0)
        return ;
    return ;
}

int    scma_is_initialized(void)
{
    int initialized;

    initialized = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (g_scma_initialized)
    {
        ft_errno = ER_SUCCESS;
        initialized = 1;
    }
    else
    {
        ft_errno = ER_SUCCESS;
    }
    if (scma_mutex_unlock() != 0)
        return (0);
    return (initialized);
}

static ft_size_t    scma_next_generation(ft_size_t generation)
{
    ft_size_t next_generation;

    next_generation = generation + 1;
    if (next_generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        next_generation = 1;
    if (next_generation == 0)
        next_generation = 1;
    return (next_generation);
}

scma_handle    scma_allocate(ft_size_t size)
{
    scma_handle result_handle;
    ft_size_t required_size;
    scma_block_span span;
    ft_size_t index;
    int found_slot;
    scma_block *block;

    result_handle = scma_invalid_handle();
    if (scma_mutex_lock() != 0)
        return (result_handle);
    if (!g_scma_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        goto cleanup;
    }
    if (size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    scma_compact();
    if (size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    required_size = g_scma_used_size + size;
    if (required_size < g_scma_used_size)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        goto cleanup;
    }
    if (required_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_NO_MEMORY;
        goto cleanup;
    }
    if (!scma_ensure_capacity(required_size))
        goto cleanup;
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
        scma_block *new_block;
        ft_size_t new_index;

        if (!scma_ensure_block_capacity(g_scma_block_count + 1))
            goto cleanup;
        new_index = g_scma_block_count;
        new_block = &g_scma_blocks_data[static_cast<size_t>(new_index)];
        new_block->offset = g_scma_used_size;
        new_block->size = size;
        new_block->in_use = 1;
        new_block->generation = 1;
        g_scma_block_count = g_scma_block_count + 1;
        g_scma_used_size += size;
        result_handle.index = new_index;
        result_handle.generation = new_block->generation;
        ft_errno = ER_SUCCESS;
        goto cleanup;
    }
    block = &span.data[static_cast<size_t>(index)];
    block->offset = g_scma_used_size;
    block->size = size;
    block->in_use = 1;
    block->generation = scma_next_generation(block->generation);
    result_handle.index = index;
    result_handle.generation = block->generation;
    g_scma_used_size += size;
    ft_errno = ER_SUCCESS;

cleanup:
    if (scma_mutex_unlock() != 0)
        return (scma_invalid_handle());
    return (result_handle);
}

int    scma_free(scma_handle handle)
{
    int free_result;
    scma_block *block;

    free_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    block->in_use = 0;
    block->size = 0;
    block->generation = scma_next_generation(block->generation);
    if (g_scma_live_snapshot.active && g_scma_live_snapshot.handle.index == handle.index
        && g_scma_live_snapshot.handle.generation == handle.generation)
        scma_reset_live_snapshot();
    scma_compact();
    ft_errno = ER_SUCCESS;
    free_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
        return (0);
    return (free_result);
}

int    scma_resize(scma_handle handle, ft_size_t new_size)
{
    int resize_result;
    scma_block *block;
    ft_size_t old_size;
    unsigned char *temp_buffer;
    ft_size_t base_size;
    ft_size_t required_size;

    resize_result = 0;
    temp_buffer = ft_nullptr;
    if (scma_mutex_lock() != 0)
        return (0);
    if (new_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    old_size = block->size;
    if (old_size > 0)
        temp_buffer = static_cast<unsigned char *>(std::malloc(static_cast<size_t>(old_size)));
    if (old_size != 0 && !temp_buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        goto cleanup;
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
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (g_scma_used_size < old_size)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        goto cleanup;
    }
    base_size = g_scma_used_size - old_size;
    if (base_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - new_size)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        goto cleanup;
    }
    required_size = base_size + new_size;
    if (!scma_ensure_capacity(required_size))
        goto cleanup;
    block->in_use = 0;
    scma_compact();
    block->offset = g_scma_used_size;
    block->size = new_size;
    block->in_use = 1;
    g_scma_used_size += new_size;
    if (g_scma_live_snapshot.active && g_scma_live_snapshot.handle.index == handle.index
        && g_scma_live_snapshot.handle.generation == handle.generation)
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
    ft_errno = ER_SUCCESS;
    resize_result = 1;

cleanup:
    if (temp_buffer)
        std::free(temp_buffer);
    if (scma_mutex_unlock() != 0)
        return (0);
    return (resize_result);
}

ft_size_t    scma_get_size(scma_handle handle)
{
    ft_size_t size_result;
    scma_block *block;

    size_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    size_result = block->size;

cleanup:
    if (scma_mutex_unlock() != 0)
        return (0);
    return (size_result);
}

int    scma_handle_is_valid(scma_handle handle)
{
    int valid;

    valid = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (scma_validate_handle(handle, ft_nullptr))
    {
        ft_errno = ER_SUCCESS;
        valid = 1;
    }
    if (scma_mutex_unlock() != 0)
        return (0);
    return (valid);
}

int    scma_write(scma_handle handle, ft_size_t offset,
            const void *source, ft_size_t size)
{
    int write_result;
    scma_block *block;
    unsigned char *heap_data;

    write_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    if (!source)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        goto cleanup;
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + static_cast<size_t>(block->offset + offset),
        source,
        static_cast<size_t>(size));
    scma_update_tracked_snapshot(handle, offset, source, size);
    ft_errno = ER_SUCCESS;
    write_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
        return (0);
    return (write_result);
}

int    scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size)
{
    int read_result;
    scma_block *block;
    unsigned char *heap_data;

    read_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    if (!destination)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        goto cleanup;
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    heap_data = scma_get_heap_data();
    std::memcpy(destination,
        heap_data + static_cast<size_t>(block->offset + offset),
        static_cast<size_t>(size));
    ft_errno = ER_SUCCESS;
    read_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
        return (0);
    return (read_result);
}

void    *scma_snapshot(scma_handle handle, ft_size_t *size)
{
    scma_block *block;
    void *copy;
    void *snapshot_result;
    unsigned char *heap_data;

    copy = ft_nullptr;
    snapshot_result = ft_nullptr;
    if (scma_mutex_lock() != 0)
        return (ft_nullptr);
    if (!scma_validate_handle(handle, &block))
        goto cleanup;
    if (size)
        *size = block->size;
    if (block->size == 0)
    {
        scma_reset_live_snapshot();
        goto cleanup;
    }
    copy = std::malloc(static_cast<size_t>(block->size));
    if (!copy)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        scma_reset_live_snapshot();
        goto cleanup;
    }
    heap_data = scma_get_heap_data();
    std::memcpy(copy,
        heap_data + static_cast<size_t>(block->offset),
        static_cast<size_t>(block->size));
    if (size)
        scma_track_live_snapshot(handle, static_cast<unsigned char *>(copy), block->size, 1);
    else
        scma_track_live_snapshot(scma_invalid_handle(), ft_nullptr, 0, 0);
    ft_errno = ER_SUCCESS;
    snapshot_result = copy;
    copy = ft_nullptr;

cleanup:
    if (copy)
        std::free(copy);
    if (scma_mutex_unlock() != 0)
    {
        if (snapshot_result)
        {
            std::free(snapshot_result);
            snapshot_result = ft_nullptr;
        }
        return (ft_nullptr);
    }
    return (snapshot_result);
}

void    scma_debug_dump(void)
{
    scma_block_span span;
    ft_size_t index;
    size_t heap_capacity;

    if (scma_mutex_lock() != 0)
        return ;
    if (!g_scma_initialized)
    {
        std::printf("[scma] not initialized\n");
        goto cleanup;
    }
    span = scma_get_block_span();
    heap_capacity = static_cast<size_t>(g_scma_heap_capacity);
    std::printf("[scma] blocks=%llu used=%llu capacity=%zu\n",
        static_cast<unsigned long long>(span.count),
        static_cast<unsigned long long>(g_scma_used_size),
        heap_capacity);
    index = 0;
    while (index < span.count)
    {
        scma_block *block;

        block = &span.data[static_cast<size_t>(index)];
        std::printf("  [%llu] offset=%llu size=%llu in_use=%d generation=%llu\n",
            static_cast<unsigned long long>(index),
            static_cast<unsigned long long>(block->offset),
            static_cast<unsigned long long>(block->size),
            block->in_use,
            static_cast<unsigned long long>(block->generation));
        index++;
    }

cleanup:
    if (scma_mutex_unlock() != 0)
        return ;
    return ;
}
