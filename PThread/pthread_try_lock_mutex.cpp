#include <errno.h>
#include <system_error>
#include <vector>
#include <time.h>
#include "../CPP_class/class_nullptr.hpp"
#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"

int pt_mutex::try_lock() const
{
    pt_thread_id_type thread_id = pt_thread_self();
    int ensure_error = this->ensure_native_mutex();

    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);

    pthread_t owner = this->_owner.load(std::memory_order_relaxed);
    if (this->_lock.load(std::memory_order_acquire)
            && pt_thread_equal(owner, thread_id))
        return (FT_ERR_MUTEX_ALREADY_LOCKED);

    int tracking_error = FT_ERR_SUCCESS;
    pt_mutex_vector owned_mutexes =
        pt_lock_tracking::get_owned_mutexes(thread_id, &tracking_error);
    if (tracking_error != FT_ERR_SUCCESS)
        return (tracking_error);

    int wait_result = pt_lock_tracking::notify_wait(thread_id,
            static_cast<const void *>(this), owned_mutexes);
    if (wait_result != FT_ERR_SUCCESS)
        return (wait_result);

    if (!this->_native_mutex->try_lock())
    {
        pt_lock_tracking::notify_released(thread_id,
                static_cast<const void *>(this));
        return (FT_ERR_MUTEX_ALREADY_LOCKED);
    }

    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock.store(true, std::memory_order_release);

    int notify_error = pt_lock_tracking::notify_acquired(thread_id,
            static_cast<const void *>(this));
    if (notify_error != FT_ERR_SUCCESS)
    {
        this->_lock.store(false, std::memory_order_release);
        this->_owner.store(0, std::memory_order_relaxed);
        try
        {
            this->_native_mutex->unlock();
        }
        catch (const std::system_error &)
        {
        }
        pt_lock_tracking::notify_released(thread_id,
                static_cast<const void *>(this));
        return (notify_error);
    }

    return (FT_ERR_SUCCESS);
}
