#include "../PThread/pthread_internal.hpp"
#include "ft_price_definition.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_price_definition::_last_error = FT_ERR_SUCCESS;

ft_price_definition::ft_price_definition() noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0),
      _maximum_value(0), _mutex(ft_nullptr),
      _initialised_state(ft_price_definition::_state_uninitialised)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_price_definition::~ft_price_definition() noexcept
{
    if (this->_initialised_state == ft_price_definition::_state_uninitialised)
        return ;
    if (this->_initialised_state == ft_price_definition::_state_initialised)
        (void)this->destroy();
    return ;
}

void ft_price_definition::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_price_definition lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_price_definition::set_error(int error_code) const noexcept
{
    ft_price_definition::_last_error = error_code;
    return ;
}

void ft_price_definition::abort_if_not_initialised(const char *method_name) const
{
    if (this->_initialised_state == ft_price_definition::_state_initialised)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialised");
    return ;
}

int ft_price_definition::initialize() noexcept
{
    if (this->_initialised_state == ft_price_definition::_state_initialised)
    {
        this->abort_lifecycle_error("ft_price_definition::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_item_id = 0;
    this->_rarity = 0;
    this->_base_value = 0;
    this->_minimum_value = 0;
    this->_maximum_value = 0;
    this->_initialised_state = ft_price_definition::_state_initialised;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_price_definition::initialize(const ft_price_definition &other) noexcept
{
    int initialize_error;

    if (other._initialised_state != ft_price_definition::_state_initialised)
    {
        other.abort_lifecycle_error("ft_price_definition::initialize(copy)",
            "source object is not initialised");
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
    this->_item_id = other._item_id;
    this->_rarity = other._rarity;
    this->_base_value = other._base_value;
    this->_minimum_value = other._minimum_value;
    this->_maximum_value = other._maximum_value;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_price_definition::initialize(ft_price_definition &&other) noexcept
{
    return (this->initialize(static_cast<const ft_price_definition &>(other)));
}

int ft_price_definition::initialize(int item_id, int rarity, int base_value,
    int minimum_value, int maximum_value) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_item_id = item_id;
    this->_rarity = rarity;
    this->_base_value = base_value;
    this->_minimum_value = minimum_value;
    this->_maximum_value = maximum_value;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_price_definition::destroy() noexcept
{
    int disable_error;

    if (this->_initialised_state != ft_price_definition::_state_initialised)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_item_id = 0;
    this->_rarity = 0;
    this->_base_value = 0;
    this->_minimum_value = 0;
    this->_maximum_value = 0;
    this->_initialised_state = ft_price_definition::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_price_definition::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialised("ft_price_definition::enable_thread_safety");
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

int ft_price_definition::disable_thread_safety() noexcept
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

bool ft_price_definition::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_price_definition::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
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

int ft_price_definition::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_price_definition::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_price_definition::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_price_definition::get_item_id() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_item_id");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_item_id);
}

void ft_price_definition::set_item_id(int item_id) noexcept
{
    this->abort_if_not_initialised("ft_price_definition::set_item_id");
    this->_item_id = item_id;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_rarity() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_rarity");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rarity);
}

void ft_price_definition::set_rarity(int rarity) noexcept
{
    this->abort_if_not_initialised("ft_price_definition::set_rarity");
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_base_value() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_base_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_base_value);
}

void ft_price_definition::set_base_value(int base_value) noexcept
{
    this->abort_if_not_initialised("ft_price_definition::set_base_value");
    this->_base_value = base_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_minimum_value() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_minimum_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_minimum_value);
}

void ft_price_definition::set_minimum_value(int minimum_value) noexcept
{
    this->abort_if_not_initialised("ft_price_definition::set_minimum_value");
    this->_minimum_value = minimum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_price_definition::get_maximum_value() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_maximum_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_maximum_value);
}

void ft_price_definition::set_maximum_value(int maximum_value) noexcept
{
    this->abort_if_not_initialised("ft_price_definition::set_maximum_value");
    this->_maximum_value = maximum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_price_definition::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialised("ft_price_definition::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

int ft_price_definition::get_error() const noexcept
{
    return (ft_price_definition::_last_error);
}

const char *ft_price_definition::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}
