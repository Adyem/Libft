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
        ft_global_error_stack_push(ft_errno);
        return (return_value);
    }
    ft_errno = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}
