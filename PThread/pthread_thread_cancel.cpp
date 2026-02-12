#include "pthread.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../Errno/errno.hpp"
#include <errno.h>

int pt_thread_cancel(pthread_t thread)
{
    int return_value;
    int error_code;

    return_value = cmp_thread_cancel(thread);
    if (return_value != 0)
    {
        if (return_value < 0)
            error_code = ft_map_system_error(errno);
        else
            error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (return_value);
}
