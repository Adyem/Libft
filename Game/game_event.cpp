#include "../PThread/pthread_internal.hpp"
#include "game_event.hpp"
#include "../Template/move.hpp"
#include <climits>
#include <new>

thread_local int ft_event::_last_error = FT_ERR_SUCCESS;

ft_event::ft_event() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_event::~ft_event() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_event::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_event::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return ;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_event::get_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_id;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_event::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;

    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_id = id;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_event::get_duration() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_duration;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_event::set_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_duration = duration;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_event::add_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    if (duration < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (duration > 0 && this->_duration > INT_MAX - duration)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return (FT_ERR_OUT_OF_RANGE);
    }
    this->_duration += duration;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void ft_event::sub_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (duration < 0 || duration > this->_duration)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_duration -= duration;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_event::get_modifier1() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier1);
}

void ft_event::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_event::get_modifier2() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier2);
}

void ft_event::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_event::get_modifier3() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier3);
}

void ft_event::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_event::get_modifier4() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier4);
}

void ft_event::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_event::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

const ft_function<void(ft_world&, ft_event&)> &ft_event::get_callback() const noexcept
{
    this->set_error(FT_ERR_SUCCESS);
    return (this->_callback);
}

void ft_event::set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    this->_callback = ft_move(callback);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_event::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_event::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_event::is_thread_safe() const noexcept
{
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_event::get_error() const noexcept
{
    return (ft_event::_last_error);
}

const char *ft_event::get_error_str() const noexcept
{
    return (ft_strerror(ft_event::_last_error));
}

void ft_event::set_error(int err) const noexcept
{
    ft_event::_last_error = err;
    return ;
}
