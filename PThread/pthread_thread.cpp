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
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESSS)
    {
        if (this->_joinable)
            this->detach_locked();
        else
            this->_start_payload.reset();
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error(lock_error);
    this->teardown_thread_safety();
    return ;
}

ft_thread::ft_thread(ft_thread &&other)
    : _thread(), _joinable(false), _error_code(FT_ERR_SUCCESSS), _start_payload(),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
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
        int this_lock_error = this->lock_internal(&this_lock_acquired);
        if (this_lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(this_lock_error);
            return (*this);
        }
        other_lock_acquired = false;
        int other_lock_error = other.lock_internal(&other_lock_acquired);
        if (other_lock_error != FT_ERR_SUCCESSS)
        {
            this->unlock_internal(this_lock_acquired);
            this->set_error(other_lock_error);
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
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_thread *>(this)->set_error(lock_error);
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
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (!this->_joinable)
    {
        this->_start_payload.reset();
        this->set_error(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    int join_result = pt_thread_join(this->_thread, ft_nullptr);
    if (join_result != 0)
    {
        int join_error = ft_global_error_stack_last_error();

        this->set_error(join_error);
        if (join_result == ESRCH || join_result == EINVAL)
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
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
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

    result = this->lock_internal(lock_acquired);
    this->_error_code = result;
    return (result);
}

void ft_thread::unlock(bool lock_acquired) const
{
    int mutex_error;

    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        this->_error_code = FT_ERR_SUCCESSS;
        return ;
    }
    mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->_error_code = mutex_error;
        return ;
    }
    this->_error_code = FT_ERR_SUCCESSS;
    return ;
}

int ft_thread::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_state_mutex->lock(THREAD_ID);
    int mutex_error = this->_state_mutex->operation_error_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            return (FT_ERR_SUCCESSS);
        }
        return (mutex_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

void ft_thread::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    this->_state_mutex->unlock(THREAD_ID);
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
        return (FT_ERR_SUCCESSS);
    }
    int detach_result = pt_thread_detach(this->_thread);
    int detach_error = ft_global_error_stack_last_error();
    if (detach_result != 0)
    {
        this->set_error(detach_error);
        if (detach_result == ESRCH || detach_result == EINVAL)
        {
            this->_joinable = false;
            this->_start_payload.reset();
        }
        return (detach_error);
    }
    this->_joinable = false;
    this->_start_payload.reset();
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}
