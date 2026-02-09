#include <system_error>

#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"

int pt_recursive_mutex::try_lock(pthread_t thread_id) const
{
    int ensure_error = this->ensure_native_mutex();
    if (ensure_error != FT_ERR_SUCCESSS)
        return (ensure_error);

    if (this->_lock.load(std::memory_order_acquire))
    {
        pt_thread_id_type owner = this->_owner.load(std::memory_order_relaxed);
        if (pt_thread_equal(owner, thread_id))
        {
            this->_lock_depth.fetch_add(1, std::memory_order_relaxed);
            return (FT_ERR_SUCCESSS);
        }
    }

    bool acquired = false;
    try
    {
        acquired = this->_native_mutex->try_lock();
    }
    catch (const std::system_error &error)
    {
        return (ft_map_system_error(error.code().value()));
    }
    if (!acquired)
        return (FT_ERR_MUTEX_ALREADY_LOCKED);

    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock.store(true, std::memory_order_release);
    this->_lock_depth.store(1, std::memory_order_relaxed);

    int notify_error = pt_lock_tracking::notify_acquired(thread_id,
            static_cast<const void *>(this));
    if (notify_error != FT_ERR_SUCCESSS)
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

    return (FT_ERR_SUCCESSS);
}
