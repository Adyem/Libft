#include "kv_store.hpp"

#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

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

kv_store_entry::kv_store_entry() noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _initialized_state(kv_store_entry::_state_uninitialized)
    , _mutex(ft_nullptr)
{
    return ;
}

kv_store_entry::~kv_store_entry() noexcept
{
    if (this->_initialized_state == kv_store_entry::_state_initialized)
        (void)this->destroy();
    return ;
}

int kv_store_entry::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("kv_store_entry::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::disable_thread_safety() noexcept
{
    int destroy_error;

    this->abort_if_not_initialized("kv_store_entry::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool kv_store_entry::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int kv_store_entry::initialize() noexcept
{
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
    if (value_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    expiration_error = other.has_expiration(copied_has_expiration);
    if (expiration_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    expiration_error = other.get_expiration(copied_expiration_timestamp);
    if (expiration_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    if (this->set_value(copied_value) != FT_ERR_SUCCESS)
    {
        this->_initialized_state = kv_store_entry::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    if (this->configure_expiration(copied_has_expiration,
            copied_expiration_timestamp) != FT_ERR_SUCCESS)
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
        return (FT_ERR_INVALID_STATE);
    destroy_error = this->disable_thread_safety();
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
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_value = value;
    if (this->_value.c_str() == ft_nullptr)
    {
        pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_NO_MEMORY);
    }
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::set_value(const char *value_string) noexcept
{
    ft_string temporary_value;

    this->abort_if_not_initialized("kv_store_entry::set_value(const char *)");
    if (value_string == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    temporary_value = value_string;
    if (temporary_value.c_str() == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    return (this->set_value(temporary_value));
}

int kv_store_entry::copy_value(ft_string &destination) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::copy_value");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    destination = this->_value;
    if (destination.c_str() == ft_nullptr)
    {
        pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_NO_MEMORY);
    }
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::get_value_pointer(const char **value_pointer) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::get_value_pointer");
    if (value_pointer == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    *value_pointer = this->_value.c_str();
    if (*value_pointer == ft_nullptr)
    {
        pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_STATE);
    }
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::configure_expiration(bool has_expiration, long long expiration_timestamp) noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::configure_expiration");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_has_expiration = has_expiration;
    if (has_expiration)
        this->_expiration_timestamp = expiration_timestamp;
    else
        this->_expiration_timestamp = 0;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::has_expiration(bool &has_expiration) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::has_expiration");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    has_expiration = this->_has_expiration;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::get_expiration(long long &expiration_timestamp) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::get_expiration");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    expiration_timestamp = this->_expiration_timestamp;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

int kv_store_entry::is_expired(long long current_time, bool &expired) const noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("kv_store_entry::is_expired");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_has_expiration == false)
    {
        expired = false;
        unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        if (unlock_error != FT_ERR_SUCCESS)
            return (unlock_error);
        return (FT_ERR_SUCCESS);
    }
    if (current_time < 0)
    {
        expired = false;
        pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    expired = this->_expiration_timestamp <= current_time;
    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}
