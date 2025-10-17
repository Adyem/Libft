#include <pthread.h>
#include <errno.h>
#include "../Errno/errno.hpp"
#include "pthread.hpp"

int pt_thread_join(pthread_t thread, void **retval)
{
    int return_value;

    if (!thread)
    {
        return_value = ESRCH;
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    return_value = pthread_join(thread, retval);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}
