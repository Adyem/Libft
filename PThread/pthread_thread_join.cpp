#include <pthread.h>
#include "../Errno/errno.hpp"
#include "pthread.hpp"

int pt_thread_join(pthread_t thread, void **retval)
{
    int return_value;

    return_value = pthread_join(thread, retval);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}
