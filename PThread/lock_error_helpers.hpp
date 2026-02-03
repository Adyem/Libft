#ifndef PTHREAD_LOCK_ERROR_HELPERS_HPP
# define PTHREAD_LOCK_ERROR_HELPERS_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "mutex.hpp"
#include "unique_lock.hpp"

static inline int ft_mutex_pop_last_error(pt_mutex *mutex)
{
    if (mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int error_code = ft_global_error_stack_pop_newest();

    ft_global_error_stack_pop_newest();
    return (error_code);
}

static inline int ft_unique_lock_pop_last_error(const ft_unique_lock<pt_mutex> &mutex_guard)
{
    unsigned long long operation_id = mutex_guard.last_operation_id();

    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    return (mutex_guard.pop_operation_error(operation_id));
}

#endif
