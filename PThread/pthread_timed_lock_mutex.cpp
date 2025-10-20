#include <errno.h>
#include <vector>
#include <time.h>
#include "pthread.hpp"
#include "mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

static bool compute_absolute_deadline(const struct timespec &relative_time, struct timespec *absolute_time, int *error_code)
{
    struct timespec current_time;

    if (relative_time.tv_sec < 0)
    {
        *error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (relative_time.tv_nsec < 0)
    {
        *error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (clock_gettime(CLOCK_REALTIME, &current_time) != 0)
    {
        *error_code = ft_map_system_error(errno);
        return (false);
    }
    absolute_time->tv_sec = current_time.tv_sec + relative_time.tv_sec;
    absolute_time->tv_nsec = current_time.tv_nsec + relative_time.tv_nsec;
    while (absolute_time->tv_nsec >= 1000000000L)
    {
        absolute_time->tv_nsec -= 1000000000L;
        absolute_time->tv_sec += 1;
    }
    *error_code = ER_SUCCESS;
    return (true);
}

int pt_mutex::try_lock_until(pthread_t thread_id, const struct timespec &absolute_time) const
{
    pt_mutex_vector owned_mutexes;
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
        this->set_error(tracker_error);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_timedlock(&this->_native_mutex, &absolute_time);
    if (mutex_error != 0)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        if (mutex_error == ETIMEDOUT)
        {
            this->set_error(ER_SUCCESS);
            return (ETIMEDOUT);
        }
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock = true;
    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
    this->set_error(ER_SUCCESS);
    return (FT_SUCCESS);
}

int pt_mutex::try_lock_for(pthread_t thread_id, const struct timespec &relative_time) const
{
    struct timespec absolute_time;
    int conversion_error;

    if (!compute_absolute_deadline(relative_time, &absolute_time, &conversion_error))
    {
        this->set_error(conversion_error);
        return (FT_SUCCESS);
    }
    return (this->try_lock_until(thread_id, absolute_time));
}

