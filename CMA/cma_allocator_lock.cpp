#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "cma_internal.hpp"

static pt_recursive_mutex *g_cma_allocator_mutex = ft_nullptr;

int32_t cma_enable_thread_safety(void)
{
    if (g_cma_allocator_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    g_cma_allocator_mutex = new (std::nothrow) pt_recursive_mutex();
    if (g_cma_allocator_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int32_t result = g_cma_allocator_mutex->initialize();
    if (result != FT_ERR_SUCCESSS)
    {
        delete g_cma_allocator_mutex;
        g_cma_allocator_mutex = ft_nullptr;
        return (result);
    }
    return (FT_ERR_SUCCESSS);
}

int32_t cma_disable_thread_safety(void)
{
    if (g_cma_allocator_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int32_t result = g_cma_allocator_mutex->destroy();
    delete g_cma_allocator_mutex;
    g_cma_allocator_mutex = ft_nullptr;
    return (result);
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

int32_t cma_lock_allocator(bool *lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_INVALID_ARGUMENT);
    *lock_acquired = false;
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_INITIALIZATION_FAILED);
    int32_t mutex_error = mutex_pointer->lock();
    if (mutex_error != FT_ERR_SUCCESSS)
        return (FT_ERR_INVALID_STATE);
    if (cma_metadata_make_writable() != 0)
    {
        mutex_pointer->unlock();
        return (FT_ERR_INVALID_STATE);
    }
    bool guard_incremented = cma_metadata_guard_increment();
    if (!guard_incremented)
    {
        mutex_pointer->unlock();
        return (FT_ERR_INVALID_STATE);
    }
    *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int32_t cma_unlock_allocator(bool lock_acquired)
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESSS);
    bool guard_decremented = cma_metadata_guard_decrement();
    pt_recursive_mutex *mutex_pointer = cma_allocator_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_INITIALIZATION_FAILED);
    int32_t mutex_error = mutex_pointer->unlock();
    if (!guard_decremented)
    {
        if (mutex_error != FT_ERR_SUCCESSS)
            return (mutex_error);
        return (FT_ERR_INVALID_STATE);
    }
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    return (FT_ERR_SUCCESSS);
}
