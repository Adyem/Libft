#include <system_error>
#include "pthread.hpp"
#include "mutex.hpp"
#include "../Logger/logger.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../System_utils/system_utils.hpp"

int pt_mutex::unlock() const
{
    pt_thread_id_type thread_id = pt_thread_self();
    int ensure_error = this->ensure_native_mutex();

    if (ensure_error != FT_ERR_SUCCESSS)
        return (ensure_error);
    if (!this->_lock.load(std::memory_order_acquire))
        return (FT_ERR_INVALID_STATE);
    pthread_t owner = this->_owner.load(std::memory_order_relaxed);
    if (!pt_thread_equal(owner, thread_id))
        return (FT_ERR_INVALID_ARGUMENT);
    int unlock_error = FT_ERR_SUCCESSS;
    try
    {
        this->_native_mutex->unlock();
    }
    catch (const std::system_error &error)
    {
        unlock_error = ft_map_system_error(error.code().value());
    }
    this->_lock.store(false, std::memory_order_release);
    this->_owner.store(0, std::memory_order_release);
    int notify_result = pt_lock_tracking::notify_released(thread_id,
            static_cast<const void *>(this));
    if (unlock_error != FT_ERR_SUCCESSS)
        return (unlock_error);
    return (notify_result);
}
