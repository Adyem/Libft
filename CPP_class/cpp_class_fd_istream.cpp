#include "class_fd_istream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

void ft_fd_istream::abort_lifecycle_error(const char *method_name,
    const char *reason) noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_fd_istream lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_fd_istream::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_fd_istream::_state_initialized)
        return ;
    ft_fd_istream::abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int ft_fd_istream::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_fd_istream::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_fd_istream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_fd_istream::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_fd_istream::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_fd_istream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_fd_istream::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("ft_fd_istream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ft_fd_istream::ft_fd_istream(int fd) noexcept
    : ft_istream(), _fd(fd), _mutex(ft_nullptr),
      _initialized_state(ft_fd_istream::_state_uninitialized)
{
    int initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_fd_istream::_state_destroyed;
        return ;
    }
    this->_initialized_state = ft_fd_istream::_state_initialized;
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    if (this->_initialized_state == ft_fd_istream::_state_uninitialized)
    {
        ft_fd_istream::abort_lifecycle_error("ft_fd_istream::~ft_fd_istream",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_fd_istream::_state_initialized)
    {
        if (this->_mutex != ft_nullptr)
            this->disable_thread_safety();
        this->_initialized_state = ft_fd_istream::_state_destroyed;
    }
    return ;
}

void ft_fd_istream::set_fd(int fd) noexcept
{
    this->abort_if_not_initialized("ft_fd_istream::set_fd");
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return ;
    this->_fd = fd;
    this->unlock_mutex();
    return ;
}

int ft_fd_istream::get_fd() const noexcept
{
    int descriptor;

    this->abort_if_not_initialized("ft_fd_istream::get_fd");
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (-1);
    descriptor = this->_fd;
    if (this->unlock_mutex() != FT_ERR_SUCCESS)
        return (-1);
    return (descriptor);
}

ssize_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    int descriptor;
    int unlock_error;
    ssize_t result;

    this->abort_if_not_initialized("ft_fd_istream::do_read");
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (-1);
    descriptor = this->_fd;
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    result = su_read(descriptor, buffer, count);
    if (result < 0)
        return (-1);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_fd_istream::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
