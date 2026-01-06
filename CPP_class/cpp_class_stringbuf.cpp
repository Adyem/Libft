#include "class_stringbuf.hpp"
#include "class_nullptr.hpp"
#include "../Template/move.hpp"

ft_stringbuf::ft_stringbuf(const ft_string &string) noexcept
    : _storage(string), _position(0), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int storage_error;

    storage_error = ft_string::last_operation_error();
    if (storage_error != FT_ERR_SUCCESSS)
        this->set_error(storage_error);
    return ;
}

ft_stringbuf::ft_stringbuf(const ft_stringbuf &other) noexcept
    : _storage(other._storage), _position(0), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->_position = 0;
        this->_error_code = other_guard.get_error();
        ft_errno = other_guard.get_error();
        return ;
    }
    this->_position = other._position;
    this->_error_code = other._error_code;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->_error_code = ft_string::last_operation_error();
    this->set_error_unlocked(this->_error_code);
    ft_stringbuf::finalize_lock(other_guard);
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(const ft_stringbuf &other) noexcept
{
    if (this == &other)
    {
        this->set_error(other._error_code);
        return (*this);
    }
    const ft_stringbuf *first;
    const ft_stringbuf *second;
    ft_unique_lock<pt_mutex> first_guard;
    ft_unique_lock<pt_mutex> second_guard;

    first = this;
    second = &other;
    if (first > second)
    {
        const ft_stringbuf *temporary;

        temporary = first;
        first = second;
        second = temporary;
    }
    first_guard = ft_unique_lock<pt_mutex>(first->_mutex);
    if (first_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(first_guard.get_error());
        ft_stringbuf::finalize_lock(first_guard);
        return (*this);
    }
    second_guard = ft_unique_lock<pt_mutex>(second->_mutex);
    if (second_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(second_guard.get_error());
        ft_stringbuf::finalize_lock(second_guard);
        ft_stringbuf::finalize_lock(first_guard);
        return (*this);
    }
    this->_storage = other._storage;
    this->_position = other._position;
    this->_error_code = other._error_code;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->_error_code = ft_string::last_operation_error();
    this->set_error_unlocked(this->_error_code);
    ft_stringbuf::finalize_lock(second_guard);
    ft_stringbuf::finalize_lock(first_guard);
    return (*this);
}

ft_stringbuf::ft_stringbuf(ft_stringbuf &&other) noexcept
    : _storage(ft_move(other._storage)), _position(0), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->_position = 0;
        this->_error_code = other_guard.get_error();
        ft_errno = other_guard.get_error();
        return ;
    }
    this->_position = other._position;
    this->_error_code = other._error_code;
    other._position = 0;
    other._error_code = FT_ERR_SUCCESSS;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->_error_code = ft_string::last_operation_error();
    this->set_error_unlocked(this->_error_code);
    ft_stringbuf::finalize_lock(other_guard);
    return ;
}

ft_stringbuf &ft_stringbuf::operator=(ft_stringbuf &&other) noexcept
{
    if (this == &other)
        return (*this);
    const ft_stringbuf *first;
    const ft_stringbuf *second;
    ft_unique_lock<pt_mutex> first_guard;
    ft_unique_lock<pt_mutex> second_guard;

    first = this;
    second = &other;
    if (first > second)
    {
        const ft_stringbuf *temporary;

        temporary = first;
        first = second;
        second = temporary;
    }
    first_guard = ft_unique_lock<pt_mutex>(first->_mutex);
    if (first_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(first_guard.get_error());
        ft_stringbuf::finalize_lock(first_guard);
        return (*this);
    }
    second_guard = ft_unique_lock<pt_mutex>(second->_mutex);
    if (second_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(second_guard.get_error());
        ft_stringbuf::finalize_lock(second_guard);
        ft_stringbuf::finalize_lock(first_guard);
        return (*this);
    }
    this->_storage = ft_move(other._storage);
    this->_position = other._position;
    this->_error_code = other._error_code;
    other._position = 0;
    other._error_code = FT_ERR_SUCCESSS;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->_error_code = ft_string::last_operation_error();
    this->set_error_unlocked(this->_error_code);
    ft_stringbuf::finalize_lock(second_guard);
    ft_stringbuf::finalize_lock(first_guard);
    return (*this);
}

ft_stringbuf::~ft_stringbuf() noexcept
{
    return ;
}

void ft_stringbuf::set_error_unlocked(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void ft_stringbuf::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_stringbuf::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    if (local_guard.get_error() != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_mutex>();
        ft_errno = local_guard.get_error();
        return (local_guard.get_error());
    }
    guard = ft_move(local_guard);
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

void ft_stringbuf::finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != FT_ERR_SUCCESSS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

std::size_t ft_stringbuf::read(char *buffer, std::size_t count)
{
    int lock_error;
    std::size_t index;
    bool failure_occurred;

    ft_unique_lock<pt_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_stringbuf::finalize_lock(guard);
        return (0);
    }
    if (buffer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        ft_stringbuf::finalize_lock(guard);
        return (0);
    }
    index = 0;
    failure_occurred = false;
    while (index < count && this->_position < this->_storage.size())
    {
        const char *current;

        current = this->_storage.at(this->_position);
        if (!current)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            failure_occurred = true;
            break ;
        }
        buffer[index] = *current;
        index++;
        this->_position++;
    }
    if (!failure_occurred)
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_stringbuf::finalize_lock(guard);
    return (index);
}

bool ft_stringbuf::is_bad() const noexcept
{
    int lock_error;
    bool result;

    ft_unique_lock<pt_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_stringbuf::finalize_lock(guard);
        return (true);
    }
    result = (this->_error_code != FT_ERR_SUCCESSS);
    ft_errno = this->_error_code;
    ft_stringbuf::finalize_lock(guard);
    return (result);
}

int ft_stringbuf::get_error() const noexcept
{
    int lock_error;
    int error_value;

    ft_unique_lock<pt_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_stringbuf::finalize_lock(guard);
        return (lock_error);
    }
    error_value = this->_error_code;
    ft_errno = error_value;
    ft_stringbuf::finalize_lock(guard);
    return (error_value);
}

const char *ft_stringbuf::get_error_str() const noexcept
{
    int lock_error;
    const char *error_string;

    ft_unique_lock<pt_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_stringbuf::finalize_lock(guard);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    ft_errno = this->_error_code;
    ft_stringbuf::finalize_lock(guard);
    return (error_string);
}

ft_string ft_stringbuf::str() const
{
    int lock_error;
    const char *start;
    ft_string result;

    ft_unique_lock<pt_mutex> guard;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_stringbuf::finalize_lock(guard);
        return (ft_string(lock_error));
    }
    start = this->_storage.c_str();
    result = ft_string(start + this->_position);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error_unlocked(ft_string::last_operation_error());
    else
        this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_stringbuf::finalize_lock(guard);
    return (result);
}
