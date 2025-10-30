#include "class_ofstream.hpp"
#include "class_nullptr.hpp"
#include "../Template/move.hpp"
#include <fcntl.h>

ft_ofstream::ft_ofstream() noexcept
    : _file(), _error_code(ER_SUCCESS), _mutex()
{
    return ;
}

ft_ofstream::~ft_ofstream() noexcept
{
    return ;
}

void ft_ofstream::set_error_unlocked(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void ft_ofstream::set_error(int error_code) const noexcept
{
    this->set_error_unlocked(error_code);
    return ;
}

int ft_ofstream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
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

void ft_ofstream::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
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

int ft_ofstream::open(const char *filename) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int open_result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_ofstream::restore_errno(guard, entry_errno);
        return (1);
    }
    if (filename == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        ft_ofstream::restore_errno(guard, entry_errno);
        return (1);
    }
    open_result = this->_file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (open_result != 0)
    {
        this->set_error_unlocked(this->_file.get_error());
        ft_ofstream::restore_errno(guard, entry_errno);
        return (1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft_ofstream::restore_errno(guard, entry_errno);
    return (0);
}

ssize_t ft_ofstream::write(const char *string) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    ssize_t result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_ofstream::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (string == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        ft_ofstream::restore_errno(guard, entry_errno);
        return (-1);
    }
    result = this->_file.write(string);
    if (result < 0)
    {
        this->set_error_unlocked(this->_file.get_error());
        ft_ofstream::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft_ofstream::restore_errno(guard, entry_errno);
    return (result);
}

void ft_ofstream::close() noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int previous_fd;
    int current_fd;
    int file_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_ofstream::restore_errno(guard, entry_errno);
        return ;
    }
    previous_fd = this->_file.get_fd();
    this->_file.close();
    current_fd = this->_file.get_fd();
    file_error = this->_file.get_error();
    if (previous_fd >= 0 && current_fd == previous_fd && file_error != ER_SUCCESS)
    {
        this->set_error_unlocked(file_error);
        ft_ofstream::restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft_ofstream::restore_errno(guard, entry_errno);
    return ;
}

int ft_ofstream::get_error() const noexcept
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
        ft_ofstream::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_value = this->_error_code;
    ft_ofstream::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_ofstream::get_error_str() const noexcept
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
        ft_ofstream::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_string = ft_strerror(this->_error_code);
    ft_ofstream::restore_errno(guard, entry_errno);
    return (error_string);
}
