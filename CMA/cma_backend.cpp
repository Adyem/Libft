#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <cstdlib>
#include <cstring>

static cma_backend_hooks g_cma_backend_hooks = {ft_nullptr, ft_nullptr,
    ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr};
static bool g_cma_backend_enabled = false;

static void cma_backend_set_error(int *error_code, int value)
{
    if (error_code != ft_nullptr)
        *error_code = value;
    return ;
}

static ft_size_t cma_backend_query_size(const void *memory_pointer)
{
    if (!g_cma_backend_hooks.get_allocation_size)
        return (0);
    return (g_cma_backend_hooks.get_allocation_size(memory_pointer,
            g_cma_backend_hooks.user_data));
}

static int cma_backend_query_ownership(const void *memory_pointer)
{
    if (!g_cma_backend_hooks.owns_allocation)
        return (0);
    if (!memory_pointer)
        return (0);
    return (g_cma_backend_hooks.owns_allocation(memory_pointer,
            g_cma_backend_hooks.user_data));
}

static void cma_backend_track_allocation(ft_size_t allocation_size)
{
    bool lock_acquired = false;
    int lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    g_cma_allocation_count++;
    if (allocation_size != 0)
    {
        g_cma_current_bytes += allocation_size;
        if (g_cma_current_bytes > g_cma_peak_bytes)
            g_cma_peak_bytes = g_cma_current_bytes;
    }
    cma_unlock_allocator(lock_acquired);
    return ;
}

static void cma_backend_track_free(ft_size_t allocation_size)
{
    bool lock_acquired = false;
    int lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    if (allocation_size != 0)
    {
        if (g_cma_current_bytes >= allocation_size)
            g_cma_current_bytes -= allocation_size;
        else
            g_cma_current_bytes = 0;
    }
    g_cma_free_count++;
    cma_unlock_allocator(lock_acquired);
    return ;
}

