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
    pthread_t owner;

    owner = this->_owner.load(std::memory_order_relaxed);
    if (!pt_thread_equal(owner, thread_id))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_SUCCESS);
    }
    if (!this->_lock)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_unlock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
    this->_owner.store(0, std::memory_order_relaxed);
    this->_lock = false;
    this->set_error(ER_SUCCESS);
    return (FT_SUCCESS);
}

