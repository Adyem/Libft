#include "class_fd_istream.hpp"
#include "../Template/move.hpp"

ft_fd_istream::ft_fd_istream(int fd) noexcept
    : _fd(fd)
    , _mutex()
{
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    return ;
}

int ft_fd_istream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
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

void ft_fd_istream::restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno) noexcept
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

void ft_fd_istream::set_fd(int fd) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_fd_istream::restore_errno(guard, entry_errno);
        return ;
    }
    this->_fd = fd;
    ft_fd_istream::restore_errno(guard, entry_errno);
    return ;
}

int ft_fd_istream::get_fd() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int descriptor;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_fd_istream::restore_errno(guard, entry_errno);
        return (-1);
    }
    descriptor = this->_fd;
    ft_fd_istream::restore_errno(guard, entry_errno);
    return (descriptor);
}

std::size_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int descriptor;
    ssize_t result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        ft_fd_istream::restore_errno(guard, entry_errno);
        return (0);
    }
    descriptor = this->_fd;
    ft_fd_istream::restore_errno(guard, entry_errno);
    result = su_read(descriptor, buffer, count);
    if (result < 0)
    {
        int read_error;

        read_error = ft_errno;
        if (read_error == ER_SUCCESS)
            read_error = FT_ERR_INVALID_HANDLE;
        this->set_error(read_error);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (static_cast<std::size_t>(result));
}
