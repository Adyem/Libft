#include "../PThread/pthread_internal.hpp"
#include "ft_rarity_band.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_rarity_band::_last_error = FT_ERR_SUCCESS;

ft_rarity_band::ft_rarity_band() noexcept
    : _rarity(0), _value_multiplier(1.0), _mutex(ft_nullptr),
      _initialized_state(ft_rarity_band::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_rarity_band::~ft_rarity_band() noexcept
{
    if (this->_initialized_state == ft_rarity_band::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_rarity_band::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_rarity_band::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_rarity_band lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_rarity_band::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_rarity_band::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

void ft_rarity_band::set_error(int error_code) const noexcept
{
    ft_rarity_band::_last_error = error_code;
    return ;
}

int ft_rarity_band::initialize() noexcept
{
    if (this->_initialized_state == ft_rarity_band::_state_initialized)
    {
        this->abort_lifecycle_error("ft_rarity_band::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_rarity = 0;
    this->_value_multiplier = 1.0;
    this->_initialized_state = ft_rarity_band::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_rarity_band::initialize(const ft_rarity_band &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_rarity_band::_state_initialized)
    {
        other.abort_lifecycle_error("ft_rarity_band::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_rarity = other._rarity;
    this->_value_multiplier = other._value_multiplier;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_rarity_band::initialize(ft_rarity_band &&other) noexcept
{
    return (this->initialize(static_cast<const ft_rarity_band &>(other)));
}

int ft_rarity_band::initialize(int rarity, double value_multiplier) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_rarity = rarity;
    this->_value_multiplier = value_multiplier;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_rarity_band::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_rarity_band::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_rarity = 0;
    this->_value_multiplier = 1.0;
    this->_initialized_state = ft_rarity_band::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_rarity_band::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_rarity_band::enable_thread_safety");
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

int ft_rarity_band::disable_thread_safety() noexcept
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

bool ft_rarity_band::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_rarity_band::lock_internal(bool *lock_acquired) const noexcept
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

int ft_rarity_band::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_rarity_band::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_rarity_band::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_rarity_band::get_rarity() const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::get_rarity");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rarity);
}

void ft_rarity_band::set_rarity(int rarity) noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::set_rarity");
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

double ft_rarity_band::get_value_multiplier() const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::get_value_multiplier");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_value_multiplier);
}

void ft_rarity_band::set_value_multiplier(double value_multiplier) noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::set_value_multiplier");
    this->_value_multiplier = value_multiplier;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_rarity_band::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_rarity_band::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

int ft_rarity_band::get_error() const noexcept
{
    return (ft_rarity_band::_last_error);
}

const char *ft_rarity_band::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
