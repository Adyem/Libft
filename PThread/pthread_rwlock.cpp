#include <pthread.h>
#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"

int pt_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attributes)
{
    int return_value = pthread_rwlock_init(rwlock, attributes);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

int pt_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_rdlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

int pt_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_wrlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

int pt_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int return_value;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    return_value = pthread_rwlock_unlock(rwlock);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

int pt_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_destroy(rwlock);
    if (return_value != 0)
    {
        ft_errno = ft_map_system_error(return_value);
        return (return_value);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (return_value);
}

static void pt_rwlock_strategy_set_error(t_pt_rwlock *rwlock, int error_code)
{
    if (rwlock != ft_nullptr)
        rwlock->error_code = error_code;
    ft_errno = error_code;
    return ;
}

static int pt_rwlock_strategy_map_error(t_pt_rwlock *rwlock, int system_error)
{
    int mapped_error;

    mapped_error = ft_map_system_error(system_error);
    pt_rwlock_strategy_set_error(rwlock, mapped_error);
    return (mapped_error);
}

int pt_rwlock_strategy_init(t_pt_rwlock *rwlock, t_pt_rwlock_strategy strategy)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    system_error = pthread_mutex_init(&rwlock->mutex, ft_nullptr);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    system_error = pthread_cond_init(&rwlock->reader_condition, ft_nullptr);
    if (system_error != 0)
    {
        pthread_mutex_destroy(&rwlock->mutex);
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    }
    system_error = pthread_cond_init(&rwlock->writer_condition, ft_nullptr);
    if (system_error != 0)
    {
        pthread_cond_destroy(&rwlock->reader_condition);
        pthread_mutex_destroy(&rwlock->mutex);
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    }
    rwlock->active_readers = 0;
    rwlock->waiting_readers = 0;
    rwlock->active_writers = 0;
    rwlock->waiting_writers = 0;
    rwlock->strategy = strategy;
    pt_rwlock_strategy_set_error(rwlock, FT_ER_SUCCESSS);
    return (FT_ER_SUCCESSS);
}

static int pt_rwlock_strategy_unlock_mutex(t_pt_rwlock *rwlock)
{
    int system_error;

    system_error = pthread_mutex_unlock(&rwlock->mutex);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    return (FT_ER_SUCCESSS);
}

int pt_rwlock_strategy_rdlock(t_pt_rwlock *rwlock)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    system_error = pthread_mutex_lock(&rwlock->mutex);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    rwlock->waiting_readers++;
    if (rwlock->strategy == PT_RWLOCK_STRATEGY_WRITER_PRIORITY)
    {
        while (rwlock->active_writers > 0 || rwlock->waiting_writers > 0)
        {
            system_error = pthread_cond_wait(&rwlock->reader_condition, &rwlock->mutex);
            if (system_error != 0)
            {
                rwlock->waiting_readers--;
                pt_rwlock_strategy_unlock_mutex(rwlock);
                return (pt_rwlock_strategy_map_error(rwlock, system_error));
            }
        }
    }
    else
    {
        while (rwlock->active_writers > 0)
        {
            system_error = pthread_cond_wait(&rwlock->reader_condition, &rwlock->mutex);
            if (system_error != 0)
            {
                rwlock->waiting_readers--;
                pt_rwlock_strategy_unlock_mutex(rwlock);
                return (pt_rwlock_strategy_map_error(rwlock, system_error));
            }
        }
    }
    rwlock->waiting_readers--;
    rwlock->active_readers++;
    system_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (system_error != FT_ER_SUCCESSS)
        return (system_error);
    pt_rwlock_strategy_set_error(rwlock, FT_ER_SUCCESSS);
    return (FT_ER_SUCCESSS);
}

