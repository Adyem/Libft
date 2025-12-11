#include "CMA.hpp"
#include "cma_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <cstdlib>
#include <cstring>

static cma_backend_hooks g_cma_backend_hooks = {ft_nullptr, ft_nullptr,
    ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr};
static bool g_cma_backend_enabled = false;

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
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    g_cma_allocation_count++;
    if (allocation_size != 0)
    {
        g_cma_current_bytes += allocation_size;
        if (g_cma_current_bytes > g_cma_peak_bytes)
            g_cma_peak_bytes = g_cma_current_bytes;
    }
    ft_errno = FT_ERR_SUCCESSS;
    allocator_guard.unlock();
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

static void cma_backend_track_free(ft_size_t allocation_size)
{
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
        return ;
    if (allocation_size != 0)
    {
        if (g_cma_current_bytes >= allocation_size)
            g_cma_current_bytes -= allocation_size;
        else
            g_cma_current_bytes = 0;
    }
    g_cma_free_count++;
    ft_errno = FT_ERR_SUCCESSS;
    allocator_guard.unlock();
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int cma_set_backend(const cma_backend_hooks *hooks)
{
    if (!hooks || !hooks->allocate || !hooks->deallocate
        || !hooks->get_allocation_size || !hooks->owns_allocation)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    g_cma_backend_hooks = *hooks;
    g_cma_backend_enabled = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void cma_clear_backend(void)
{
    std::memset(&g_cma_backend_hooks, 0, sizeof(g_cma_backend_hooks));
    g_cma_backend_enabled = false;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int cma_backend_is_enabled(void)
{
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

void *cma_backend_allocate(ft_size_t size)
{
    if (!g_cma_backend_enabled)
        return (ft_nullptr);
    if (!g_cma_backend_hooks.allocate)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (ft_nullptr);
    }
    if (size == 0)
        size = 1;
    void *memory_pointer = g_cma_backend_hooks.allocate(size,
            g_cma_backend_hooks.user_data);
    if (!memory_pointer)
    {
        if (ft_errno == FT_ERR_SUCCESSS)
            ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    cma_backend_track_allocation(allocation_size);
    cma_leak_tracker_record_allocation(memory_pointer, allocation_size);
    ft_errno = FT_ERR_SUCCESSS;
    return (memory_pointer);
}

void cma_backend_deallocate(void *memory_pointer)
{
    if (!memory_pointer)
        return ;
    if (!g_cma_backend_enabled)
        return ;
    if (!g_cma_backend_hooks.deallocate)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return ;
    }
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    cma_leak_tracker_record_free(memory_pointer);
    cma_backend_track_free(allocation_size);
    g_cma_backend_hooks.deallocate(memory_pointer, g_cma_backend_hooks.user_data);
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void *cma_backend_aligned_allocate(ft_size_t alignment, ft_size_t size)
{
    if (!g_cma_backend_enabled)
        return (ft_nullptr);
    if (g_cma_backend_hooks.aligned_allocate)
    {
        void *memory_pointer = g_cma_backend_hooks.aligned_allocate(alignment,
                size, g_cma_backend_hooks.user_data);
        if (!memory_pointer)
        {
            if (ft_errno == FT_ERR_SUCCESSS)
                ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
        cma_backend_track_allocation(allocation_size);
        cma_leak_tracker_record_allocation(memory_pointer, allocation_size);
        ft_errno = FT_ERR_SUCCESSS;
        return (memory_pointer);
    }
    return (cma_backend_allocate(size));
}

static void cma_backend_update_stats_for_resize(ft_size_t previous_size,
        ft_size_t new_size)
{
    cma_allocator_guard allocator_guard;

    if (!allocator_guard.is_active())
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
    ft_errno = FT_ERR_SUCCESSS;
    allocator_guard.unlock();
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void *cma_backend_reallocate(void *memory_pointer, ft_size_t size)
{
    if (!g_cma_backend_enabled)
        return (ft_nullptr);
    if (!memory_pointer)
        return (cma_backend_allocate(size));
    if (size == 0)
    {
        cma_backend_deallocate(memory_pointer);
        ft_errno = FT_ERR_SUCCESSS;
        return (ft_nullptr);
    }
    ft_size_t previous_size = cma_backend_query_size(memory_pointer);
    if (g_cma_backend_hooks.reallocate)
    {
        void *new_pointer = g_cma_backend_hooks.reallocate(memory_pointer,
                size, g_cma_backend_hooks.user_data);
        if (!new_pointer)
        {
            if (ft_errno == FT_ERR_SUCCESSS)
                ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        ft_size_t new_size_value = cma_backend_query_size(new_pointer);
        cma_backend_update_stats_for_resize(previous_size, new_size_value);
        cma_leak_tracker_record_free(memory_pointer);
        cma_leak_tracker_record_allocation(new_pointer, new_size_value);
        ft_errno = FT_ERR_SUCCESSS;
        return (new_pointer);
    }
    void *new_pointer = cma_backend_allocate(size);
    if (!new_pointer)
        return (ft_nullptr);
    ft_size_t copy_size = previous_size;
    if (copy_size > size)
        copy_size = size;
    if (copy_size != 0)
        ft_memcpy(new_pointer, memory_pointer, static_cast<size_t>(copy_size));
    cma_backend_deallocate(memory_pointer);
    ft_errno = FT_ERR_SUCCESSS;
    return (new_pointer);
}

ft_size_t cma_backend_block_size(const void *memory_pointer)
{
    if (!g_cma_backend_enabled)
        return (0);
    if (!memory_pointer)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (!cma_backend_query_ownership(memory_pointer))
        return (0);
    ft_size_t allocation_size = cma_backend_query_size(memory_pointer);
    ft_errno = FT_ERR_SUCCESSS;
    return (allocation_size);
}

int cma_backend_checked_block_size(const void *memory_pointer,
        ft_size_t *block_size)
{
    if (!block_size)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *block_size = 0;
    if (!g_cma_backend_enabled)
        return (-1);
    if (!memory_pointer)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (!cma_backend_query_ownership(memory_pointer))
        return (-1);
    *block_size = cma_backend_query_size(memory_pointer);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}
