#include <system_error>
#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"

int pt_recursive_mutex::unlock() const
{
    if (this->_native_mutex == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
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
        return (FT_ERR_SUCCESSS);
    }
    int mutex_error = FT_ERR_SUCCESSS;
    try
    {
        this->_native_mutex->unlock();
    }
    catch (const std::system_error &error)
    {
        mutex_error = ft_map_system_error(error.code().value());
    }
    this->_lock_depth.store(0, std::memory_order_relaxed);
    this->_lock.store(false, std::memory_order_release);
    this->_owner.store(0, std::memory_order_release);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    return (pt_lock_tracking::notify_released(thread_id,
            static_cast<const void *>(this)));
}
