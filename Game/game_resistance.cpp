#include "game_resistance.hpp"
#include "../Errno/errno.hpp"
#include <new>

thread_local int ft_resistance::_last_error = FT_ERR_SUCCESS;

ft_resistance::ft_resistance() noexcept
    : _percent_value(0), _flat_value(0), _mutex(ft_nullptr)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_resistance::~ft_resistance() noexcept
{
    (void)this->disable_thread_safety();
    return ;
}

int ft_resistance::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_resistance::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    if (this->_mutex == ft_nullptr)
        return ;
    (void)this->_mutex->unlock();
    return ;
}

int ft_resistance::set_percent(int percent_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_percent_value = percent_value;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_flat(int flat_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_values(int percent_value, int flat_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_percent_value = percent_value;
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::reset() noexcept
{
    return (this->set_values(0, 0));
}

int ft_resistance::get_percent() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int percent_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    percent_value = this->_percent_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (percent_value);
}

int ft_resistance::get_flat() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int flat_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    flat_value = this->_flat_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (flat_value);
}

int ft_resistance::enable_thread_safety() noexcept
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

int ft_resistance::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_resistance::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_resistance::get_error() const noexcept
{
    return (ft_resistance::_last_error);
}

const char *ft_resistance::get_error_str() const noexcept
{
    return (ft_strerror(ft_resistance::_last_error));
}

void ft_resistance::set_error(int error) const noexcept
{
    ft_resistance::_last_error = error;
    return ;
}
