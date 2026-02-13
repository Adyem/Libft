#include <system_error>
#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"

int pt_recursive_mutex::unlock() const
{
    int ensure_error = this->ensure_native_mutex();
    if (ensure_error != FT_ERR_SUCCESS)
        return (ensure_error);
    pt_thread_id_type thread_id = pt_thread_self();
    if (!this->_lock.load(std::memory_order_acquire))
        return (FT_ERR_INVALID_STATE);
    pt_thread_id_type owner = this->_owner.load(std::memory_order_relaxed);
    if (!pt_thread_equal(owner, thread_id))
        return (FT_ERR_INVALID_ARGUMENT);
    std::size_t current_depth = this->_lock_depth.load(std::memory_order_relaxed);
    if (current_depth > 1)
    {
        this->_lock_depth.store(current_depth - 1, std::memory_order_relaxed);
        return (FT_ERR_SUCCESS);
    }
    int mutex_error = FT_ERR_SUCCESS;
    try
    {
        this->_native_mutex->unlock();
    }
    catch (const std::system_error &error)
    {
        mutex_error = cmp_map_system_error_to_ft(error.code().value());
    }
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_lock_depth.store(0, std::memory_order_relaxed);
    this->_lock.store(false, std::memory_order_release);
    this->_owner.store(0, std::memory_order_release);
    (void)pt_lock_tracking::notify_released(thread_id,
            static_cast<const void *>(this));
    return (FT_ERR_SUCCESS);
}
