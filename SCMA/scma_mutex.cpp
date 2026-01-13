#include <cstddef>
#include "../Errno/errno.hpp"
#include "../Libft/limits.hpp"
#include "../PThread/pthread.hpp"
#include "SCMA.hpp"

static pt_mutex g_scma_mutex;
static thread_local ft_size_t g_scma_lock_depth = 0;

pt_mutex    &scma_runtime_mutex(void)
{
    int error_code;

    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (g_scma_mutex);
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
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (lock_depth == 0)
    {
        pt_mutex &mutex = scma_runtime_mutex();

        mutex.lock(THREAD_ID);
        if (mutex.get_error() != FT_ERR_SUCCESSS)
        {
            error_code = mutex.get_error();
            ft_errno = error_code;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
    }
    lock_depth = lock_depth + 1;
    error_code = FT_ERR_SUCCESSS;
    ft_errno = error_code;
    ft_global_error_stack_push(error_code);
    return (0);
}

int    scma_mutex_unlock(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    int error_code;

    if (lock_depth == 0)
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    int previous_errno;

    previous_errno = ft_errno;
    lock_depth = lock_depth - 1;
    if (lock_depth == 0)
    {
        pt_mutex &mutex = scma_runtime_mutex();

        mutex.unlock(THREAD_ID);
        if (mutex.get_error() != FT_ERR_SUCCESSS)
        {
            error_code = mutex.get_error();
            ft_errno = error_code;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
    }
    ft_errno = previous_errno;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

int    scma_mutex_close(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();
    int error_code;

    if (lock_depth == 0)
    {
        error_code = FT_ERR_INVALID_STATE;
        ft_errno = error_code;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    int previous_errno;

    previous_errno = ft_errno;
    while (lock_depth > 0)
    {
        if (scma_mutex_unlock() != 0)
        {
            error_code = ft_errno;
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
            ft_global_error_stack_push(error_code);
            return (-1);
        }
    }
    ft_errno = previous_errno;
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}

ft_size_t    scma_mutex_lock_count(void)
{
    ft_size_t lock_depth;
    int error_code;

    lock_depth = scma_runtime_lock_depth();
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (lock_depth);
}
