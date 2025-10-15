#include <vector>
#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_mutex::lock(pthread_t thread_id) const
{
    std::vector<pthread_mutex_t *> owned_mutexes;
    int mutex_error;

    this->set_error(ER_SUCCESS);
    if (!this->ensure_native_mutex())
        return (FT_SUCCESS);
    if (this->_lock && pt_thread_equal(this->_owner.load(std::memory_order_relaxed), thread_id))
    {
        this->set_error(FT_ERR_MUTEX_ALREADY_LOCKED);
        return (FT_SUCCESS);
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    if (ft_errno != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        return (FT_SUCCESS);
    }
    if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
    {
        int tracker_error;

        tracker_error = ft_errno;
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        this->_lock = false;
        this->_owner.store(0, std::memory_order_relaxed);
        this->set_error(tracker_error);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_lock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        this->_lock = false;
        this->_owner.store(0, std::memory_order_relaxed);
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
    this->set_error(ER_SUCCESS);
    return (FT_SUCCESS);
}
