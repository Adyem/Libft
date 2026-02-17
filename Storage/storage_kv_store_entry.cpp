#include "kv_store.hpp"

#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

void kv_store_entry::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "kv_store_entry lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void kv_store_entry::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == kv_store_entry::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int kv_store_entry::lock_entry() const noexcept
{
    this->abort_if_not_initialized("kv_store_entry::lock_entry");
    return (this->_mutex.lock());
}

int kv_store_entry::unlock_entry() const noexcept
{
    this->abort_if_not_initialized("kv_store_entry::unlock_entry");
    return (this->_mutex.unlock());
}

kv_store_entry::kv_store_entry() noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _initialized_state(kv_store_entry::_state_uninitialized)
    , _mutex()
{
    return ;
}

kv_store_entry::~kv_store_entry() noexcept
{
    if (this->_initialized_state == kv_store_entry::_state_uninitialized)
    {
        this->abort_lifecycle_error("kv_store_entry::~kv_store_entry",
            "called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == kv_store_entry::_state_initialized)
        (void)this->destroy();
    return ;
}

int kv_store_entry::initialize() noexcept
{
    int initialize_error;

    if (this->_initialized_state == kv_store_entry::_state_initialized)
    {
        this->abort_lifecycle_error("kv_store_entry::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_value = "";
    if (this->_value.c_str() == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    this->_has_expiration = false;
    this->_expiration_timestamp = 0;
    initialize_error = this->_mutex.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (initialize_error);
    }
    this->_initialized_state = kv_store_entry::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::initialize(const kv_store_entry &other) noexcept
{
    ft_string copied_value;
    bool copied_has_expiration;
    long long copied_expiration_timestamp;
    int value_error;
    int expiration_error;
    int init_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    other.abort_if_not_initialized("kv_store_entry::initialize(const kv_store_entry &)");
    init_error = this->initialize();
    if (init_error != FT_ERR_SUCCESS)
        return (init_error);
    value_error = other.copy_value(copied_value);
    if (value_error != 0)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    expiration_error = other.has_expiration(copied_has_expiration);
    if (expiration_error != 0)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    expiration_error = other.get_expiration(copied_expiration_timestamp);
    if (expiration_error != 0)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    if (this->set_value(copied_value) != 0)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    if (this->configure_expiration(copied_has_expiration, copied_expiration_timestamp) != 0)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::destroy() noexcept
{
    int destroy_error;

    if (this->_initialized_state != kv_store_entry::_state_initialized)
    {
        this->abort_lifecycle_error("kv_store_entry::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->_mutex.destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    this->_initialized_state = kv_store_entry::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::set_value(const ft_string &value) noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::set_value(const ft_string &)");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    this->_value = value;
    if (this->_value.c_str() == ft_nullptr)
    {
        this->unlock_entry();
        return (-1);
    }
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::set_value(const char *value_string) noexcept
{
    ft_string temporary_value;

    this->abort_if_not_initialized("kv_store_entry::set_value(const char *)");
    if (value_string == ft_nullptr)
        return (-1);
    temporary_value = value_string;
    if (temporary_value.c_str() == ft_nullptr)
        return (-1);
    return (this->set_value(temporary_value));
}

int kv_store_entry::copy_value(ft_string &destination) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::copy_value");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    destination = this->_value;
    if (destination.c_str() == ft_nullptr)
    {
        this->unlock_entry();
        return (-1);
    }
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::get_value_pointer(const char **value_pointer) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::get_value_pointer");
    if (value_pointer == ft_nullptr)
        return (-1);
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    *value_pointer = this->_value.c_str();
    if (*value_pointer == ft_nullptr)
    {
        this->unlock_entry();
        return (-1);
    }
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::configure_expiration(bool has_expiration, long long expiration_timestamp) noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::configure_expiration");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    this->_has_expiration = has_expiration;
    if (has_expiration)
        this->_expiration_timestamp = expiration_timestamp;
    else
        this->_expiration_timestamp = 0;
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::has_expiration(bool &has_expiration) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::has_expiration");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    has_expiration = this->_has_expiration;
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::get_expiration(long long &expiration_timestamp) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::get_expiration");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    expiration_timestamp = this->_expiration_timestamp;
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int kv_store_entry::is_expired(long long current_time, bool &expired) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::is_expired");
    lock_error = this->lock_entry();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_has_expiration == false)
    {
        expired = false;
        unlock_error = this->unlock_entry();
        if (unlock_error != FT_ERR_SUCCESS)
            return (-1);
        return (0);
    }
    if (current_time < 0)
    {
        expired = false;
        this->unlock_entry();
        return (-1);
    }
    expired = this->_expiration_timestamp <= current_time;
    unlock_error = this->unlock_entry();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