int cma_set_backend(const cma_backend_hooks *hooks)
{
    int error_code;

    if (!hooks || !hooks->allocate || !hooks->deallocate
        || !hooks->get_allocation_size || !hooks->owns_allocation)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    g_cma_backend_hooks = *hooks;
    g_cma_backend_enabled = true;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

void cma_clear_backend(void)
{
    int error_code;

    std::memset(&g_cma_backend_hooks, 0, sizeof(g_cma_backend_hooks));
    g_cma_backend_enabled = false;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return ;
}

int cma_backend_is_enabled(void)
{
    int error_code;

    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    if (g_cma_backend_enabled)
        return (1);
    return (0);
}

int cma_backend_owns_pointer(const void *memory_pointer)
{
    if (!g_cma_backend_enabled)
        return (0);
    return (cma_backend_query_ownership(memory_pointer));
}

void *cma_backend_allocate(ft_size_t size, int *error_code)
{
    if (!g_cma_backend_enabled)
    {
        cma_backend_set_error(error_code, FT_ERR_INVALID_STATE);
        return (ft_nullptr);
    }
    if (!g_cma_backend_hooks.allocate)
    {
        cma_backend_set_error(error_code, FT_ERR_INVALID_STATE);
        return (ft_nullptr);
    }
    if (size == 0)
        size = 1;
    void *memory_pointer = g_cma_backend_hooks.allocate(size,
            g_cma_backend_hooks.user_data);
    if (!memory_pointer)
    {
        cma_backend_set_error(error_code, FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    cma_backend_track_allocation(allocation_size);
    cma_backend_set_error(error_code, FT_ERR_SUCCESSS);
    return (memory_pointer);
}

int cma_backend_deallocate(void *memory_pointer)
{
    if (!memory_pointer)
        return (FT_ERR_SUCCESSS);
    if (!g_cma_backend_enabled)
        return (FT_ERR_INVALID_STATE);
    if (!g_cma_backend_hooks.deallocate)
        return (FT_ERR_INVALID_STATE);
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    cma_backend_track_free(allocation_size);
    g_cma_backend_hooks.deallocate(memory_pointer, g_cma_backend_hooks.user_data);
    return (FT_ERR_SUCCESSS);
}

void *cma_backend_aligned_allocate(ft_size_t alignment, ft_size_t size,
        int *error_code)
{
    if (!g_cma_backend_enabled)
    {
        cma_backend_set_error(error_code, FT_ERR_INVALID_STATE);
        return (ft_nullptr);
    }
    if (g_cma_backend_hooks.aligned_allocate)
    {
        void *memory_pointer = g_cma_backend_hooks.aligned_allocate(alignment,
                size, g_cma_backend_hooks.user_data);
        if (!memory_pointer)
        {
            cma_backend_set_error(error_code, FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
        cma_backend_track_allocation(allocation_size);
        cma_backend_set_error(error_code, FT_ERR_SUCCESSS);
        return (memory_pointer);
    }
    return (cma_backend_allocate(size, error_code));
}

static void cma_backend_update_stats_for_resize(ft_size_t previous_size,
        ft_size_t new_size)
{
    bool lock_acquired = false;
    int lock_error = cma_lock_allocator(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
        return ;
    if (previous_size != 0)
    {
        if (g_cma_current_bytes >= previous_size)
            g_cma_current_bytes -= previous_size;
        else
            g_cma_current_bytes = 0;
    }
    if (new_size != 0)
    {
        g_cma_current_bytes += new_size;
        if (g_cma_current_bytes > g_cma_peak_bytes)
            g_cma_peak_bytes = g_cma_current_bytes;
    }
    cma_unlock_allocator(lock_acquired);
    return ;
}

void *cma_backend_reallocate(void *memory_pointer, ft_size_t size,
        int *error_code)
{
    if (!g_cma_backend_enabled)
    {
        cma_backend_set_error(error_code, FT_ERR_INVALID_STATE);
        return (ft_nullptr);
    }
    if (!memory_pointer)
        return (cma_backend_allocate(size, error_code));
    if (size == 0)
    {
        cma_backend_set_error(error_code, cma_backend_deallocate(memory_pointer));
        return (ft_nullptr);
    }
    ft_size_t previous_size = cma_backend_query_size(memory_pointer);
    if (g_cma_backend_hooks.reallocate)
    {
        void *new_pointer = g_cma_backend_hooks.reallocate(memory_pointer,
                size, g_cma_backend_hooks.user_data);
        if (!new_pointer)
        {
            cma_backend_set_error(error_code, FT_ERR_NO_MEMORY);
            return (ft_nullptr);
        }
        ft_size_t new_size_value = cma_backend_query_size(new_pointer);
        cma_backend_update_stats_for_resize(previous_size, new_size_value);
        cma_backend_set_error(error_code, FT_ERR_SUCCESSS);
        return (new_pointer);
    }
    void *new_pointer = cma_backend_allocate(size, error_code);
    if (!new_pointer)
        return (ft_nullptr);
    ft_size_t copy_size = previous_size;
    if (copy_size > size)
        copy_size = size;
    if (copy_size != 0)
        ft_memcpy(new_pointer, memory_pointer, static_cast<size_t>(copy_size));
    cma_backend_set_error(error_code, cma_backend_deallocate(memory_pointer));
    return (new_pointer);
}

ft_size_t cma_backend_block_size(const void *memory_pointer)
{
    int error_code;

    if (!g_cma_backend_enabled)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!memory_pointer)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (!cma_backend_query_ownership(memory_pointer))
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (allocation_size);
}

int cma_backend_checked_block_size(const void *memory_pointer,
        ft_size_t *block_size)
{
    int error_code;

    if (!block_size)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    *block_size = 0;
    if (!g_cma_backend_enabled)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (!memory_pointer)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (!cma_backend_query_ownership(memory_pointer))
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    *block_size = cma_backend_query_size(memory_pointer);
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
