#include "../PThread/pthread_internal.hpp"
#include "game_resistance.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_resistance::_last_error = FT_ERR_SUCCESS;

ft_resistance::ft_resistance() noexcept
    : _percent_value(0), _flat_value(0), _mutex(ft_nullptr),
      _initialized_state(ft_resistance::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_resistance::~ft_resistance() noexcept
{
    if (this->_initialized_state == ft_resistance::_state_initialized)
        (void)this->destroy();
    else
        this->_initialized_state = ft_resistance::_state_destroyed;
    return ;
}

void ft_resistance::abort_lifecycle_error(const char *method_name,
        const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_resistance lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_resistance::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_resistance::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_resistance::initialize() noexcept
{
    if (this->_initialized_state == ft_resistance::_state_initialized)
    {
        this->abort_lifecycle_error("ft_resistance::initialize",
            "already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_percent_value = 0;
    this->_flat_value = 0;
    this->_initialized_state = ft_resistance::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::destroy() noexcept
{
    if (this->_initialized_state != ft_resistance::_state_initialized)
    {
        this->_initialized_state = ft_resistance::_state_destroyed;
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_percent_value = 0;
    this->_flat_value = 0;
    if (this->_mutex != ft_nullptr)
        (void)this->disable_thread_safety();
    this->_initialized_state = ft_resistance::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::lock_internal(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_resistance::lock_internal");
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_resistance::unlock_internal(bool lock_acquired) const noexcept
{
    int unlock_error;

    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(unlock_error);
        return (unlock_error);
    }
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_percent(int percent_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_resistance::set_percent");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_percent_value = percent_value;
    this->set_error(FT_ERR_SUCCESS);
    lock_error = this->unlock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_flat(int flat_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_resistance::set_flat");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    this->_flat_value = flat_value;
    this->set_error(FT_ERR_SUCCESS);
    lock_error = this->unlock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    return (FT_ERR_SUCCESS);
}

int ft_resistance::set_values(int percent_value, int flat_value) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_resistance::set_values");
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
    lock_error = this->unlock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
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

    this->abort_if_not_initialized("ft_resistance::get_percent");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    percent_value = this->_percent_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    lock_error = this->unlock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    return (percent_value);
}

int ft_resistance::get_flat() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int flat_value;

    this->abort_if_not_initialized("ft_resistance::get_flat");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_resistance *>(this)->set_error(lock_error);
        return (0);
    }
    flat_value = this->_flat_value;
    const_cast<ft_resistance *>(this)->set_error(FT_ERR_SUCCESS);
    lock_error = this->unlock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    return (flat_value);
}

int ft_resistance::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
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
