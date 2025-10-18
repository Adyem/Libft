#include <errno.h>

#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::unlock(pthread_t thread_id) const
{
    int mutex_error;

    this->set_error(ER_SUCCESS);
    if (!this->_native_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    bool tracking_reports_owned;

    tracking_reports_owned = false;
    if (this->_lock)
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (!pt_thread_equal(owner, thread_id))
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (FT_SUCCESS);
        }
        tracking_reports_owned = true;
    }
    else
    {
        pt_mutex_vector owned_mutexes;
        ft_size_t index;

        owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
        if (ft_errno != ER_SUCCESS)
        {
            this->set_error(ft_errno);
            return (FT_SUCCESS);
        }
        index = 0;
        while (index < owned_mutexes.size())
        {
            if (owned_mutexes[index] == &this->_native_mutex)
            {
                tracking_reports_owned = true;
                break ;
            }
            index += 1;
        }
    }
    if (!tracking_reports_owned)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_unlock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        if (mutex_error == EPERM || mutex_error == EINVAL)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
        }
        else
        {
            this->set_error(FT_ERR_INVALID_STATE);
        }
        return (FT_SUCCESS);
    }
    pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
    this->_owner.store(0, std::memory_order_relaxed);
    this->_lock = false;
    this->set_error(ER_SUCCESS);
    return (FT_SUCCESS);
}

