#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "../Errno/errno.hpp"
#include "pthread.hpp"

int pt_thread_join(pthread_t thread, void **retval)
{
    int return_value;
    int error_code;

    if (!thread)
    {
        return_value = ESRCH;
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    return_value = pthread_join(thread, retval);
    if (return_value != 0)
    {
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
}

int pt_thread_timed_join(pthread_t thread, void **retval, long timeout_ms)
{
    int return_value;
    int error_code;
    struct timespec absolute_timeout;
    long additional_nanoseconds;

    if (!thread)
    {
        return_value = ESRCH;
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    if (timeout_ms < 0)
    {
        return_value = EINVAL;
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
#ifdef __linux__
    return_value = clock_gettime(CLOCK_REALTIME, &absolute_timeout);
    if (return_value != 0)
    {
        return_value = errno;
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    absolute_timeout.tv_sec += timeout_ms / 1000;
    additional_nanoseconds = (timeout_ms % 1000) * 1000000;
    absolute_timeout.tv_nsec += additional_nanoseconds;
    while (absolute_timeout.tv_nsec >= 1000000000)
    {
        absolute_timeout.tv_nsec -= 1000000000;
        absolute_timeout.tv_sec += 1;
    }
    return_value = pthread_timedjoin_np(thread, retval, &absolute_timeout);
    if (return_value != 0)
    {
        error_code = ft_map_system_error(return_value);
        ft_global_error_stack_push(error_code);
        return (return_value);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (return_value);
#else
    (void)retval;
    (void)timeout_ms;
    error_code = FT_ERR_INVALID_STATE;
    ft_global_error_stack_push(error_code);
    return (EINVAL);
#endif
}
