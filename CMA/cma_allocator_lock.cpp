#include <new>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "cma_internal.hpp"

static pt_recursive_mutex *g_cma_allocator_mutex = ft_nullptr;

int cma_enable_thread_safety(void)
{
    if (g_cma_allocator_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = pt_recursive_mutex_create_with_error(&g_cma_allocator_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

void cma_disable_thread_safety(void)
{
    pt_recursive_mutex_destroy(&g_cma_allocator_mutex);
    return ;
}

bool cma_is_thread_safe_enabled(void)
{
    return (g_cma_allocator_mutex != ft_nullptr);
}

static pt_recursive_mutex *cma_allocator_mutex(void)
{
    if (g_cma_allocator_mutex == ft_nullptr)
    {
        if (cma_enable_thread_safety() != FT_ERR_SUCCESSS)
            return (ft_nullptr);
    }
    return (g_cma_allocator_mutex);
}

int cma_lock_allocator(bool *lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_INVALID_ARGUMENT);
    *lock_acquired = false;
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    int mutex_error = pt_recursive_mutex_lock_with_error(*mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        return (FT_ERR_INVALID_STATE);
    }
    if (cma_metadata_make_writable() != 0)
    {
        pt_recursive_mutex_unlock_with_error(*mutex_pointer);
        return (FT_ERR_INVALID_STATE);
    }
    bool guard_incremented = cma_metadata_guard_increment();
    if (!guard_incremented)
    {
        pt_recursive_mutex_unlock_with_error(*mutex_pointer);
        return (FT_ERR_INVALID_STATE);
    }
    *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int cma_unlock_allocator(bool lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESSS);
    bool guard_decremented = cma_metadata_guard_decrement();
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        return (FT_ERR_INITIALIZATION_FAILED);
    }
    int mutex_error = pt_recursive_mutex_unlock_with_error(*mutex_pointer);
    if (!guard_decremented || mutex_error != FT_ERR_SUCCESSS)
        return (FT_ERR_INVALID_STATE);
    return (FT_ERR_SUCCESSS);
}
