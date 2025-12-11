#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::lock(pthread_t thread_id) const
{
    pt_mutex_vector owned_mutexes;
    int mutex_error;
    bool lock_flag;

    this->set_error(FT_ERR_SUCCESSS);
    if (!this->ensure_native_mutex())
        return (FT_SUCCESS);
    lock_flag = this->_lock.load(std::memory_order_acquire);
    if (lock_flag)
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (pt_thread_equal(owner, thread_id))
        {
            this->set_error(FT_ERR_MUTEX_ALREADY_LOCKED);
            return (FT_SUCCESS);
        }
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_errno);
        return (FT_SUCCESS);
    }
    if (lock_flag)
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (owner == 0)
        {
            ft_size_t index;

            index = 0;
            while (index < owned_mutexes.size())
            {
                if (owned_mutexes[index] == &this->_native_mutex)
                {
                    this->set_error(FT_ERR_MUTEX_ALREADY_LOCKED);
                    return (FT_SUCCESS);
                }
                index += 1;
            }
        }
    }
    if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
    {
        int tracker_error;

        tracker_error = ft_errno;
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        this->set_error(tracker_error);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_lock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock.store(true, std::memory_order_release);
    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}
