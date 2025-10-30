#include "class_istream.hpp"
#include "class_nullptr.hpp"
#include "../Template/move.hpp"

ft_istream::ft_istream() noexcept
    : _gcount(0)
    , _bad(false)
    , _error_code(ER_SUCCESS)
    , _mutex()
{
    return ;
}

ft_istream::~ft_istream() noexcept
{
    return ;
}

void ft_istream::set_error_unlocked(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_istream::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_istream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
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

void ft_istream::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

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
    ft_errno = entry_errno;
    return ;
}

void ft_istream::read(char *buffer, std::size_t count)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    std::size_t readed;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        this->_bad = true;
        this->_gcount = 0;
        ft_istream::restore_errno(guard, entry_errno);
        return ;
    }
    this->_error_code = ER_SUCCESS;
    this->_gcount = 0;
    this->_bad = false;
    if (buffer == ft_nullptr && count > 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        this->_bad = true;
        ft_istream::restore_errno(guard, entry_errno);
        return ;
    }
    readed = this->do_read(buffer, count);
    this->_gcount = readed;
    if (this->_error_code != ER_SUCCESS)
        this->_bad = true;
    ft_istream::restore_errno(guard, entry_errno);
    return ;
}

std::size_t ft_istream::gcount() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    std::size_t count_value;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (0);
    }
    count_value = this->_gcount;
    ft_istream::restore_errno(guard, entry_errno);
    return (count_value);
}

bool ft_istream::bad() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    bool is_bad_result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (true);
    }
    is_bad_result = this->_bad;
    ft_istream::restore_errno(guard, entry_errno);
    return (is_bad_result);
}

int ft_istream::get_error() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int error_code;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_istream::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_code = this->_error_code;
    ft_istream::restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_istream::get_error_str() const noexcept
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
        ft_istream::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    ft_istream::restore_errno(guard, entry_errno);
    return (error_string);
}
