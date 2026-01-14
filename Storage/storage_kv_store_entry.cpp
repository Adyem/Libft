#include "kv_store.hpp"

#include <new>

#include "../Libft/libft.hpp"
#include "../Template/move.hpp"

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

    if (local_guard.get_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    guard = ft_move(local_guard);
    return (FT_ERR_SUCCESSS);
}

kv_store_entry::kv_store_entry() noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

kv_store_entry::kv_store_entry(const kv_store_entry &other) noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_value = other._value;
    if (this->_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_value.get_error());
        return ;
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

kv_store_entry::kv_store_entry(kv_store_entry &&other) noexcept
    : _value()
    , _has_expiration(false)
    , _expiration_timestamp(0)
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return ;
    }
    this->_value = ft_move(other._value);
    if (this->_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_value.get_error());
        return ;
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    other._has_expiration = false;
    other._expiration_timestamp = 0;
    other._value = ft_string();
    if (other._value.get_error() != FT_ERR_SUCCESSS)
    {
        other.set_error_unlocked(other._value.get_error());
        return ;
    }
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

kv_store_entry &kv_store_entry::operator=(const kv_store_entry &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = this->lock_entry(this_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_value = other._value;
    if (this->_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_value.get_error());
        return (*this);
    }
    this->_has_expiration = other._has_expiration;
    this->_expiration_timestamp = other._expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
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
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return (*this);
    }
    lock_error = other.lock_entry(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    moved_value = ft_move(other._value);
    if (moved_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(moved_value.get_error());
        return (*this);
    }
    moved_has_expiration = other._has_expiration;
    moved_expiration_timestamp = other._expiration_timestamp;
    other._has_expiration = false;
    other._expiration_timestamp = 0;
    other._value = ft_string();
    if (other._value.get_error() != FT_ERR_SUCCESSS)
    {
        other.set_error_unlocked(other._value.get_error());
        return (*this);
    }
    other.set_error_unlocked(FT_ERR_SUCCESSS);
    this->reinitialize_mutex();
    lock_error = this->lock_entry(new_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (*this);
    }
    this->_value = ft_move(moved_value);
    if (this->_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_value.get_error());
        return (*this);
    }
    this->_has_expiration = moved_has_expiration;
    this->_expiration_timestamp = moved_expiration_timestamp;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
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
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (-1);
    }
    this->_value = value;
    if (this->_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(this->_value.get_error());
        return (-1);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
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
    if (temporary_value.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(temporary_value.get_error());
        return (-1);
    }
    return (this->set_value(temporary_value));
}

int kv_store_entry::copy_value(ft_string &destination) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    destination = this->_value;
    if (destination.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(destination.get_error());
        return (-1);
    }
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
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
    if (lock_error != FT_ERR_SUCCESSS)
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
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

int kv_store_entry::configure_expiration(bool has_expiration, long long expiration_timestamp) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error_unlocked(lock_error);
        return (-1);
    }
    this->_has_expiration = has_expiration;
    if (has_expiration)
        this->_expiration_timestamp = expiration_timestamp;
    else
        this->_expiration_timestamp = 0;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

int kv_store_entry::has_expiration(bool &has_expiration) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    has_expiration = this->_has_expiration;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

int kv_store_entry::get_expiration(long long &expiration_timestamp) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    expiration_timestamp = this->_expiration_timestamp;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

int kv_store_entry::is_expired(long long current_time, bool &expired) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_entry(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        const_cast<kv_store_entry *>(this)->set_error_unlocked(lock_error);
        return (-1);
    }
    if (this->_has_expiration == false)
    {
        expired = false;
        const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
        return (0);
    }
    if (current_time < 0)
    {
        expired = false;
        const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    expired = this->_expiration_timestamp <= current_time;
    const_cast<kv_store_entry *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
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
