#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "pthread_lock_tracking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

int pt_recursive_mutex::lock(pthread_t thread_id) const
{
    pt_mutex_vector owned_mutexes;
    int mutex_error;
    int tracking_error;
    bool lock_flag;

    this->operation_error_push(FT_ERR_SUCCESSS);
    if (!this->ensure_native_mutex())
        return (FT_SUCCESS);
    lock_flag = this->_lock.load(std::memory_order_acquire);
    if (lock_flag)
    {
        pthread_t owner;

        owner = this->_owner.load(std::memory_order_relaxed);
        if (pt_thread_equal(owner, thread_id))
        {
            std::size_t current_depth;

            current_depth = this->_lock_depth.load(std::memory_order_relaxed);
            this->_lock_depth.store(current_depth + 1, std::memory_order_relaxed);
            this->operation_error_push(FT_ERR_SUCCESSS);
            return (FT_SUCCESS);
        }
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
    {
        this->operation_error_push(tracking_error);
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
                    this->operation_error_push(FT_ERR_MUTEX_ALREADY_LOCKED);
                    return (FT_SUCCESS);
                }
                index += 1;
            }
        }
    }
    if (!pt_lock_tracking::notify_wait(thread_id, &this->_native_mutex, owned_mutexes))
    {
        tracking_error = ft_global_error_stack_pop_newest();
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        if (tracking_error == FT_ERR_SUCCESSS)
            tracking_error = FT_ERR_INVALID_STATE;
        this->operation_error_push(tracking_error);
        return (FT_SUCCESS);
    }
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        this->operation_error_push(tracking_error);
        return (FT_SUCCESS);
    }
    mutex_error = pthread_mutex_lock(&this->_native_mutex);
    if (mutex_error != 0)
    {
        pt_lock_tracking::notify_released(thread_id, &this->_native_mutex);
        ft_global_error_stack_pop_newest();
        this->operation_error_push(FT_ERR_INVALID_STATE);
        return (FT_SUCCESS);
    }
    this->_owner.store(thread_id, std::memory_order_relaxed);
    this->_lock.store(true, std::memory_order_release);
    this->_lock_depth.store(1, std::memory_order_relaxed);
    pt_lock_tracking::notify_acquired(thread_id, &this->_native_mutex);
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
        this->operation_error_push(tracking_error);
    else
        this->operation_error_push(FT_ERR_SUCCESSS);
    return (FT_SUCCESS);
}
