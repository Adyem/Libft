#include "condition.hpp"
#include "mutex.hpp"
#include "pthread.hpp"
#include "pthread_internal.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <new>
#include <cerrno>
#include <time.h>

static bool compute_wait_deadline(const struct timespec &relative_time, struct timespec *absolute_time, int *error_code)
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
#if defined(CLOCK_MONOTONIC)
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0)
#else
    if (clock_gettime(CLOCK_REALTIME, &current_time) != 0)
#endif
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
    *error_code = FT_ERR_SUCCESSS;
    return (true);
}

pt_condition_variable::pt_condition_variable()
    : _condition(), _mutex(), _condition_initialized(false), _mutex_initialized(false),
    _state_mutex(ft_nullptr)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    this->_mutex_initialized = true;
#if defined(CLOCK_MONOTONIC)
    pthread_condattr_t condition_attributes;

    if (pthread_condattr_init(&condition_attributes) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return ;
    }
    if (pthread_condattr_setclock(&condition_attributes, CLOCK_MONOTONIC) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        pthread_condattr_destroy(&condition_attributes);
        return ;
    }
    if (pt_cond_init(&this->_condition, &condition_attributes) != 0)
    {
        pthread_condattr_destroy(&condition_attributes);
        return ;
    }
    pthread_condattr_destroy(&condition_attributes);
#else
    if (pt_cond_init(&this->_condition, ft_nullptr) != 0)
        return ;
#endif
    this->_condition_initialized = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

pt_condition_variable::~pt_condition_variable()
{
    if (this->_condition_initialized)
        pt_cond_destroy(&this->_condition);
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int pt_condition_variable::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int state_error = pt_mutex_lock_with_error(*this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        if (state_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            return (FT_ERR_SUCCESSS);
        }
        return (state_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

void pt_condition_variable::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    pt_mutex_unlock_with_error(*this->_state_mutex);
    return ;
}

void pt_condition_variable::teardown_thread_safety()
{
    pt_mutex_destroy(&this->_state_mutex);
    return ;
}

int pt_condition_variable::enable_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    int mutex_error = pt_mutex_create_with_error(&this->_state_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

void pt_condition_variable::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

bool pt_condition_variable::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_state_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

int pt_condition_variable::lock_state(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(result);
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

void pt_condition_variable::unlock_state(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr)
    {
        int state_error = ft_global_error_stack_peek_last_error();
        if (state_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(state_error);
            return ;
        }
    }
    else
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int pt_condition_variable::wait(pt_mutex &mutex)
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!mutex.lockState())
    {
        ft_global_error_stack_push(FT_ERR_MUTEX_NOT_OWNER);
        return (-1);
    }
    if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    if (pt_mutex_unlock_with_error(mutex) != FT_ERR_SUCCESSS)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        return (-1);
    }
    if (pt_cond_wait(&this->_condition, &this->_mutex) != 0)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        if (pt_mutex_lock_with_error(mutex) != FT_ERR_SUCCESSS)
            return (-1);
        return (-1);
    }
    if (pt_pthread_mutex_unlock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
    {
        if (pt_mutex_lock_with_error(mutex) != FT_ERR_SUCCESSS)
            return (-1);
        return (-1);
    }
    if (pt_mutex_lock_with_error(mutex) != FT_ERR_SUCCESSS)
        return (-1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int pt_condition_variable::wait_for(pt_mutex &mutex, const struct timespec &relative_time)
{
    struct timespec absolute_time;
    int conversion_error;

    if (!compute_wait_deadline(relative_time, &absolute_time, &conversion_error))
    {
        ft_global_error_stack_push(conversion_error);
        return (-1);
    }
    return (this->wait_until(mutex, absolute_time));
}

int pt_condition_variable::wait_until(pt_mutex &mutex, const struct timespec &absolute_time)
{
    int wait_result;
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!mutex.lockState())
    {
        ft_global_error_stack_push(FT_ERR_MUTEX_NOT_OWNER);
        return (-1);
    }
    if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    if (pt_mutex_unlock_with_error(mutex) != FT_ERR_SUCCESSS)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        return (-1);
    }
    wait_result = pthread_cond_timedwait(&this->_condition, &this->_mutex, &absolute_time);
    if (pt_pthread_mutex_unlock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
    {
        if (pt_mutex_lock_with_error(mutex) != FT_ERR_SUCCESSS)
            return (-1);
        return (-1);
    }
    if (pt_mutex_lock_with_error(mutex) != FT_ERR_SUCCESSS)
        return (-1);
    if (wait_result == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (0);
    }
    if (wait_result == ETIMEDOUT)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (ETIMEDOUT);
    }
    ft_global_error_stack_push(ft_map_system_error(wait_result));
    return (-1);
}

int pt_condition_variable::signal()
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    if (pt_cond_signal(&this->_condition) != 0)
    {
        int signal_error;

        signal_error = ft_global_error_stack_peek_last_error();
        pthread_mutex_unlock(&this->_mutex);
        ft_global_error_stack_push(signal_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

int pt_condition_variable::broadcast()
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    if (pt_cond_broadcast(&this->_condition) != 0)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        return (-1);
    }
    if (pt_pthread_mutex_unlock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
