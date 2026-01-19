#include <errno.h>
#include <vector>
#include <time.h>
#include "../CPP_class/class_nullptr.hpp"
#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::try_lock(pthread_t thread_id) const
{
    pt_mutex_vector owned_mutexes;
    int mutex_error;
    int tracking_error;

    pt_mutex::operation_error_push(FT_ERR_SUCCESSS);
    if (!this->ensure_native_mutex())
        return (FT_SUCCESS);
    if (this->_lock && pt_thread_equal(this->_owner.load(std::memory_order_relaxed), thread_id))
    {
        int retry_count;

        owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
        tracking_error = ft_global_error_stack_pop_newest();
        if (tracking_error != FT_ERR_SUCCESSS)
        {
            pt_mutex::operation_error_push(tracking_error);
            return (FT_SUCCESS);
        }
        retry_count = 0;
        while (retry_count < 10)
        {
            if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
            {
                tracking_error = ft_global_error_stack_pop_newest();
                pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
                ft_global_error_stack_pop_newest();
                if (tracking_error == FT_ERR_SUCCESSS)
                    tracking_error = FT_ERR_INVALID_STATE;
                pt_mutex::operation_error_push(tracking_error);
                return (FT_SUCCESS);
            }
            tracking_error = ft_global_error_stack_pop_newest();
            if (tracking_error != FT_ERR_SUCCESSS)
            {
                pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
                ft_global_error_stack_pop_newest();
                pt_mutex::operation_error_push(tracking_error);
                return (FT_SUCCESS);
            }
            pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
            ft_global_error_stack_pop_newest();
            struct timespec retry_sleep;

            retry_sleep.tv_sec = 0;
            retry_sleep.tv_nsec = 10000000;
            nanosleep(&retry_sleep, ft_nullptr);
            retry_count++;
        }
        pt_mutex::operation_error_push(FT_ERR_MUTEX_ALREADY_LOCKED);
        return (FT_SUCCESS);
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
    {
        pt_mutex::operation_error_push(tracking_error);
        return (FT_SUCCESS);
    }
    if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
    {
        tracking_error = ft_global_error_stack_pop_newest();
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        if (tracking_error == FT_ERR_SUCCESSS)
            tracking_error = FT_ERR_INVALID_STATE;
        pt_mutex::operation_error_push(tracking_error);
        return (FT_SUCCESS);
    }
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        pt_mutex::operation_error_push(tracking_error);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_trylock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        if (mutex_error == EBUSY)
        {
            int retry_count;

            retry_count = 0;
            pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
            ft_global_error_stack_pop_newest();
            while (retry_count < 10)
            {
                if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
                {
                    tracking_error = ft_global_error_stack_pop_newest();
                    pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
                    ft_global_error_stack_pop_newest();
                    if (tracking_error == FT_ERR_SUCCESSS)
                        tracking_error = FT_ERR_INVALID_STATE;
                    pt_mutex::operation_error_push(tracking_error);
                    return (FT_SUCCESS);
                }
                tracking_error = ft_global_error_stack_pop_newest();
                if (tracking_error != FT_ERR_SUCCESSS)
                {
                    pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
                    ft_global_error_stack_pop_newest();
                    pt_mutex::operation_error_push(tracking_error);
                    return (FT_SUCCESS);
                }
                mutex_error = pthread_mutex_trylock(&this->_native_mutex);
                pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
                ft_global_error_stack_pop_newest();
                if (mutex_error == 0)
                {
                    this->_owner.store(thread_id, std::memory_order_relaxed);
                    this->_lock = true;
                    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
                    tracking_error = ft_global_error_stack_pop_newest();
                    if (tracking_error != FT_ERR_SUCCESSS)
                        pt_mutex::operation_error_push(tracking_error);
                    else
                        pt_mutex::operation_error_push(FT_ERR_SUCCESSS);
                    return (FT_SUCCESS);
                }
                if (mutex_error != EBUSY)
                {
                    pt_mutex::operation_error_push(FT_ERR_INVALID_STATE);
                    return (FT_SUCCESS);
                }
                struct timespec retry_sleep;

                retry_sleep.tv_sec = 0;
                retry_sleep.tv_nsec = 10000000;
                nanosleep(&retry_sleep, ft_nullptr);
                retry_count++;
            }
            pt_mutex::operation_error_push(FT_ERR_MUTEX_ALREADY_LOCKED);
            return (FT_SUCCESS);
        }
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        pt_mutex::operation_error_push(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
        pt_mutex::operation_error_push(tracking_error);
    else
        pt_mutex::operation_error_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}
