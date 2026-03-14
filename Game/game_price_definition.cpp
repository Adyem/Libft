#include "../PThread/pthread_internal.hpp"
#include "ft_price_definition.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t ft_price_definition::_last_error = FT_ERR_SUCCESS;

ft_price_definition::ft_price_definition() noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0),
      _maximum_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_price_definition::ft_price_definition(const ft_price_definition &other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0),
      _maximum_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_price_definition::ft_price_definition(copy)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
    }
    return ;
}

ft_price_definition::ft_price_definition(ft_price_definition &&other) noexcept
    : _item_id(0), _rarity(0), _base_value(0), _minimum_value(0),
      _maximum_value(0), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_price_definition::ft_price_definition(move)",
            "source object is not initialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

ft_price_definition::~ft_price_definition() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        return ;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_price_definition::set_error(int32_t error_code) noexcept
{
    ft_price_definition::_last_error = error_code;
    return (error_code);
}

int32_t ft_price_definition::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_price_definition::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_item_id = 0;
    this->_rarity = 0;
    this->_base_value = 0;
    this->_minimum_value = 0;
    this->_maximum_value = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_price_definition::initialize(const ft_price_definition &other) noexcept
{
    int32_t initialize_error;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_price_definition::initialize(copy)",
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

int32_t ft_price_definition::initialize(ft_price_definition &&other) noexcept
{
    return (this->initialize(static_cast<const ft_price_definition &>(other)));
}

int32_t ft_price_definition::move(ft_price_definition &other) noexcept
{
    return (this->initialize(static_cast<ft_price_definition &&>(other)));
}

int32_t ft_price_definition::initialize(int32_t item_id, int32_t rarity, int32_t base_value,
    int32_t minimum_value, int32_t maximum_value) noexcept
{
    int32_t initialize_error;

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

int32_t ft_price_definition::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_item_id = 0;
    this->_rarity = 0;
    this->_base_value = 0;
    this->_minimum_value = 0;
    this->_maximum_value = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_price_definition::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::enable_thread_safety");
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

int32_t ft_price_definition::disable_thread_safety() noexcept
{
    int32_t destroy_error;

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

ft_bool ft_price_definition::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_price_definition::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_price_definition::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_price_definition::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_price_definition::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::unlock");
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}

int32_t ft_price_definition::get_item_id() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::get_item_id");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_item_id);
}

void ft_price_definition::set_item_id(int32_t item_id) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::set_item_id");
    this->_item_id = item_id;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_price_definition::get_rarity() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::get_rarity");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_rarity);
}

void ft_price_definition::set_rarity(int32_t rarity) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::set_rarity");
    this->_rarity = rarity;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_price_definition::get_base_value() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::get_base_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_base_value);
}

void ft_price_definition::set_base_value(int32_t base_value) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::set_base_value");
    this->_base_value = base_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_price_definition::get_minimum_value() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::get_minimum_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_minimum_value);
}

void ft_price_definition::set_minimum_value(int32_t minimum_value) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::set_minimum_value");
    this->_minimum_value = minimum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t ft_price_definition::get_maximum_value() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::get_maximum_value");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_maximum_value);
}

void ft_price_definition::set_maximum_value(int32_t maximum_value) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_price_definition::set_maximum_value");
    this->_maximum_value = maximum_value;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}


int32_t ft_price_definition::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_price_definition::get_error");
    return (ft_price_definition::_last_error);
}

const char *ft_price_definition::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_price_definition::get_error_str");
    return (ft_strerror(ft_price_definition::_last_error));
}
