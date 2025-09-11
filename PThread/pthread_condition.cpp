#include <cerrno>
#include <pthread.h>
#include "condition.hpp"
#include "../Errno/errno.hpp"

int pt_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attributes)
{
    int return_value = pthread_cond_init(condition, attributes);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return (return_value);
}

int pt_cond_destroy(pthread_cond_t *condition)
{
    int return_value = pthread_cond_destroy(condition);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return (return_value);
}

int pt_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex)
{
    int return_value = pthread_cond_wait(condition, mutex);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return (return_value);
}

int pt_cond_signal(pthread_cond_t *condition)
{
    int return_value = pthread_cond_signal(condition);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return (return_value);
}

int pt_cond_broadcast(pthread_cond_t *condition)
{
    int return_value = pthread_cond_broadcast(condition);
    if (return_value != 0)
        ft_errno = errno + ERRNO_OFFSET;
    return (return_value);
}

