#include <errno.h>

#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "../Logger/logger.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../System_utils/system_utils.hpp"

int pt_recursive_mutex::unlock(pthread_t thread_id) const
{
    int mutex_error;
    bool tracking_reports_owned;
    bool should_notify_release;
    int result;
    pt_mutex_vector owned_mutexes;
    bool fully_release;
    int lock_error;
    int tracking_error;

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    tracking_reports_owned = false;
    should_notify_release = false;
    result = FT_SUCCESS;
    fully_release = true;
    bool state_lock_acquired;
    bool native_initialized;

    state_lock_acquired = false;
    lock_error = this->lock_internal(&state_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        result = lock_error;
        goto cleanup;
    }
    native_initialized = this->_native_initialized;
    lock_error = this->unlock_internal(state_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        result = lock_error;
        goto cleanup;
    }
    if (!native_initialized)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        result = FT_ERR_INVALID_STATE;
        su_abort();
    }
    if (this->_lock.load(std::memory_order_acquire))
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (owner == 0)
        {
            owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
            tracking_error = ft_global_error_stack_drop_last_error();
            if (tracking_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(tracking_error);
                result = tracking_error;
                goto cleanup;
            }
            ft_size_t index;

            index = 0;
            while (index < owned_mutexes.size())
            {
                if (owned_mutexes[index] == &this->_native_mutex)
                {
                    tracking_reports_owned = true;
                    should_notify_release = true;
                    break ;
                }
                index += 1;
            }
            if (!tracking_reports_owned)
            {
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                result = FT_ERR_INVALID_ARGUMENT;
                su_abort();
            }
        }
        else if (!pt_thread_equal(owner, thread_id))
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            result = FT_ERR_INVALID_ARGUMENT;
            su_abort();
        }
        else
        {
            tracking_reports_owned = true;
            should_notify_release = true;
        }
    }
    else
    {
        ft_size_t index;

        owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
        tracking_error = ft_global_error_stack_drop_last_error();
        if (tracking_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(tracking_error);
            result = tracking_error;
            goto cleanup;
        }
        index = 0;
        while (index < owned_mutexes.size())
        {
            if (owned_mutexes[index] == &this->_native_mutex)
            {
                tracking_reports_owned = true;
                should_notify_release = true;
                break ;
            }
            index += 1;
        }
    }
    if (!tracking_reports_owned)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        result = FT_ERR_INVALID_ARGUMENT;
        su_abort();
    }
    std::size_t current_depth;

    current_depth = this->_lock_depth.load(std::memory_order_relaxed);
    if (current_depth > 1)
    {
        this->_lock_depth.store(current_depth - 1, std::memory_order_relaxed);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        fully_release = false;
        goto cleanup;
    }
    mutex_error = pthread_mutex_unlock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        ft_log_error("pt_recursive_mutex::unlock pthread_mutex_unlock failed mutex=%p thread=%p error=%d",
            &this->_native_mutex, reinterpret_cast<void *>(thread_id), mutex_error);
        if (should_notify_release)
        {
            pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
            tracking_error = ft_global_error_stack_drop_last_error();
            if (tracking_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(tracking_error);
                result = tracking_error;
            }
        }
        if (mutex_error == EPERM || mutex_error == EINVAL)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            result = FT_ERR_INVALID_ARGUMENT;
            su_abort();
        }
        else
        {
            ft_global_error_stack_push(FT_ERR_INVALID_STATE);
            result = FT_ERR_INVALID_STATE;
            su_abort();
        }
    }
    if (should_notify_release)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        tracking_error = ft_global_error_stack_drop_last_error();
        if (tracking_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(tracking_error);
            result = tracking_error;
            goto cleanup;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);

cleanup:
    if (fully_release)
    {
        this->_lock_depth.store(0, std::memory_order_relaxed);
        this->_lock.store(false, std::memory_order_release);
        this->_owner.store(0, std::memory_order_release);
    }
    return (result);
}
