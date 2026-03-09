#include "class_fd_istream.hpp"
#include "class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

int32_t ft_fd_istream::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_fd_istream::enable_thread_safety");
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

int32_t ft_fd_istream::disable_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_fd_istream::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_fd_istream::is_thread_safe(void) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_fd_istream::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ft_fd_istream::ft_fd_istream() noexcept
    : ft_istream(), _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_fd_istream::ft_fd_istream(int32_t file_descriptor) noexcept
    : ft_istream(), _file_descriptor(file_descriptor), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_fd_istream::ft_fd_istream(const ft_fd_istream &other) noexcept
    : ft_istream(), _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_fd_istream::ft_fd_istream copy source",
            "called with uninitialised source object");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_file_descriptor = other._file_descriptor;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (other._mutex != ft_nullptr && this->enable_thread_safety() != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_fd_istream::ft_fd_istream(ft_fd_istream &&other) noexcept
    : ft_istream(), _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_fd_istream::ft_fd_istream move source",
            "called with uninitialised source object");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_file_descriptor = other._file_descriptor;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        (void)other.disable_thread_safety();
    }
    other._file_descriptor = -1;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    (void)this->destroy();
    return ;
}

int32_t ft_fd_istream::move(ft_fd_istream &other) noexcept
{
    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_fd_istream::move",
            "called with uninitialised source object");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        int32_t destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_file_descriptor = -1;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    uint32_t initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_file_descriptor = other._file_descriptor;
    other._file_descriptor = -1;
    if (other._mutex != ft_nullptr)
    {
        int32_t thread_safety_error = this->enable_thread_safety();
        if (thread_safety_error != FT_ERR_SUCCESS)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (thread_safety_error);
        }
        (void)other.disable_thread_safety();
    }
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

void ft_fd_istream::set_file_descriptor(int32_t file_descriptor) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_fd_istream::set_file_descriptor");
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_file_descriptor = file_descriptor;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_fd_istream::get_file_descriptor() const noexcept
{
    int32_t file_descriptor;

    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_fd_istream::get_file_descriptor");
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (-1);
    file_descriptor = this->_file_descriptor;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (file_descriptor);
}

ssize_t ft_fd_istream::do_read(char *buffer, ft_size_t count)
{
    int32_t file_descriptor;
    ssize_t result;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_fd_istream::do_read");
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (-1);
    file_descriptor = this->_file_descriptor;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    result = su_read(file_descriptor, buffer, count);
    if (result < 0)
        return (-1);
    return (result);
}
