#include "class_fd_istream.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"

ft_fd_istream::ft_fd_istream(int fd) noexcept
    : _fd(fd)
    , _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::ft_fd_istream(const ft_fd_istream &other) noexcept
    : ft_istream(other)
    , _fd(0)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other_guard.unlock();
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::ft_fd_istream(ft_fd_istream &&other) noexcept
    : ft_istream(ft_move(other))
    , _fd(0)
    , _mutex()
{
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other._fd = -1;
    other_guard.unlock();
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_fd_istream::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
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

ft_fd_istream &ft_fd_istream::operator=(const ft_fd_istream &other) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    ft_istream::operator=(other);
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (*this);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_fd_istream &ft_fd_istream::operator=(ft_fd_istream &&other) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    ft_istream::operator=(ft_move(other));
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    other.set_fd(-1);
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (*this);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

void ft_fd_istream::set_fd(int fd) noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_fd = fd;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_fd_istream::get_fd() const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int descriptor;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (-1);
    }
    descriptor = this->_fd;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (descriptor);
}

std::size_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int descriptor;
    ssize_t result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (0);
    }
    descriptor = this->_fd;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    result = su_read(descriptor, buffer, count);
    int read_error;

    read_error = ft_global_error_stack_pop_newest();
    if (result < 0)
    {
        if (read_error == FT_ERR_SUCCESSS)
            read_error = FT_ERR_INVALID_HANDLE;
        this->set_error(read_error);
        return (0);
    }
    if (read_error != FT_ERR_SUCCESSS)
    {
        this->set_error(read_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (static_cast<std::size_t>(result));
}
