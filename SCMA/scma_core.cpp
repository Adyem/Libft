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

void    scma_mutex_lock_guard::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

scma_mutex_lock_guard::scma_mutex_lock_guard(void)
{
    this->_owns_lock = false;
    this->_entered = false;
    this->_error_code = ER_SUCCESS;
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    if (lock_depth == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (lock_depth == 0)
    {
        pt_mutex &mutex = scma_runtime_mutex();
        mutex.lock(THREAD_ID);
        if (mutex.get_error() != ER_SUCCESS)
        {
            this->set_error(mutex.get_error());
            return ;
        }
        this->_owns_lock = true;
    }
    lock_depth = lock_depth + 1;
    this->_entered = true;
    this->_error_code = ER_SUCCESS;
    return ;
}

scma_mutex_lock_guard::~scma_mutex_lock_guard(void)
{
    if (!this->_entered)
        return ;
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    if (lock_depth == 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    lock_depth = lock_depth - 1;
    if (lock_depth == 0 && this->_owns_lock)
    {
        pt_mutex &mutex = scma_runtime_mutex();
        mutex.unlock(THREAD_ID);
        if (mutex.get_error() != ER_SUCCESS)
        {
            this->set_error(mutex.get_error());
            return ;
        }
        this->_owns_lock = false;
    }
    this->_error_code = ER_SUCCESS;
    return ;
}

bool    scma_mutex_lock_guard::owns_lock(void) const
{
    return (this->_owns_lock);
}

int     scma_mutex_lock_guard::get_error(void) const
{
    return (this->_error_code);
}

const char  *scma_mutex_lock_guard::get_error_str(void) const
{
    return (ft_strerror(this->_error_code));
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
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return ;
    if (!g_scma_initialized)
        return ;
    scma_block_span span;
    unsigned char *heap_data;
    ft_size_t new_offset;
    ft_size_t index;

    span = scma_get_block_span();
    if (span.count == 0)
    {
        g_scma_used_size = 0;
        return ;
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
    return ;
}

static int    scma_validate_handle(scma_handle handle, scma_block **out_block)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = mutex_guard.get_error();
        return (0);
    }
    if (!g_scma_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_handle_is_invalid(handle))
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (0);
    }
    scma_block_span span;
    scma_block *block;

    span = scma_get_block_span();
    if (handle.index >= span.count)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (0);
    }
    block = &span.data[static_cast<size_t>(handle.index)];
    if (!block->in_use)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (0);
    }
    if (block->generation != handle.generation)
    {
        ft_errno = FT_ERR_INVALID_HANDLE;
        return (0);
    }
    if (out_block)
        *out_block = block;
    ft_errno = ER_SUCCESS;
    return (1);
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
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    if (g_scma_initialized)
    {
        ft_errno = FT_ERR_ALREADY_INITIALIZED;
        return (0);
    }
    if (initial_capacity == 0)
        initial_capacity = 1024;
    if (initial_capacity > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
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
        return (0);
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
    ft_errno = ER_SUCCESS;
    return (1);
}

void    scma_shutdown(void)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return ;
    if (!g_scma_initialized)
        return ;
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
    ft_errno = ER_SUCCESS;
    return ;
}

int    scma_is_initialized(void)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    if (g_scma_initialized)
        return (1);
    return (0);
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
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (scma_invalid_handle());
    if (!g_scma_initialized)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (scma_invalid_handle());
    }
    if (size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (scma_invalid_handle());
    }
    scma_compact();
    if (size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (scma_invalid_handle());
    }
    ft_size_t required_size;

    required_size = g_scma_used_size + size;
    if (required_size < g_scma_used_size)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (scma_invalid_handle());
    }
    if (required_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (scma_invalid_handle());
    }
    if (!scma_ensure_capacity(required_size))
        return (scma_invalid_handle());
    scma_block_span span;
    ft_size_t index;
    int found_slot;

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
        scma_handle handle;
        ft_size_t new_index;

        if (!scma_ensure_block_capacity(g_scma_block_count + 1))
            return (scma_invalid_handle());
        new_index = g_scma_block_count;
        new_block = &g_scma_blocks_data[static_cast<size_t>(new_index)];
        new_block->offset = g_scma_used_size;
        new_block->size = size;
        new_block->in_use = 1;
        new_block->generation = 1;
        g_scma_block_count = g_scma_block_count + 1;
        g_scma_used_size += size;
        handle.index = new_index;
        handle.generation = new_block->generation;
        ft_errno = ER_SUCCESS;
        return (handle);
    }
    scma_block *block;

    block = &span.data[static_cast<size_t>(index)];
    block->offset = g_scma_used_size;
    block->size = size;
    block->in_use = 1;
    block->generation = scma_next_generation(block->generation);
    scma_handle handle;

    handle.index = index;
    handle.generation = block->generation;
    g_scma_used_size += size;
    ft_errno = ER_SUCCESS;
    return (handle);
}

