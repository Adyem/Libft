#include <system_error>

#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"

int pt_recursive_mutex::lock() const
{
    int ensure_error = this->ensure_native_mutex();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);

    pt_thread_id_type thread_id = pt_thread_self();
    if (this->_lock.load(std::memory_order_acquire))
    {
        pt_thread_id_type owner = this->_owner.load(std::memory_order_relaxed);
        if (pt_thread_equal(owner, thread_id))
        {
            this->_lock_depth.fetch_add(1, std::memory_order_relaxed);
            return (FT_ERR_SUCCESS);
        }
    }

    pt_mutex_vector owned_mutexes;
    int tracking_error = FT_ERR_SUCCESS;
    const void *mutex_handle = static_cast<const void *>(this->_native_mutex);

    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id, &tracking_error);
    if (tracking_error != FT_ERR_SUCCESS)
    {
        pt_buffer_destroy(owned_mutexes);
        return (tracking_error);
    }

    int wait_result = pt_lock_tracking::notify_wait(thread_id,
            mutex_handle, owned_mutexes);
    pt_buffer_destroy(owned_mutexes);
    if (wait_result != FT_ERR_SUCCESS)
        return (wait_result);

    int mutex_error = FT_ERR_SUCCESS;
    try
    {
        this->_native_mutex->lock();
    }
    catch (const std::system_error &error)
    {
        mutex_error = cmp_map_system_error_to_ft(error.code().value());
    }
    if (mutex_error != FT_ERR_SUCCESS)
    {
        pt_lock_tracking::notify_released(thread_id, mutex_handle);
        return (mutex_error);
    }

    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock.store(true, std::memory_order_release);
    this->_lock_depth.store(1, std::memory_order_relaxed);

    int notify_error = pt_lock_tracking::notify_acquired(thread_id,
            mutex_handle);
    if (notify_error != FT_ERR_SUCCESS)
    {
        this->_lock.store(false, std::memory_order_release);
        this->_owner.store(0, std::memory_order_release);
        this->_lock_depth.store(0, std::memory_order_relaxed);
        try
        {
            this->_native_mutex->unlock();
        }
        catch (const std::system_error &)
        {
        }
        return (notify_error);
    }

    return (FT_ERR_SUCCESS);
}
