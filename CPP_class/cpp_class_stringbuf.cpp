#include "class_stringbuf.hpp"
#include "class_nullptr.hpp"
#include "../Template/move.hpp"

ft_stringbuf::ft_stringbuf(const ft_string &string) noexcept
    : _storage(string), _position(0), _error_code(ER_SUCCESS), _mutex()
{
    if (this->_storage.get_error() != ER_SUCCESS)
        this->set_error(this->_storage.get_error());
    return ;
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
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

void ft_stringbuf::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

    (void)entry_errno;
    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != ER_SUCCESS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = ER_SUCCESS;
    return ;
}

std::size_t ft_stringbuf::read(char *buffer, std::size_t count)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    std::size_t index;
    bool failure_occurred;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_stringbuf::restore_errno(guard, entry_errno);
        return (0);
    }
    if (buffer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        ft_stringbuf::restore_errno(guard, entry_errno);
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
        this->set_error_unlocked(ER_SUCCESS);
    ft_stringbuf::restore_errno(guard, entry_errno);
    return (index);
}

bool ft_stringbuf::is_bad() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    bool result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_stringbuf::restore_errno(guard, entry_errno);
        return (true);
    }
    result = (this->_error_code != ER_SUCCESS);
    ft_stringbuf::restore_errno(guard, entry_errno);
    return (result);
}

int ft_stringbuf::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_stringbuf::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_value = this->_error_code;
    ft_stringbuf::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_stringbuf::get_error_str() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    const char *error_string;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_stringbuf::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    ft_stringbuf::restore_errno(guard, entry_errno);
    return (error_string);
}

ft_string ft_stringbuf::str() const
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    const char *start;
    ft_string result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_stringbuf::restore_errno(guard, entry_errno);
        return (ft_string(lock_error));
    }
    start = this->_storage.c_str();
    result = ft_string(start + this->_position);
    if (result.get_error() != ER_SUCCESS)
        this->set_error_unlocked(result.get_error());
    else
        this->set_error_unlocked(ER_SUCCESS);
    ft_stringbuf::restore_errno(guard, entry_errno);
    return (result);
}
