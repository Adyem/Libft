#include "kv_store.hpp"

#include <new>

#include "../Basic/basic.hpp"
#include "../Template/move.hpp"

static int storage_kv_capture_string_error(const ft_string &value) noexcept
{
    unsigned long long operation_id = value.last_operation_id();

    if (operation_id == 0)
        return (FT_ERR_SUCCESS);
    int error_code = value.pop_operation_error(operation_id);

    if (error_code != FT_ERR_SUCCESS)
        return (error_code);
    return (FT_ERR_SUCCESS);
}

void kv_store_entry::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

void kv_store_entry::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

void kv_store_entry::reinitialize_mutex() noexcept
{
    this->_mutex.~pt_mutex();
    new (&this->_mutex) pt_mutex();
    return ;
}

int kv_store_entry::lock_entry(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);
    int guard_error = ft_global_error_stack_drop_last_error();

    if (guard_error != FT_ERR_SUCCESS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (guard_error);
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESS);
}

kv_store_entry::kv_store_entry() noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex()
{
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

kv_store_entry::kv_store_entry(const kv_store_entry &other) noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_value = other._value;
    {
        int value_error = storage_kv_capture_string_error(this->_value);

        if (value_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(value_error);
            return ;
        }
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

kv_store_entry::kv_store_entry(kv_store_entry &&other) noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_value = ft_move(other._value);
    {
        int value_error = storage_kv_capture_string_error(this->_value);

        if (value_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(value_error);
            return ;
        }
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    other._has_expiration = false;
    other._expiration_timestamp = 0;
    other._value = ft_string();
    {
        int other_error = storage_kv_capture_string_error(other._value);

        if (other_error != FT_ERR_SUCCESS)
        {
            other.set_error_unlocked(other_error);
            return ;
        }
    }
    other.set_error_unlocked(FT_ERR_SUCCESS);
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return ;
}

kv_store_entry &kv_store_entry::operator=(const kv_store_entry &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        this->set_error_unlocked(FT_ERR_SUCCESS);
        return (*this);
    }
    lock_error = this->lock_entry(this_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_value = other._value;
    {
        int value_error = storage_kv_capture_string_error(this->_value);

        if (value_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(value_error);
            return (*this);
        }
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (*this);
}

kv_store_entry &kv_store_entry::operator=(kv_store_entry &&other) noexcept
{
    ft_unique_lock<pt_mutex> other_guard;
    ft_unique_lock<pt_mutex> new_guard;
    ft_string moved_value;
    bool moved_has_expiration;
    long long moved_expiration_timestamp;
    int lock_error;

    if (this == &other)
    {
        this->set_error_unlocked(FT_ERR_SUCCESS);
        return (*this);
    }
    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    moved_value = ft_move(other._value);
    {
        int moved_error = storage_kv_capture_string_error(moved_value);

        if (moved_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(moved_error);
            return (*this);
        }
    }
    moved_has_expiration = other._has_expiration;
    moved_expiration_timestamp = other._expiration_timestamp;
    other._has_expiration = false;
    other._expiration_timestamp = 0;
    other._value = ft_string();
    {
        int other_error = storage_kv_capture_string_error(other._value);

        if (other_error != FT_ERR_SUCCESS)
        {
            other.set_error_unlocked(other_error);
            return (*this);
        }
    }
    other.set_error_unlocked(FT_ERR_SUCCESS);
    this->reinitialize_mutex();
    lock_error = this->lock_entry(new_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_value = ft_move(moved_value);
    {
        int value_error = storage_kv_capture_string_error(this->_value);

        if (value_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(value_error);
            return (*this);
        }
    }
    this->_has_expiration = moved_has_expiration;
    this->_expiration_timestamp = moved_expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (*this);
}

kv_store_entry::~kv_store_entry() noexcept
{
    return ;
}

int kv_store_entry::set_value(const ft_string &value) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (-1);
    }
    this->_value = value;
    {
        int value_error = storage_kv_capture_string_error(this->_value);

        if (value_error != FT_ERR_SUCCESS)
        {
            this->set_error_unlocked(value_error);
            return (-1);
        }
    }
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::set_value(const char *value_string) noexcept
{
    ft_string temporary_value;

    if (value_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    temporary_value = value_string;
    {
        int temp_error = storage_kv_capture_string_error(temporary_value);

        if (temp_error != FT_ERR_SUCCESS)
        {
            this->set_error(temp_error);
            return (-1);
        }
    }
    return (this->set_value(temporary_value));
}

int kv_store_entry::copy_value(ft_string &destination) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    destination = this->_value;
    {
        int dest_error = storage_kv_capture_string_error(destination);

        if (dest_error != FT_ERR_SUCCESS)
        {
            const_cast<kv_store_entry *>(this)->set_error_unlocked(dest_error);
            return (-1);
        }
    }
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::get_value_pointer(const char **value_pointer) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    if (value_pointer == ft_nullptr)
    {
        const_cast<kv_store_entry *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    *value_pointer = this->_value.c_str();
    if (*value_pointer == ft_nullptr)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::configure_expiration(bool has_expiration, long long expiration_timestamp) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        return (-1);
    }
    this->_has_expiration = has_expiration;
    if (has_expiration)
        this->_expiration_timestamp = expiration_timestamp;
    else
        this->_expiration_timestamp = 0;
    this->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::has_expiration(bool &has_expiration) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    has_expiration = this->_has_expiration;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::get_expiration(long long &expiration_timestamp) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    expiration_timestamp = this->_expiration_timestamp;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::is_expired(long long current_time, bool &expired) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    if (this->_has_expiration == false)
    {
        expired = false;
        const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
        return (0);
    }
    if (current_time < 0)
    {
        expired = false;
        const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    expired = this->_expiration_timestamp <= current_time;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESS);
    return (0);
}

int kv_store_entry::get_error() const noexcept
{
    return (this->_error_code);
}

const char *kv_store_entry::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
