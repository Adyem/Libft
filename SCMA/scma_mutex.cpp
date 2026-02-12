#include <cstddef>
#include <new>
#include <mutex>
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

static pt_recursive_mutex    *g_scma_mutex = ft_nullptr;
static std::mutex g_scma_mutex_control;
static thread_local ft_size_t g_scma_lock_depth = 0;
static ft_size_t g_scma_mutex_users = 0;

int32_t scma_enable_thread_safety(void)
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialization_error;

    g_scma_mutex_control.lock();
    if (g_scma_mutex != ft_nullptr)
    {
        g_scma_mutex_control.unlock();
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        g_scma_mutex_control.unlock();
        return (FT_ERR_NO_MEMORY);
    }
    initialization_error = mutex_pointer->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        g_scma_mutex_control.unlock();
        return (initialization_error);
    }
    g_scma_mutex = mutex_pointer;
    g_scma_mutex_control.unlock();
    return (FT_ERR_SUCCESS);
}

int32_t scma_disable_thread_safety(void)
{
    int32_t destroy_error;

    g_scma_mutex_control.lock();
    if (g_scma_mutex == ft_nullptr)
    {
        g_scma_mutex_control.unlock();
        return (FT_ERR_SUCCESS);
    }
    if (g_scma_mutex_users != 0
        || g_scma_lock_depth != 0
        || g_scma_mutex->lockState())
    {
        g_scma_mutex_control.unlock();
        return (FT_ERR_THREAD_BUSY);
    }
    destroy_error = g_scma_mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
    {
        g_scma_mutex_control.unlock();
        return (destroy_error);
    }
    delete g_scma_mutex;
    g_scma_mutex = ft_nullptr;
    g_scma_mutex_control.unlock();
    return (FT_ERR_SUCCESS);
}

bool scma_is_thread_safe_enabled(void)
{
    bool is_enabled;

    g_scma_mutex_control.lock();
    is_enabled = (g_scma_mutex != ft_nullptr);
    g_scma_mutex_control.unlock();
    return (is_enabled);
}

static pt_recursive_mutex *scma_runtime_mutex_ptr(void)
{
    return (g_scma_mutex);
}

pt_recursive_mutex    *scma_runtime_mutex(void)
{
    pt_recursive_mutex *mutex_pointer;

    g_scma_mutex_control.lock();
    mutex_pointer = scma_runtime_mutex_ptr();
    g_scma_mutex_control.unlock();
    return (mutex_pointer);
}

static ft_size_t    &scma_runtime_lock_depth(void)
{
    return (g_scma_lock_depth);
}

int32_t    scma_mutex_lock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    if (lock_depth == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        return (-1);
    }
    if (lock_depth == 0)
    {
        g_scma_mutex_control.lock();
        mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            g_scma_mutex_control.unlock();
            return (-1);
        }
        g_scma_mutex_users = g_scma_mutex_users + 1;
        g_scma_mutex_control.unlock();
        mutex_error = mutex_pointer->lock();
        g_scma_mutex_control.lock();
        g_scma_mutex_users = g_scma_mutex_users - 1;
        g_scma_mutex_control.unlock();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
    }
    lock_depth = lock_depth + 1;
    return (0);
}

int32_t    scma_mutex_unlock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    if (lock_depth == 0)
    {
        return (-1);
    }
    if (lock_depth == 1)
    {
        g_scma_mutex_control.lock();
        mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            g_scma_mutex_control.unlock();
            return (-1);
        }
        g_scma_mutex_users = g_scma_mutex_users + 1;
        g_scma_mutex_control.unlock();
        mutex_error = mutex_pointer->unlock();
        g_scma_mutex_control.lock();
        g_scma_mutex_users = g_scma_mutex_users - 1;
        g_scma_mutex_control.unlock();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            return (-1);
        }
        lock_depth = 0;
        return (0);
    }
    lock_depth = lock_depth - 1;
    return (0);
}

int32_t    scma_mutex_close(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();

    if (lock_depth == 0)
    {
        return (-1);
    }
    while (lock_depth > 0)
    {
        if (scma_mutex_unlock() != 0)
            return (-1);
    }
    return (0);
}

ft_size_t    scma_mutex_lock_count(void)
{
    ft_size_t lock_depth;

    lock_depth = scma_runtime_lock_depth();
    return (lock_depth);
}
