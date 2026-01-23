#include "thread.hpp"
#include <cerrno>
#include "mutex.hpp"
#include <new>
#include "../Template/move.hpp"

ft_thread::start_payload::start_payload()
    : function()
{
    return ;
}

ft_thread::start_payload::~start_payload()
{
    return ;
}

void *ft_thread::start_routine(void *data)
{
    std::shared_ptr<start_payload> *payload_capsule;
    std::shared_ptr<start_payload> payload;

    payload_capsule = static_cast<std::shared_ptr<start_payload> *>(data);
    if (payload_capsule == ft_nullptr)
        return (ft_nullptr);
    payload = *payload_capsule;
    delete payload_capsule;
    if (!payload)
        return (ft_nullptr);
    payload->function();
    return (ft_nullptr);
}

void ft_thread::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

ft_thread::ft_thread()
    : _thread(), _joinable(false), _error_code(FT_ERR_SUCCESSS), _start_payload(),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    return ;
}

ft_thread::~ft_thread()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        if (this->_joinable)
            this->detach_locked();
        else
            this->_start_payload.reset();
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error(ft_errno);
    this->teardown_thread_safety();
    return ;
}

ft_thread::ft_thread(ft_thread &&other)
    : _thread(), _joinable(false), _error_code(FT_ERR_SUCCESSS), _start_payload(),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;

    lock_acquired = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_thread = other._thread;
    this->_joinable = other._joinable;
    this->_error_code = other._error_code;
    this->_start_payload = ft_move(other._start_payload);
    if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
    {
        this->_state_mutex = other._state_mutex;
        this->_thread_safe_enabled = true;
        other._state_mutex = ft_nullptr;
        other._thread_safe_enabled = false;
    }
    other._joinable = false;
    other._error_code = FT_ERR_SUCCESSS;
    other.unlock_internal(lock_acquired);
    return ;
}

ft_thread &ft_thread::operator=(ft_thread &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;

    if (this != &other)
    {
        this_lock_acquired = false;
        if (this->lock_internal(&this_lock_acquired) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        other_lock_acquired = false;
        if (other.lock_internal(&other_lock_acquired) != 0)
        {
            this->unlock_internal(this_lock_acquired);
            this->set_error(ft_errno);
            return (*this);
        }
        if (this->_joinable)
            this->detach_locked();
        this->_thread = other._thread;
        this->_joinable = other._joinable;
        this->_error_code = other._error_code;
        this->_start_payload = ft_move(other._start_payload);
        this->teardown_thread_safety();
        if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
        {
            this->_state_mutex = other._state_mutex;
            this->_thread_safe_enabled = true;
            other._state_mutex = ft_nullptr;
            other._thread_safe_enabled = false;
        }
        other._joinable = false;
        other._error_code = FT_ERR_SUCCESSS;
        other._start_payload.reset();
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
    }
    return (*this);
}

bool ft_thread::joinable() const
{
    bool lock_acquired;
    bool joinable_state;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_thread *>(this)->set_error(ft_errno);
        return (false);
    }
    joinable_state = this->_joinable;
    const_cast<ft_thread *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (joinable_state);
}

void ft_thread::join()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (!this->_joinable)
    {
        this->_start_payload.reset();
        this->set_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (pt_thread_join(this->_thread, ft_nullptr) != 0)
    {
        int join_error;

        join_error = ft_errno;
        this->set_error(join_error);
        if (join_error == ESRCH || join_error == EINVAL)
        {
            this->_joinable = false;
            this->_start_payload.reset();
        }
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_joinable = false;
    this->_start_payload.reset();
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_thread::detach()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->detach_locked();
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_thread::get_error() const
{
    return (this->_error_code);
}

const char *ft_thread::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

int ft_thread::enable_thread_safety()
{
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    state_mutex = new (std::nothrow) pt_mutex();
    if (state_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    int mutex_error = state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        delete state_mutex;
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

void ft_thread::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

bool ft_thread::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_thread *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

int ft_thread::lock(bool *lock_acquired) const
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        this->_error_code = ft_errno;
        return (result);
    }
    this->_error_code = FT_ERR_SUCCESSS;
    return (result);
}

void ft_thread::unlock(bool lock_acquired) const
{
    int mutex_error;

    ft_errno = FT_ERR_SUCCESSS;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        this->_error_code = FT_ERR_SUCCESSS;
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->_error_code = mutex_error;
        ft_errno = mutex_error;
        return ;
    }
    this->_error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

int ft_thread::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    int mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = FT_ERR_SUCCESSS;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            ft_errno = FT_ERR_SUCCESSS;
            return (0);
        }
        ft_errno = mutex_error;
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

void ft_thread::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    this->_state_mutex->unlock(THREAD_ID);
    int mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_errno = mutex_error;
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void ft_thread::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

int ft_thread::detach_locked()
{
    if (!this->_joinable)
    {
        this->_start_payload.reset();
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (pt_thread_detach(this->_thread) != 0)
    {
        int detach_error;

        detach_error = ft_errno;
        this->set_error(detach_error);
        if (detach_error == ESRCH || detach_error == EINVAL)
        {
            this->_joinable = false;
            this->_start_payload.reset();
        }
        return (-1);
    }
    this->_joinable = false;
    this->_start_payload.reset();
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}
