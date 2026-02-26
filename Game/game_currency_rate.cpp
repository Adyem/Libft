#include "ft_currency_rate.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_currency_rate::_last_error = FT_ERR_SUCCESS;

ft_currency_rate::ft_currency_rate() noexcept
    : _currency_id(0), _rate_to_base(1.0), _display_precision(2),
      _mutex(ft_nullptr),
      _initialized_state(ft_currency_rate::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_currency_rate::~ft_currency_rate() noexcept
{
    if (this->_initialized_state == ft_currency_rate::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_currency_rate::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_currency_rate::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_currency_rate lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_currency_rate::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_currency_rate::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_currency_rate::initialize() noexcept
{
    if (this->_initialized_state == ft_currency_rate::_state_initialized)
    {
        this->abort_lifecycle_error("ft_currency_rate::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_currency_id = 0;
    this->_rate_to_base = 1.0;
    this->_display_precision = 2;
    this->_initialized_state = ft_currency_rate::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_currency_rate::initialize(const ft_currency_rate &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_currency_rate::_state_initialized)
    {
        other.abort_lifecycle_error("ft_currency_rate::initialize(copy)",
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
    this->_currency_id = other._currency_id;
    this->_rate_to_base = other._rate_to_base;
    this->_display_precision = other._display_precision;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_currency_rate::initialize(ft_currency_rate &&other) noexcept
{
    return (this->initialize(static_cast<const ft_currency_rate &>(other)));
}

int ft_currency_rate::initialize(int currency_id, double rate_to_base,
    int display_precision) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_currency_id = currency_id;
    this->_rate_to_base = rate_to_base;
    this->_display_precision = display_precision;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_currency_rate::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_currency_rate::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_currency_id = 0;
    this->_rate_to_base = 1.0;
    this->_display_precision = 2;
    this->_initialized_state = ft_currency_rate::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_currency_rate::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_currency_rate::enable_thread_safety");
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

int ft_currency_rate::disable_thread_safety() noexcept
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

bool ft_currency_rate::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_currency_rate::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
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

int ft_currency_rate::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = this->_mutex->unlock();
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_currency_rate::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_currency_rate::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_currency_rate::get_currency_id() const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::get_currency_id");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_currency_id);
}

void ft_currency_rate::set_currency_id(int currency_id) noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::set_currency_id");
    this->_currency_id = currency_id;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

double ft_currency_rate::get_rate_to_base() const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::get_rate_to_base");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rate_to_base);
}

void ft_currency_rate::set_rate_to_base(double rate_to_base) noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::set_rate_to_base");
    this->_rate_to_base = rate_to_base;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_currency_rate::get_display_precision() const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::get_display_precision");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_display_precision);
}

void ft_currency_rate::set_display_precision(int display_precision) noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::set_display_precision");
    this->_display_precision = display_precision;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_currency_rate::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_currency_rate::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

void ft_currency_rate::set_error(int error_code) const noexcept
{
    ft_currency_rate::_last_error = error_code;
    return ;
}

int ft_currency_rate::get_error() const noexcept
{
    return (ft_currency_rate::_last_error);
}

const char *ft_currency_rate::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
