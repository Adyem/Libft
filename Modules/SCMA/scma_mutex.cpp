#include <new>
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "SCMA.hpp"
#include "../PThread/mutex.hpp"

static pt_recursive_mutex    *g_scma_mutex = ft_nullptr;
static pt_recursive_mutex    *g_scma_mutex_control = ft_nullptr;
static thread_local ft_size_t g_scma_lock_depth = 0;
static ft_size_t g_scma_mutex_users = 0;

static int32_t scma_control_mutex_prepare(void)
{
    pt_recursive_mutex *created_mutex;
    int32_t initialize_result;

    if (g_scma_mutex_control != ft_nullptr)
        return (FT_ERR_SUCCESS);
    created_mutex = new (std::nothrow) pt_recursive_mutex();
    if (created_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_result = created_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete created_mutex;
        return (initialize_result);
    }
    g_scma_mutex_control = created_mutex;
    return (FT_ERR_SUCCESS);
}

static int32_t scma_control_mutex_lock(void)
{
    int32_t prepare_result;
    int32_t lock_result;

    prepare_result = scma_control_mutex_prepare();
    if (prepare_result != FT_ERR_SUCCESS)
        return (prepare_result);
    lock_result = pt_recursive_mutex_lock_if_not_null(g_scma_mutex_control);
    if (lock_result != FT_ERR_SUCCESS)
        return (lock_result);
    return (FT_ERR_SUCCESS);
}

static void scma_control_mutex_unlock(void)
{
    if (g_scma_mutex_control == ft_nullptr)
        return ;
    (void)pt_recursive_mutex_unlock_if_not_null(g_scma_mutex_control);
    return ;
}

int32_t scma_enable_thread_safety(void)
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialization_error;
    int32_t control_lock_error;

    control_lock_error = scma_control_mutex_lock();
    if (control_lock_error != FT_ERR_SUCCESS)
        return (control_lock_error);
    if (g_scma_mutex != ft_nullptr)
    {
        scma_control_mutex_unlock();
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        scma_control_mutex_unlock();
        return (FT_ERR_NO_MEMORY);
    }
    initialization_error = mutex_pointer->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        scma_control_mutex_unlock();
        return (initialization_error);
    }
    g_scma_mutex = mutex_pointer;
    scma_control_mutex_unlock();
    return (FT_ERR_SUCCESS);
}

int32_t scma_disable_thread_safety(void)
{
    pt_recursive_mutex *control_mutex_to_destroy;
    int32_t control_destroy_error;
    int32_t destroy_error;
    int32_t control_lock_error;

    control_lock_error = scma_control_mutex_lock();
    if (control_lock_error != FT_ERR_SUCCESS)
        return (control_lock_error);
    if (g_scma_mutex == ft_nullptr)
    {
        control_mutex_to_destroy = g_scma_mutex_control;
        g_scma_mutex_control = ft_nullptr;
        (void)pt_recursive_mutex_unlock_if_not_null(control_mutex_to_destroy);
        if (control_mutex_to_destroy != ft_nullptr)
        {
            control_destroy_error = control_mutex_to_destroy->destroy();
            delete control_mutex_to_destroy;
            if (control_destroy_error != FT_ERR_SUCCESS)
                return (control_destroy_error);
        }
        return (FT_ERR_SUCCESS);
    }
    if (g_scma_mutex_users != 0
        || g_scma_lock_depth != 0
        || g_scma_mutex->lockState())
    {
        scma_control_mutex_unlock();
        return (FT_ERR_THREAD_BUSY);
    }
    destroy_error = g_scma_mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
    {
        scma_control_mutex_unlock();
        return (destroy_error);
    }
    delete g_scma_mutex;
    g_scma_mutex = ft_nullptr;
    control_mutex_to_destroy = g_scma_mutex_control;
    g_scma_mutex_control = ft_nullptr;
    (void)pt_recursive_mutex_unlock_if_not_null(control_mutex_to_destroy);
    if (control_mutex_to_destroy != ft_nullptr)
    {
        control_destroy_error = control_mutex_to_destroy->destroy();
        delete control_mutex_to_destroy;
        if (control_destroy_error != FT_ERR_SUCCESS)
            return (control_destroy_error);
    }
    return (FT_ERR_SUCCESS);
}

ft_bool scma_is_thread_safe_enabled(void)
{
    ft_bool is_enabled;
    int32_t control_lock_error;

    control_lock_error = scma_control_mutex_lock();
    if (control_lock_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    is_enabled = FT_FALSE;
    if (g_scma_mutex != ft_nullptr)
        is_enabled = FT_TRUE;
    scma_control_mutex_unlock();
    return (is_enabled);
}

static pt_recursive_mutex *scma_runtime_mutex_ptr(void)
{
    return (g_scma_mutex);
}

pt_recursive_mutex    *scma_runtime_mutex(void)
{
    pt_recursive_mutex *mutex_pointer;
    int32_t control_lock_error;

    control_lock_error = scma_control_mutex_lock();
    if (control_lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    mutex_pointer = scma_runtime_mutex_ptr();
    scma_control_mutex_unlock();
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
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (lock_depth == 0)
    {
        if (scma_control_mutex_lock() != FT_ERR_SUCCESS)
            return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
        mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            scma_control_mutex_unlock();
            lock_depth = 1;
            return (FT_ERR_SUCCESS);
        }
        g_scma_mutex_users = g_scma_mutex_users + 1;
        scma_control_mutex_unlock();
        mutex_error = pt_recursive_mutex_lock_if_not_null(mutex_pointer);
        if (scma_control_mutex_lock() != FT_ERR_SUCCESS)
            return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
        g_scma_mutex_users = g_scma_mutex_users - 1;
        scma_control_mutex_unlock();
        if (mutex_error != FT_ERR_SUCCESS)
        {
            return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
        }
    }
    lock_depth = lock_depth + 1;
    return (FT_ERR_SUCCESS);
}

int32_t    scma_mutex_unlock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    pt_recursive_mutex *mutex_pointer;
    if (lock_depth == 0)
    {
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    }
    if (lock_depth == 1)
    {
        if (scma_control_mutex_lock() != FT_ERR_SUCCESS)
            return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            scma_control_mutex_unlock();
            lock_depth = 0;
            return (FT_ERR_SUCCESS);
        }
        g_scma_mutex_users = g_scma_mutex_users + 1;
        scma_control_mutex_unlock();
        (void)pt_recursive_mutex_unlock_if_not_null(mutex_pointer);
        if (scma_control_mutex_lock() != FT_ERR_SUCCESS)
            return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
        g_scma_mutex_users = g_scma_mutex_users - 1;
        scma_control_mutex_unlock();
        lock_depth = 0;
        return (FT_ERR_SUCCESS);
    }
    lock_depth = lock_depth - 1;
    return (FT_ERR_SUCCESS);
}

int32_t    scma_mutex_close(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();

    if (lock_depth == 0)
    {
        return (FT_ERR_SYS_MUTEX_UNLOCK_FAILED);
    }
    while (lock_depth > 0)
    {
        (void)scma_mutex_unlock();
    }
    return (FT_ERR_SUCCESS);
}

ft_size_t    scma_mutex_lock_count(void)
{
    ft_size_t lock_depth;

    lock_depth = scma_runtime_lock_depth();
    return (lock_depth);
}
