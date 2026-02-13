#include "class_istream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

void ft_istream::abort_lifecycle_error(const char *method_name,
    const char *reason) noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_istream lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_istream::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == ft_istream::_state_initialized)
        return ;
    ft_istream::abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int ft_istream::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_istream::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

ft_istream::ft_istream() noexcept
    : _gcount(0), _is_valid(true), _mutex(ft_nullptr),
      _initialized_state(ft_istream::_state_uninitialized)
{
    return ;
}

ft_istream::~ft_istream() noexcept
{
    if (this->_initialized_state == ft_istream::_state_uninitialized)
    {
        ft_istream::abort_lifecycle_error("ft_istream::~ft_istream",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_istream::_state_initialized)
    {
        int destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return ;
    }
    return ;
}

int ft_istream::initialize() noexcept
{
    if (this->_initialized_state == ft_istream::_state_initialized)
    {
        ft_istream::abort_lifecycle_error("ft_istream::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_gcount = 0;
    this->_is_valid = true;
    this->_initialized_state = ft_istream::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_istream::destroy() noexcept
{
    int destroy_error;

    if (this->_initialized_state != ft_istream::_state_initialized)
    {
        ft_istream::abort_lifecycle_error("ft_istream::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_gcount = 0;
    this->_is_valid = true;
    destroy_error = FT_ERR_SUCCESS;
    if (this->_mutex != ft_nullptr)
    {
        destroy_error = this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_initialized_state = ft_istream::_state_destroyed;
    return (destroy_error);
}

int ft_istream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_istream::enable_thread_safety");
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

void ft_istream::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("ft_istream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return ;
    this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return ;
}

bool ft_istream::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("ft_istream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ssize_t ft_istream::read(char *buffer, std::size_t count) noexcept
{
    ssize_t bytes_read;
    int unlock_error;

    this->abort_if_not_initialized("ft_istream::read");
    if (buffer == ft_nullptr && count > 0)
    {
        this->_is_valid = false;
        this->_gcount = 0;
        return (-1);
    }
    if (this->lock_mutex() != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        this->_gcount = 0;
        return (-1);
    }
    bytes_read = this->do_read(buffer, count);
    if (bytes_read < 0)
    {
        this->_is_valid = false;
        this->_gcount = 0;
    }
    else
    {
        this->_is_valid = true;
        this->_gcount = static_cast<std::size_t>(bytes_read);
    }
    unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->_is_valid = false;
        this->_gcount = 0;
        return (-1);
    }
    return (bytes_read);
}

std::size_t ft_istream::gcount() const noexcept
{
    std::size_t count_value;

    this->abort_if_not_initialized("ft_istream::gcount");
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (0);
    count_value = this->_gcount;
    if (this->unlock_mutex() != FT_ERR_SUCCESS)
        return (0);
    return (count_value);
}

bool ft_istream::is_valid() const noexcept
{
    bool result;

    this->abort_if_not_initialized("ft_istream::is_valid");
    if (this->lock_mutex() != FT_ERR_SUCCESS)
        return (false);
    result = this->_is_valid;
    if (this->unlock_mutex() != FT_ERR_SUCCESS)
        return (false);
    return (result);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_istream::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
