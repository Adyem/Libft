#include "class_ofstream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <fcntl.h>
#include <new>

void ft_ofstream::abort_lifecycle_error(const char *method_name,
    const char *reason) noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_ofstream lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_ofstream::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_ofstream::_state_initialized)
        return ;
    ft_ofstream::abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int ft_ofstream::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_ofstream::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

ft_ofstream::ft_ofstream() noexcept
    : _file(), _mutex(ft_nullptr), _initialized_state(ft_ofstream::_state_uninitialized)
{
    return ;
}

ft_ofstream::~ft_ofstream() noexcept
{
    if (this->_initialized_state == ft_ofstream::_state_uninitialized)
    {
        ft_ofstream::abort_lifecycle_error("ft_ofstream::~ft_ofstream",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_ofstream::_state_initialized)
        (void)this->destroy();
    return ;
}

int ft_ofstream::initialize() noexcept
{
    if (this->_initialized_state == ft_ofstream::_state_initialized)
    {
        ft_ofstream::abort_lifecycle_error("ft_ofstream::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_ofstream::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_ofstream::destroy() noexcept
{
    int destroy_error;

    if (this->_initialized_state != ft_ofstream::_state_initialized)
    {
        ft_ofstream::abort_lifecycle_error("ft_ofstream::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_file.close();
    destroy_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
    {
        destroy_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_initialized_state = ft_ofstream::_state_destroyed;
    return (destroy_error);
}

int ft_ofstream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_ofstream::enable_thread_safety");
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

void ft_ofstream::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_ofstream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return ;
    this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return ;
}

bool ft_ofstream::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("ft_ofstream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_ofstream::open(const char *filename) noexcept
{
    int open_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_ofstream::open");
    if (filename == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    open_error = this->lock_mutex();
    if (open_error != FT_ERR_SUCCESS)
        return (open_error);
    open_error = this->_file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    unlock_error = this->unlock_mutex();
    if (open_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_HANDLE);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}

ssize_t ft_ofstream::write(const char *string) noexcept
{
    ssize_t bytes_written;
    int unlock_error;

    this->abort_if_not_initialized("ft_ofstream::write");
    if (string == ft_nullptr)
        return (-1);
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (-1);
    bytes_written = this->_file.write(string);
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (-1);
    return (bytes_written);
}

int ft_ofstream::close() noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_ofstream::close");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    this->_file.close();
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    return (FT_ERR_SUCCESS);
}
