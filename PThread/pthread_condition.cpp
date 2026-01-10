#include <pthread.h>
#include "condition.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

int pt_cond_init(pthread_cond_t *condition, const pthread_condattr_t *attributes)
{
    int return_value = pthread_cond_init(condition, attributes);
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

int pt_cond_destroy(pthread_cond_t *condition)
{
    int return_value = pthread_cond_destroy(condition);
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

int pt_cond_wait(pthread_cond_t *condition, pthread_mutex_t *mutex)
{
    int return_value;

    if (condition == ft_nullptr || mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(ft_errno);
        return (-1);
    }
    return_value = pthread_cond_wait(condition, mutex);
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

int pt_cond_signal(pthread_cond_t *condition)
{
    int return_value = pthread_cond_signal(condition);
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

int pt_cond_broadcast(pthread_cond_t *condition)
{
    int return_value = pthread_cond_broadcast(condition);
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
