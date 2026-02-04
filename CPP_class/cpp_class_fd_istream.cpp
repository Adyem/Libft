#include "class_fd_istream.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"

static int ft_fd_istream_capture_mutex_error() noexcept
{
    int error_value;

    error_value = ft_global_error_stack_last_error();
    ft_global_error_stack_pop_newest();
    if (error_value != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(error_value);
    return (error_value);
}

ft_fd_istream::ft_fd_istream(int fd) noexcept
    : _fd(fd)
    , _mutex()
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::ft_fd_istream(const ft_fd_istream &other) noexcept
    : ft_istream(other)
    , _fd(0)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other_guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::ft_fd_istream(ft_fd_istream &&other) noexcept
    : ft_istream(ft_move(other))
    , _fd(0)
    , _mutex()
{
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other._fd = -1;
    other_guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_fd_istream::lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
{
    ft_unique_lock<pt_recursive_mutex> local_guard(this->_mutex);

    int guard_error = ft_fd_istream_capture_mutex_error();
    if (guard_error != FT_ERR_SUCCESSS)
    {
        guard = ft_unique_lock<pt_recursive_mutex>();
        ft_global_error_stack_push(guard_error);
        return (guard_error);
    }
    guard = ft_move(local_guard);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

ft_fd_istream &ft_fd_istream::operator=(const ft_fd_istream &other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    ft_istream::operator=(other);
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

ft_fd_istream &ft_fd_istream::operator=(ft_fd_istream &&other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    ft_istream::operator=(ft_move(other));
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    other.set_fd(-1);
    guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

void ft_fd_istream::set_fd(int fd) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = fd;
    guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_fd_istream::get_fd() const noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int descriptor;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    descriptor = this->_fd;
    guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return (-1);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (descriptor);
}

std::size_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    ft_unique_lock<pt_recursive_mutex> guard;
    int lock_error;
    int descriptor;
    ssize_t result;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    descriptor = this->_fd;
    guard.unlock();
    {
        int guard_error = ft_fd_istream_capture_mutex_error();
        if (guard_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(guard_error);
            return (0);
        }
    }
    result = su_read(descriptor, buffer, count);
    int read_error;

    read_error = ft_global_error_stack_pop_newest();
    if (result < 0)
    {
        if (read_error == FT_ERR_SUCCESSS)
            read_error = FT_ERR_INVALID_HANDLE;
        ft_global_error_stack_push(read_error);
        return (0);
    }
    if (read_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(read_error);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (static_cast<std::size_t>(result));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_fd_istream::get_mutex_for_validation() const noexcept
{
    return (&(this->_mutex));
}
#endif