int pt_rwlock_strategy_wrlock(t_pt_rwlock *rwlock)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    system_error = pthread_mutex_lock(&rwlock->mutex);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    rwlock->waiting_writers++;
    while (rwlock->active_readers > 0 || rwlock->active_writers > 0)
    {
        system_error = pthread_cond_wait(&rwlock->writer_condition, &rwlock->mutex);
        if (system_error != 0)
        {
            rwlock->waiting_writers--;
            pt_rwlock_strategy_unlock_mutex(rwlock);
            return (pt_rwlock_strategy_map_error(rwlock, system_error));
        }
    }
    rwlock->waiting_writers--;
    rwlock->active_writers = 1;
    system_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (system_error != FT_ER_SUCCESSS)
        return (system_error);
    pt_rwlock_strategy_set_error(rwlock, FT_ER_SUCCESSS);
    return (FT_ER_SUCCESSS);
}

int pt_rwlock_strategy_unlock(t_pt_rwlock *rwlock)
{
    int system_error;
    int broadcast_error;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    system_error = pthread_mutex_lock(&rwlock->mutex);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    if (rwlock->active_writers > 0)
    {
        rwlock->active_writers = 0;
        if (rwlock->strategy == PT_RWLOCK_STRATEGY_WRITER_PRIORITY)
        {
            if (rwlock->waiting_writers > 0)
            {
                system_error = pthread_cond_signal(&rwlock->writer_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    return (pt_rwlock_strategy_map_error(rwlock, system_error));
                }
            }
            else
            {
                system_error = pthread_cond_broadcast(&rwlock->reader_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    return (pt_rwlock_strategy_map_error(rwlock, system_error));
                }
            }
        }
        else
        {
            if (rwlock->waiting_readers > 0)
            {
                system_error = pthread_cond_broadcast(&rwlock->reader_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    return (pt_rwlock_strategy_map_error(rwlock, system_error));
                }
            }
            else if (rwlock->waiting_writers > 0)
            {
                system_error = pthread_cond_signal(&rwlock->writer_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    return (pt_rwlock_strategy_map_error(rwlock, system_error));
                }
            }
        }
    }
    else
    {
        if (rwlock->active_readers == 0)
        {
            pt_rwlock_strategy_unlock_mutex(rwlock);
            pt_rwlock_strategy_set_error(rwlock, FT_ERR_INVALID_OPERATION);
            return (FT_ERR_INVALID_OPERATION);
        }
        rwlock->active_readers--;
        if (rwlock->active_readers == 0 && rwlock->waiting_writers > 0)
        {
            system_error = pthread_cond_signal(&rwlock->writer_condition);
            if (system_error != 0)
            {
                pt_rwlock_strategy_unlock_mutex(rwlock);
                return (pt_rwlock_strategy_map_error(rwlock, system_error));
            }
        }
    }
    broadcast_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (broadcast_error != FT_ER_SUCCESSS)
        return (broadcast_error);
    pt_rwlock_strategy_set_error(rwlock, FT_ER_SUCCESSS);
    return (FT_ER_SUCCESSS);
}

int pt_rwlock_strategy_destroy(t_pt_rwlock *rwlock)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    system_error = pthread_cond_destroy(&rwlock->reader_condition);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    system_error = pthread_cond_destroy(&rwlock->writer_condition);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    system_error = pthread_mutex_destroy(&rwlock->mutex);
    if (system_error != 0)
        return (pt_rwlock_strategy_map_error(rwlock, system_error));
    pt_rwlock_strategy_set_error(rwlock, FT_ER_SUCCESSS);
    return (FT_ER_SUCCESSS);
}

int pt_rwlock_strategy_get_error(const t_pt_rwlock *rwlock)
{
    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    ft_errno = rwlock->error_code;
    return (rwlock->error_code);
}

const char *pt_rwlock_strategy_get_error_str(const t_pt_rwlock *rwlock)
{
    if (rwlock == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_strerror(FT_ERR_INVALID_ARGUMENT));
    }
    ft_errno = rwlock->error_code;
    return (ft_strerror(rwlock->error_code));
}

