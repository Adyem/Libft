#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"

int pt_thread_detach(pthread_t thread)
{
    int return_value;

    return_value = pthread_detach(thread);
    if (return_value != 0)
    {
        ft_errno = return_value + ERRNO_OFFSET;
        return (return_value);
    }
    ft_errno = ER_SUCCESS;
    return (return_value);
}
