#include "ft_vendor_profile.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

ft_vendor_profile::ft_vendor_profile() noexcept
    : _vendor_id(0), _buy_markup(1.0), _sell_multiplier(1.0), _tax_rate(0.0),
      _mutex(ft_nullptr),
      _initialized_state(ft_vendor_profile::_state_uninitialized)
{
    return ;
}

ft_vendor_profile::ft_vendor_profile(int vendor_id, double buy_markup,
    double sell_multiplier, double tax_rate) noexcept
    : _vendor_id(vendor_id), _buy_markup(buy_markup),
      _sell_multiplier(sell_multiplier), _tax_rate(tax_rate),
      _mutex(ft_nullptr),
      _initialized_state(ft_vendor_profile::_state_uninitialized)
{
    return ;
}

ft_vendor_profile::~ft_vendor_profile() noexcept
{
    if (this->_initialized_state == ft_vendor_profile::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_vendor_profile::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_vendor_profile::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_vendor_profile lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_vendor_profile::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_vendor_profile::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_vendor_profile::initialize() noexcept
{
    if (this->_initialized_state == ft_vendor_profile::_state_initialized)
    {
        this->abort_lifecycle_error("ft_vendor_profile::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_vendor_profile::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_vendor_profile::initialize(const ft_vendor_profile &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_vendor_profile::_state_initialized)
    {
        other.abort_lifecycle_error("ft_vendor_profile::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_vendor_id = other._vendor_id;
    this->_buy_markup = other._buy_markup;
    this->_sell_multiplier = other._sell_multiplier;
    this->_tax_rate = other._tax_rate;
    return (FT_ERR_SUCCESS);
}

int ft_vendor_profile::initialize(ft_vendor_profile &&other) noexcept
{
    return (this->initialize(static_cast<const ft_vendor_profile &>(other)));
}

int ft_vendor_profile::initialize(int vendor_id, double buy_markup,
    double sell_multiplier, double tax_rate) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_vendor_id = vendor_id;
    this->_buy_markup = buy_markup;
    this->_sell_multiplier = sell_multiplier;
    this->_tax_rate = tax_rate;
    return (FT_ERR_SUCCESS);
}

int ft_vendor_profile::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_vendor_profile::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_vendor_id = 0;
    this->_buy_markup = 1.0;
    this->_sell_multiplier = 1.0;
    this->_tax_rate = 0.0;
    this->_initialized_state = ft_vendor_profile::_state_destroyed;
    return (disable_error);
}

int ft_vendor_profile::enable_thread_safety() noexcept
{
    pt_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_vendor_profile::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_vendor_profile::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_vendor_profile::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_vendor_profile::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_vendor_profile::lock_internal(bool *lock_acquired) const noexcept
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

int ft_vendor_profile::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_vendor_profile::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_vendor_profile::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_vendor_profile::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_vendor_profile::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_vendor_profile::get_vendor_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_vendor_profile::get_vendor_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_vendor_id;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_vendor_profile::set_vendor_id(int vendor_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_vendor_profile::set_vendor_id");
    if (vendor_id < 0)
        return ;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_vendor_id = vendor_id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

double ft_vendor_profile::get_buy_markup() const noexcept
{
    bool lock_acquired;
    int lock_error;
    double value;

    this->abort_if_not_initialized("ft_vendor_profile::get_buy_markup");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_buy_markup;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_vendor_profile::set_buy_markup(double buy_markup) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_vendor_profile::set_buy_markup");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_buy_markup = buy_markup;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

double ft_vendor_profile::get_sell_multiplier() const noexcept
{
    bool lock_acquired;
    int lock_error;
    double value;

    this->abort_if_not_initialized("ft_vendor_profile::get_sell_multiplier");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_sell_multiplier;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_vendor_profile::set_sell_multiplier(double sell_multiplier) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_vendor_profile::set_sell_multiplier");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_sell_multiplier = sell_multiplier;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

double ft_vendor_profile::get_tax_rate() const noexcept
{
    bool lock_acquired;
    int lock_error;
    double value;

    this->abort_if_not_initialized("ft_vendor_profile::get_tax_rate");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0.0);
    value = this->_tax_rate;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_vendor_profile::set_tax_rate(double tax_rate) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_vendor_profile::set_tax_rate");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_tax_rate = tax_rate;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *ft_vendor_profile::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_vendor_profile::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
