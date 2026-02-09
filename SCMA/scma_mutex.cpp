#include <cstddef>
#include <new>
#include "../Errno/errno.hpp"
#include "../Basic/basic_limits.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "SCMA.hpp"
#include "scma_internal.hpp"

static pt_recursive_mutex    *g_scma_mutex = ft_nullptr;
static thread_local ft_size_t g_scma_lock_depth = 0;
static pt_recursive_mutex    g_scma_dummy_mutex;

int scma_enable_thread_safety(void)
{
    if (g_scma_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_error = pt_recursive_mutex_create_with_error(&g_scma_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        scma_record_operation_error(mutex_error);
        return (mutex_error);
    }
    return (FT_ERR_SUCCESSS);
}

void scma_disable_thread_safety(void)
{
    pt_recursive_mutex_destroy(&g_scma_mutex);
    return ;
}

bool scma_is_thread_safe_enabled(void)
{
    return (g_scma_mutex != ft_nullptr);
}

static pt_recursive_mutex *scma_runtime_mutex_ptr(void)
{
    if (g_scma_mutex == ft_nullptr)
    {
        if (scma_enable_thread_safety() != FT_ERR_SUCCESSS)
            return (ft_nullptr);
    }
    return (g_scma_mutex);
}

pt_recursive_mutex    &scma_runtime_mutex(void)
{
    pt_recursive_mutex *mutex_pointer = scma_runtime_mutex_ptr();

    if (mutex_pointer == ft_nullptr)
    {
        scma_record_operation_error(FT_ERR_INITIALIZATION_FAILED);
        return (g_scma_dummy_mutex);
    }
    scma_record_operation_error(FT_ERR_SUCCESSS);
    return (*mutex_pointer);
}

static ft_size_t    &scma_runtime_lock_depth(void)
{
    return (g_scma_lock_depth);
}

int    scma_mutex_lock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    int error_code;

    if (lock_depth == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_NO_MEMORY;
        scma_record_operation_error(error_code);
        return (-1);
    }
    if (lock_depth == 0)
    {
        pt_recursive_mutex *mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            scma_record_operation_error(FT_ERR_INITIALIZATION_FAILED);
            return (-1);
        }
        pt_recursive_mutex &mutex = *mutex_pointer;
        int mutex_error = pt_recursive_mutex_lock_with_error(mutex);
        ft_global_error_stack_drop_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            scma_record_operation_error(mutex_error);
            return (-1);
        }
    }
    lock_depth = lock_depth + 1;
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    return (0);
}

int    scma_mutex_unlock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    int error_code;

    if (lock_depth == 0)
    {
        error_code = FT_ERR_INVALID_STATE;
        scma_record_operation_error(error_code);
        return (-1);
    }
    lock_depth = lock_depth - 1;
    if (lock_depth == 0)
    {
        pt_recursive_mutex *mutex_pointer = scma_runtime_mutex_ptr();
        if (mutex_pointer == ft_nullptr)
        {
            scma_record_operation_error(FT_ERR_INITIALIZATION_FAILED);
            return (-1);
        }
        pt_recursive_mutex &mutex = *mutex_pointer;
        int mutex_error = pt_recursive_mutex_unlock_with_error(mutex);
        ft_global_error_stack_drop_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            scma_record_operation_error(mutex_error);
            return (-1);
        }
    }
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    return (0);
}

int    scma_mutex_close(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    int error_code;

    if (lock_depth == 0)
    {
        error_code = FT_ERR_INVALID_STATE;
        scma_record_operation_error(error_code);
        return (-1);
    }
    while (lock_depth > 0)
    {
        if (scma_mutex_unlock() != 0)
        {
            error_code = scma_pop_operation_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
            scma_record_operation_error(error_code);
            return (-1);
        }
    }
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    return (0);
}

ft_size_t    scma_mutex_lock_count(void)
{
    ft_size_t lock_depth;
    int error_code;

    lock_depth = scma_runtime_lock_depth();
    error_code = FT_ERR_SUCCESSS;
    scma_record_operation_error(error_code);
    return (lock_depth);
}
