#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "pthread_lock_tracking.hpp"

pt_recursive_mutex::pt_recursive_mutex()
    : _owner(0), _lock(false), _lock_depth(0), _error(FT_ERR_SUCCESSS),
    _native_initialized(false), _state_mutex(ft_nullptr)
{
    int previous_errno;

    previous_errno = ft_errno;
    ft_bzero(&this->_native_mutex, sizeof(pthread_mutex_t));
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->set_error(FT_ERR_INVALID_STATE);
        ft_set_errno_locked(previous_errno);
        return ;
    }
    this->_native_initialized = true;
    this->set_error(FT_ERR_SUCCESSS);
    ft_set_errno_locked(previous_errno);
    return ;
}

pt_recursive_mutex::~pt_recursive_mutex()
{
    int previous_errno;

    previous_errno = ft_errno;
    if (this->_native_initialized)
    {
        pthread_mutex_destroy(&this->_native_mutex);
        this->_native_initialized = false;
    }
    this->teardown_thread_safety();
    ft_set_errno_locked(previous_errno);
    return ;
}

void    pt_recursive_mutex::set_error(int error) const
{
    ft_errno_mutex().lock();
    this->_error = error;
    ft_sys_errno = error;
    ft_errno_mutex().unlock();
    return ;
}

bool pt_recursive_mutex::ensure_native_mutex() const
{
    bool lock_acquired;
    bool initialized;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    initialized = this->_native_initialized;
    if (initialized)
    {
        this->unlock_internal(lock_acquired);
        return (true);
    }
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    this->_native_initialized = true;
    this->unlock_internal(lock_acquired);
    return (true);
}

bool pt_recursive_mutex::lockState() const
{
    return (this->_lock.load(std::memory_order_acquire));
}

int pt_recursive_mutex::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void pt_recursive_mutex::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void pt_recursive_mutex::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    return ;
}

int pt_recursive_mutex::lock_state(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<pt_recursive_mutex *>(this)->set_error(ft_errno);
    else
        const_cast<pt_recursive_mutex *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

void pt_recursive_mutex::unlock_state(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<pt_recursive_mutex *>(this)->set_error(this->_state_mutex->get_error());
    else
        const_cast<pt_recursive_mutex *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

bool pt_recursive_mutex::is_owned_by_thread(pthread_t thread_id) const
{
    pthread_t owner_thread;
    pt_mutex_vector owned_mutexes;
    ft_size_t index;
    bool lock_flag;

    ft_errno = FT_ERR_SUCCESSS;
    lock_flag = this->_lock.load(std::memory_order_acquire);
    if (!lock_flag)
    {
        return (false);
    }
    owner_thread = this->_owner.load(std::memory_order_relaxed);
    if (owner_thread != 0)
    {
        bool matches_owner;

        matches_owner = (pt_thread_equal(owner_thread, thread_id) != 0);
        ft_errno = FT_ERR_SUCCESSS;
        return (matches_owner);
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (false);
    index = 0;
    while (index < owned_mutexes.size())
    {
        if (owned_mutexes[index] == &this->_native_mutex)
        {
            ft_errno = FT_ERR_SUCCESSS;
            return (true);
        }
        index += 1;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (false);
}

int pt_recursive_mutex::get_error() const
{
    int error_value;

    ft_errno_mutex().lock();
    error_value = this->_error;
    ft_errno_mutex().unlock();
    return (error_value);
}

const char *pt_recursive_mutex::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

pthread_mutex_t   *pt_recursive_mutex::get_native_mutex() const
{
    if (!this->ensure_native_mutex())
        return (ft_nullptr);
    this->set_error(FT_ERR_SUCCESSS);
    return (&this->_native_mutex);
}
