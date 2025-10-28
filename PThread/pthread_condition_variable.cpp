#include "condition.hpp"
#include "mutex.hpp"
#include "pthread.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
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
    *error_code = ER_SUCCESS;
    return (true);
}

pt_condition_variable::pt_condition_variable()
    : _condition(), _mutex(), _condition_initialized(false), _mutex_initialized(false), _error_code(ER_SUCCESS),
    _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return ;
    }
    this->_mutex_initialized = true;
#if defined(CLOCK_MONOTONIC)
    pthread_condattr_t condition_attributes;

    if (pthread_condattr_init(&condition_attributes) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return ;
    }
    if (pthread_condattr_setclock(&condition_attributes, CLOCK_MONOTONIC) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        pthread_condattr_destroy(&condition_attributes);
        return ;
    }
    if (pt_cond_init(&this->_condition, &condition_attributes) != 0)
    {
        this->set_error(ft_errno);
        pthread_condattr_destroy(&condition_attributes);
        return ;
    }
    pthread_condattr_destroy(&condition_attributes);
#else
    if (pt_cond_init(&this->_condition, ft_nullptr) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
#endif
    this->_condition_initialized = true;
    this->set_error(ER_SUCCESS);
    return ;
}

pt_condition_variable::~pt_condition_variable()
{
    if (this->_condition_initialized)
        pt_cond_destroy(&this->_condition);
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

void pt_condition_variable::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        ft_errno = error;
        return ;
    }
    this->_error_code = error;
    ft_errno = error;
    this->unlock_internal(lock_acquired);
    return ;
}

int pt_condition_variable::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void pt_condition_variable::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void pt_condition_variable::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

int pt_condition_variable::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void pt_condition_variable::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool pt_condition_variable::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<pt_condition_variable *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int pt_condition_variable::lock_state(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<pt_condition_variable *>(this)->set_error(ft_errno);
    else
        const_cast<pt_condition_variable *>(this)->set_error(ER_SUCCESS);
    return (result);
}

void pt_condition_variable::unlock_state(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
        const_cast<pt_condition_variable *>(this)->set_error(this->_state_mutex->get_error());
    else
        const_cast<pt_condition_variable *>(this)->set_error(entry_errno);
    return ;
}

int pt_condition_variable::wait(pt_mutex &mutex)
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!mutex.lockState())
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        int unlock_error;

        unlock_error = mutex.get_error();
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(unlock_error);
        return (-1);
    }
    if (pt_cond_wait(&this->_condition, &this->_mutex) != 0)
    {
        int wait_error;

        wait_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        if (mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(wait_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        int unlock_error;

        unlock_error = ft_map_system_error(errno);
        if (mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(unlock_error);
        return (-1);
    }
    if (mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        int relock_error;

        relock_error = mutex.get_error();
        this->set_error(relock_error);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::wait_for(pt_mutex &mutex, const struct timespec &relative_time)
{
    struct timespec absolute_time;
    int conversion_error;

    if (!compute_wait_deadline(relative_time, &absolute_time, &conversion_error))
    {
        this->set_error(conversion_error);
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
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!mutex.lockState())
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        int unlock_error;

        unlock_error = mutex.get_error();
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(unlock_error);
        return (-1);
    }
    wait_result = pthread_cond_timedwait(&this->_condition, &this->_mutex, &absolute_time);
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        int unlock_error;

        unlock_error = ft_map_system_error(errno);
        if (mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(unlock_error);
        return (-1);
    }
    if (mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        int relock_error;

        relock_error = mutex.get_error();
        this->set_error(relock_error);
        return (-1);
    }
    if (wait_result == 0)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    if (wait_result == ETIMEDOUT)
    {
        this->set_error(ER_SUCCESS);
        return (ETIMEDOUT);
    }
    this->set_error(ft_map_system_error(wait_result));
    return (-1);
}

int pt_condition_variable::signal()
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (pt_cond_signal(&this->_condition) != 0)
    {
        int signal_error;

        signal_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(signal_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::broadcast()
{
    bool lock_acquired;
    bool condition_initialized;
    bool mutex_initialized;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    condition_initialized = this->_condition_initialized;
    mutex_initialized = this->_mutex_initialized;
    this->unlock_internal(lock_acquired);
    if (!condition_initialized || !mutex_initialized)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (pt_cond_broadcast(&this->_condition) != 0)
    {
        int broadcast_error;

        broadcast_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(broadcast_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::get_error() const
{
    bool lock_acquired;
    int error_value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (this->_error_code);
    error_value = this->_error_code;
    this->unlock_internal(lock_acquired);
    return (error_value);
}

const char *pt_condition_variable::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}
