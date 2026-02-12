#include <pthread.h>
#include "pthread.hpp"
#include "pthread_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"

static int pt_rwlock_report_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

static int pt_rwlock_report_system_error(int system_error)
{
    int mapped_error;

    mapped_error = ft_map_system_error(system_error);
    return (pt_rwlock_report_result(mapped_error, mapped_error));
}

int pt_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attributes)
{
    int return_value = pthread_rwlock_init(rwlock, attributes);
    if (return_value != 0)
    {
        return (pt_rwlock_report_result(ft_map_system_error(return_value), return_value));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, return_value));
}

int pt_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_rdlock(rwlock);
    if (return_value != 0)
    {
        return (pt_rwlock_report_result(ft_map_system_error(return_value), return_value));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, return_value));
}

int pt_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_wrlock(rwlock);
    if (return_value != 0)
    {
        return (pt_rwlock_report_result(ft_map_system_error(return_value), return_value));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, return_value));
}

int pt_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int return_value;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    return_value = pthread_rwlock_unlock(rwlock);
    if (return_value != 0)
    {
        return (pt_rwlock_report_result(ft_map_system_error(return_value), return_value));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, return_value));
}

int pt_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    int return_value = pthread_rwlock_destroy(rwlock);
    if (return_value != 0)
    {
        return (pt_rwlock_report_result(ft_map_system_error(return_value), return_value));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, return_value));
}

int pt_rwlock_strategy_init(t_pt_rwlock *rwlock, t_pt_rwlock_strategy strategy)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    system_error = pthread_mutex_init(&rwlock->mutex, ft_nullptr);
    if (system_error != 0)
    {
        return (pt_rwlock_report_system_error(system_error));
    }
    system_error = pthread_cond_init(&rwlock->reader_condition, ft_nullptr);
    if (system_error != 0)
    {
        pthread_mutex_destroy(&rwlock->mutex);
        return (pt_rwlock_report_system_error(system_error));
    }
    system_error = pthread_cond_init(&rwlock->writer_condition, ft_nullptr);
    if (system_error != 0)
    {
        pthread_cond_destroy(&rwlock->reader_condition);
        pthread_mutex_destroy(&rwlock->mutex);
        return (pt_rwlock_report_system_error(system_error));
    }
    rwlock->active_readers = 0;
    rwlock->waiting_readers = 0;
    rwlock->active_writers = 0;
    rwlock->waiting_writers = 0;
    rwlock->strategy = strategy;
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, FT_ERR_SUCCESS));
}

static int pt_rwlock_strategy_unlock_mutex(t_pt_rwlock *rwlock)
{
    return (pt_pthread_mutex_unlock_with_error(&rwlock->mutex));
}

int pt_rwlock_strategy_rdlock(t_pt_rwlock *rwlock)
{
    int system_error;
    int unlock_error;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    system_error = pt_pthread_mutex_lock_with_error(&rwlock->mutex);
    if (system_error != FT_ERR_SUCCESS)
    {
        return (pt_rwlock_report_result(system_error, system_error));
    }
    rwlock->waiting_readers++;
    if (rwlock->strategy == PT_RWLOCK_STRATEGY_WRITER_PRIORITY)
    {
        while (rwlock->active_writers > 0 || rwlock->waiting_writers > 0)
        {
            system_error = pthread_cond_wait(&rwlock->reader_condition, &rwlock->mutex);
            if (system_error != 0)
            {
                int mapped_error;

                rwlock->waiting_readers--;
                pt_rwlock_strategy_unlock_mutex(rwlock);
                mapped_error = ft_map_system_error(system_error);
                return (pt_rwlock_report_result(mapped_error, mapped_error));
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
                int mapped_error;

                rwlock->waiting_readers--;
                pt_rwlock_strategy_unlock_mutex(rwlock);
                mapped_error = ft_map_system_error(system_error);
                return (pt_rwlock_report_result(mapped_error, mapped_error));
            }
        }
    }
    rwlock->waiting_readers--;
    rwlock->active_readers++;
    unlock_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (unlock_error != FT_ERR_SUCCESS)
        return (pt_rwlock_report_result(unlock_error, unlock_error));
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, FT_ERR_SUCCESS));
}

