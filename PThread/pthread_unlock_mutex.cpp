#include <errno.h>

#include "pthread.hpp"
#include "mutex.hpp"
#include "../Logger/logger.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::unlock(pthread_t thread_id) const
{
    int mutex_error;
    bool tracking_reports_owned;
    bool should_notify_release;
    int result;
    pt_mutex_vector owned_mutexes;

    this->set_error(ER_SUCCESS);
    tracking_reports_owned = false;
    should_notify_release = false;
    result = FT_SUCCESS;
    bool state_lock_acquired;
    bool native_initialized;

    state_lock_acquired = false;
    if (this->lock_internal(&state_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        goto cleanup;
    }
    native_initialized = this->_native_initialized;
    this->unlock_internal(state_lock_acquired);
    if (!native_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        goto cleanup;
    }
    if (this->_lock.load(std::memory_order_acquire))
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (owner == 0)
        {
            owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
            if (ft_errno != ER_SUCCESS)
            {
                this->set_error(ft_errno);
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
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                goto cleanup;
            }
        }
        else if (!pt_thread_equal(owner, thread_id))
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            goto cleanup;
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
        if (ft_errno != ER_SUCCESS)
        {
            this->set_error(ft_errno);
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
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        goto cleanup;
    }
    mutex_error = pthread_mutex_unlock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        ft_log_error("pt_mutex::unlock pthread_mutex_unlock failed mutex=%p thread=%p error=%d",
            &this->_native_mutex, reinterpret_cast<void *>(thread_id), mutex_error);
        if (should_notify_release)
        {
            pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        }
        if (mutex_error == EPERM || mutex_error == EINVAL)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
        }
        else
        {
            this->set_error(FT_ERR_INVALID_STATE);
        }
        bool reset_lock_acquired;

        reset_lock_acquired = false;
        if (this->lock_internal(&reset_lock_acquired) == 0 && reset_lock_acquired)
        {
            this->_native_initialized = false;
            this->unlock_internal(reset_lock_acquired);
        }
        else
            this->unlock_internal(reset_lock_acquired);
        goto cleanup;
    }
    if (should_notify_release)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
    }
    this->set_error(ER_SUCCESS);

cleanup:
    this->_lock.store(false, std::memory_order_release);
    this->_owner.store(0, std::memory_order_release);
    return (result);
}