int    scma_free(scma_handle handle)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    scma_block *block;

    if (!scma_validate_handle(handle, &block))
        return (0);
    block->in_use = 0;
    block->size = 0;
    block->generation = scma_next_generation(block->generation);
    scma_compact();
    ft_errno = ER_SUCCESS;
    return (1);
}

int    scma_resize(scma_handle handle, ft_size_t new_size)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    scma_block *block;
    ft_size_t old_size;

    if (new_size == 0)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (!scma_validate_handle(handle, &block))
        return (0);
    old_size = block->size;
    unsigned char *temp_buffer;

    if (old_size > 0)
        temp_buffer = static_cast<unsigned char *>(std::malloc(static_cast<size_t>(old_size)));
    else
        temp_buffer = ft_nullptr;
    if (old_size != 0 && !temp_buffer)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
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
        if (temp_buffer)
            std::free(temp_buffer);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (g_scma_used_size < old_size)
    {
        if (temp_buffer)
            std::free(temp_buffer);
        ft_errno = FT_ERR_INVALID_STATE;
        return (0);
    }
    ft_size_t base_size;
    ft_size_t required_size;
    ft_size_t old_generation;

    base_size = g_scma_used_size - old_size;
    if (base_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - new_size)
    {
        if (temp_buffer)
            std::free(temp_buffer);
        ft_errno = FT_ERR_NO_MEMORY;
        return (0);
    }
    required_size = base_size + new_size;
    if (!scma_ensure_capacity(required_size))
    {
        if (temp_buffer)
            std::free(temp_buffer);
        return (0);
    }
    old_generation = block->generation;
    block->in_use = 0;
    scma_compact();
    block->offset = g_scma_used_size;
    block->size = new_size;
    block->in_use = 1;
    block->generation = scma_next_generation(old_generation);
    g_scma_used_size += new_size;
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
    }
    ft_errno = ER_SUCCESS;
    return (1);
}

ft_size_t    scma_get_size(scma_handle handle)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    scma_block *block;

    if (!scma_validate_handle(handle, &block))
        return (0);
    return (block->size);
}

int    scma_handle_is_valid(scma_handle handle)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = mutex_guard.get_error();
        return (0);
    }
    if (scma_validate_handle(handle, ft_nullptr))
    {
        ft_errno = ER_SUCCESS;
        return (1);
    }
    return (0);
}

int    scma_write(scma_handle handle, ft_size_t offset,
            const void *source, ft_size_t size)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    scma_block *block;
    unsigned char *heap_data;

    if (!scma_validate_handle(handle, &block))
        return (0);
    if (!source)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (0);
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (0);
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (0);
    }
    heap_data = scma_get_heap_data();
    std::memcpy(heap_data + static_cast<size_t>(block->offset + offset),
        source,
        static_cast<size_t>(size));
    ft_errno = ER_SUCCESS;
    return (1);
}

int    scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (0);
    scma_block *block;
    unsigned char *heap_data;

    if (!scma_validate_handle(handle, &block))
        return (0);
    if (!destination)
    {
        ft_errno = FT_ERR_INVALID_POINTER;
        return (0);
    }
    if (offset > block->size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (0);
    }
    if (size > block->size - offset)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (0);
    }
    heap_data = scma_get_heap_data();
    std::memcpy(destination,
        heap_data + static_cast<size_t>(block->offset + offset),
        static_cast<size_t>(size));
    ft_errno = ER_SUCCESS;
    return (1);
}

void    *scma_snapshot(scma_handle handle, ft_size_t *size)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return (ft_nullptr);
    scma_block *block;

    if (!scma_validate_handle(handle, &block))
        return (ft_nullptr);
    if (size)
        *size = block->size;
    if (block->size == 0)
        return (ft_nullptr);
    void *copy;
    unsigned char *heap_data;

    copy = std::malloc(static_cast<size_t>(block->size));
    if (!copy)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    heap_data = scma_get_heap_data();
    std::memcpy(copy,
        heap_data + static_cast<size_t>(block->offset),
        static_cast<size_t>(block->size));
    ft_errno = ER_SUCCESS;
    return (copy);
}

void    scma_debug_dump(void)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
        return ;
    if (!g_scma_initialized)
    {
        std::printf("[scma] not initialized\n");
        return ;
    }
    scma_block_span span;
    ft_size_t index;
    size_t heap_capacity;

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
    return ;
}
