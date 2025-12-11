#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"

thread_local pt_thread_id_type pt_thread_id = THREAD_ID;

int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine)(void *), void *arg)
{
    int return_value;

    return_value = pthread_create(thread, attr, start_routine, arg);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (return_value);
}
