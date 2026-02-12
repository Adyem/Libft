#include "class_fd_istream.hpp"
#include "../Template/move.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include "class_nullptr.hpp"

int ft_fd_istream::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_lock_with_error(*this->_mutex));
}

int ft_fd_istream::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

int ft_fd_istream::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_fd_istream::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int ft_fd_istream::enable_thread_safety(void) noexcept
{
    return (this->prepare_thread_safety());
}

void ft_fd_istream::disable_thread_safety(void) noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_fd_istream::is_thread_safe_enabled(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
}

ft_fd_istream::ft_fd_istream(int fd) noexcept
    : _fd(fd)
    , _mutex(ft_nullptr)
{
    this->enable_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_fd_istream::ft_fd_istream(const ft_fd_istream &other) noexcept
    : ft_istream(other)
    , _fd(0)
    , _mutex(ft_nullptr)
{
    int lock_error;

    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other.unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_fd_istream::ft_fd_istream(ft_fd_istream &&other) noexcept
    : ft_istream(ft_move(other))
    , _fd(0)
    , _mutex(ft_nullptr)
{
    int lock_error;

    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = other._fd;
    other._fd = -1;
    other.unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    this->disable_thread_safety();
    return ;
}

ft_fd_istream &ft_fd_istream::operator=(const ft_fd_istream &other) noexcept
{
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    ft_istream::operator=(other);
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    this->unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

ft_fd_istream &ft_fd_istream::operator=(ft_fd_istream &&other) noexcept
{
    int lock_error;
    int descriptor;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    ft_istream::operator=(ft_move(other));
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    descriptor = other.get_fd();
    this->_fd = descriptor;
    other.set_fd(-1);
    this->unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

void ft_fd_istream::set_fd(int fd) noexcept
{
    int lock_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_fd = fd;
    this->unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

int ft_fd_istream::get_fd() const noexcept
{
    int lock_error;
    int descriptor;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    descriptor = this->_fd;
    this->unlock_mutex();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (descriptor);
}

std::size_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    int lock_error;
    int descriptor;
    ssize_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    descriptor = this->_fd;
    this->unlock_mutex();
    result = su_read(descriptor, buffer, count);
    int read_error = ft_global_error_stack_drop_last_error();
    if (result < 0)
    {
        if (read_error == FT_ERR_SUCCESS)
            read_error = FT_ERR_INVALID_HANDLE;
        ft_global_error_stack_push(read_error);
        return (0);
    }
    if (read_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(read_error);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (static_cast<std::size_t>(result));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_fd_istream::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
