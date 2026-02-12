#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"

thread_local pt_thread_id_type pt_thread_id = THREAD_ID;

int pt_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                void *(*start_routine)(void *), void *arg)
{
    int return_value;
    int error_code;

    return_value = pthread_create(thread, attr, start_routine, arg);
    if (return_value != 0)
    {
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (return_value);
}
