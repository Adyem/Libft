#include <cstddef>
#include "../Errno/errno.hpp"
#include "../Libft/limits.hpp"
#include "../PThread/pthread.hpp"
#include "SCMA.hpp"

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

int    scma_mutex_lock(void)
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
        if (mutex.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = mutex.get_error();
            return (-1);
        }
    }
    lock_depth = lock_depth + 1;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

int    scma_mutex_unlock(void)
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
        if (mutex.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = mutex.get_error();
            return (-1);
        }
    }
    ft_errno = previous_errno;
    return (0);
}

int    scma_mutex_close(void)
{
    ft_size_t &lock_depth = scma_runtime_lock_depth();

    if (lock_depth == 0)
    {
        ft_errno = FT_ERR_INVALID_STATE;
        return (-1);
    }
    int previous_errno;

    previous_errno = ft_errno;
    while (lock_depth > 0)
    {
        if (scma_mutex_unlock() != 0)
        {
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
