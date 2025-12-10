#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"

int pt_thread_detach(pthread_t thread)
{
    int return_value;

    return_value = pthread_detach(thread);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}
