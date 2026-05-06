#ifndef PTHREAD_INTERNAL_HPP
# define PTHREAD_INTERNAL_HPP

#include "mutex.hpp"
#include "recursive_mutex.hpp"

static inline int    pt_mutex_lock_if_not_null(const pt_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->lock());
}

static inline int    pt_mutex_unlock_if_not_null(const pt_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->unlock());
}

static inline int    pt_recursive_mutex_lock_if_not_null(
    const pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->lock());
}

static inline int    pt_recursive_mutex_unlock_if_not_null(
    const pt_recursive_mutex *mutex_pointer)
{
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (mutex_pointer->unlock());
}

#endif
