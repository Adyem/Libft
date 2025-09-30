#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"

int pt_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attributes)
{
    int return_value = pthread_rwlock_init(rwlock, attributes);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

int pt_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_rdlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

int pt_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_wrlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

int pt_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_unlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

int pt_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_destroy(rwlock);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}