int pt_rwlock_strategy_wrlock(t_pt_rwlock *rwlock)
{
    int system_error;
    int unlock_error;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    system_error = pt_pthread_mutex_lock_with_error(&rwlock->mutex);
    if (system_error != FT_ERR_SUCCESS)
    {
        return (pt_rwlock_report_result(system_error, system_error));
    }
    rwlock->waiting_writers++;
    while (rwlock->active_readers > 0 || rwlock->active_writers > 0)
    {
        system_error = pthread_cond_wait(&rwlock->writer_condition, &rwlock->mutex);
        if (system_error != 0)
        {
            int mapped_error;

            rwlock->waiting_writers--;
            pt_rwlock_strategy_unlock_mutex(rwlock);
            mapped_error = ft_map_system_error(system_error);
            return (pt_rwlock_report_result(mapped_error, mapped_error));
        }
    }
    rwlock->waiting_writers--;
    rwlock->active_writers = 1;
    unlock_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (unlock_error != FT_ERR_SUCCESS)
        return (pt_rwlock_report_result(unlock_error, unlock_error));
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, FT_ERR_SUCCESS));
}

int pt_rwlock_strategy_unlock(t_pt_rwlock *rwlock)
{
    int system_error;
    int unlock_error;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    system_error = pt_pthread_mutex_lock_with_error(&rwlock->mutex);
    if (system_error != FT_ERR_SUCCESS)
    {
        return (pt_rwlock_report_result(system_error, system_error));
    }
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
                    int mapped_error = ft_map_system_error(system_error);
                    return (pt_rwlock_report_result(mapped_error, mapped_error));
                }
            }
            else
            {
                system_error = pthread_cond_broadcast(&rwlock->reader_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    int mapped_error = ft_map_system_error(system_error);
                    return (pt_rwlock_report_result(mapped_error, mapped_error));
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
                    int mapped_error = ft_map_system_error(system_error);
                    return (pt_rwlock_report_result(mapped_error, mapped_error));
                }
            }
            else if (rwlock->waiting_writers > 0)
            {
                system_error = pthread_cond_signal(&rwlock->writer_condition);
                if (system_error != 0)
                {
                    pt_rwlock_strategy_unlock_mutex(rwlock);
                    int mapped_error = ft_map_system_error(system_error);
                    return (pt_rwlock_report_result(mapped_error, mapped_error));
                }
            }
        }
    }
    else
    {
        if (rwlock->active_readers == 0)
        {
            pt_rwlock_strategy_unlock_mutex(rwlock);
            return (pt_rwlock_report_result(FT_ERR_INVALID_OPERATION, FT_ERR_INVALID_OPERATION));
        }
        rwlock->active_readers--;
        if (rwlock->active_readers == 0 && rwlock->waiting_writers > 0)
        {
            system_error = pthread_cond_signal(&rwlock->writer_condition);
            if (system_error != 0)
            {
                pt_rwlock_strategy_unlock_mutex(rwlock);
                int mapped_error = ft_map_system_error(system_error);
                return (pt_rwlock_report_result(mapped_error, mapped_error));
            }
        }
    }
    unlock_error = pt_rwlock_strategy_unlock_mutex(rwlock);
    if (unlock_error != FT_ERR_SUCCESS)
        return (pt_rwlock_report_result(unlock_error, unlock_error));
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, FT_ERR_SUCCESS));
}

int pt_rwlock_strategy_destroy(t_pt_rwlock *rwlock)
{
    int system_error;

    if (rwlock == ft_nullptr)
    {
        return (pt_rwlock_report_result(FT_ERR_INVALID_ARGUMENT, -1));
    }
    system_error = pthread_cond_destroy(&rwlock->reader_condition);
    if (system_error != 0)
    {
        return (pt_rwlock_report_system_error(system_error));
    }
    system_error = pthread_cond_destroy(&rwlock->writer_condition);
    if (system_error != 0)
    {
        return (pt_rwlock_report_system_error(system_error));
    }
    system_error = pthread_mutex_destroy(&rwlock->mutex);
    if (system_error != 0)
    {
        return (pt_rwlock_report_system_error(system_error));
    }
    return (pt_rwlock_report_result(FT_ERR_SUCCESS, FT_ERR_SUCCESS));
}

int pt_rwlock_strategy_get_error(const t_pt_rwlock *rwlock)
{
    int error_code;

    if (rwlock == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    error_code = ft_global_error_stack_peek_last_error();
    return (error_code);
}

const char *pt_rwlock_strategy_get_error_str(const t_pt_rwlock *rwlock)
{
    int error_code;

    if (rwlock == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_strerror(FT_ERR_INVALID_ARGUMENT));
    }
    error_code = ft_global_error_stack_peek_last_error();
    return (ft_strerror(error_code));
}
