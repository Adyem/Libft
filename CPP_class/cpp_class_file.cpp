#include "class_file.hpp"
#include "class_file_stream.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "class_nullptr.hpp"
#include <cerrno>
#include <cstdarg>
#include <new>
#include <unistd.h>

int32_t ft_file::enable_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_file::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int32_t mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file::disable_thread_safety(void) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_file::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = this->_mutex->destroy();

    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_file::is_thread_safe(void) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_file::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

ft_file::ft_file() noexcept
    : _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_file::ft_file(const ft_file& other) noexcept
    : _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_file::ft_file copy source",
            "called with uninitialised source object");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._file_descriptor >= 0)
    {
        this->_file_descriptor = dup(other._file_descriptor);
        if (this->_file_descriptor < 0)
        {
            (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
    }
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            if (this->_file_descriptor >= 0)
                (void)su_close(this->_file_descriptor);
            this->_file_descriptor = -1;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
    }
    return ;
}

ft_file::ft_file(ft_file&& other) noexcept
    : _file_descriptor(-1), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_file::ft_file move source",
            "called with uninitialised source object");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_file_descriptor = other._file_descriptor;
    other._file_descriptor = -1;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            if (this->_file_descriptor >= 0)
                (void)su_close(this->_file_descriptor);
            this->_file_descriptor = -1;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return ;
        }
        (void)other.disable_thread_safety();
    }
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

uint32_t ft_file::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_file::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_file_descriptor = -1;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file::destroy() noexcept
{
    int32_t thread_safety_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    thread_safety_error = this->disable_thread_safety();
    if (this->_file_descriptor >= 0)
    {
        if (su_close(this->_file_descriptor) == -1 && thread_safety_error == FT_ERR_SUCCESS)
            thread_safety_error = cmp_map_system_error_to_ft(errno);
        this->_file_descriptor = -1;
    }
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (thread_safety_error);
}

int32_t ft_file::move(ft_file &other) noexcept
{
    int32_t lock_error;
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_file::move",
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
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    lock_error = pt_recursive_mutex_lock_if_not_null(other._mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (lock_error);
    }
    this->_file_descriptor = other._file_descriptor;
    other._file_descriptor = -1;
    (void)pt_recursive_mutex_unlock_if_not_null(other._mutex);
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

ft_file::~ft_file() noexcept
{
    (void)this->destroy();
    return ;
}

void    ft_file::close() noexcept
{
    int32_t lock_error;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_file_descriptor >= 0 && su_close(this->_file_descriptor) == -1)
        (void)cmp_map_system_error_to_ft(errno);
    else if (this->_file_descriptor >= 0)
        this->_file_descriptor = -1;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_file::open(const char* filename, int32_t flags, mode_t mode) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    int32_t new_file_descriptor;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    final_error = FT_ERR_SUCCESS;
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_file_descriptor = su_open(filename, flags, mode);
    if (new_file_descriptor < 0)
        final_error = cmp_map_system_error_to_ft(errno);
    else
    {
        if (this->_file_descriptor >= 0)
        {
            if (su_close(this->_file_descriptor) == -1)
            {
                final_error = cmp_map_system_error_to_ft(errno);
                su_close(new_file_descriptor);
            }
        }
        if (final_error == FT_ERR_SUCCESS)
            this->_file_descriptor = new_file_descriptor;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    return (final_error);
}

int32_t ft_file::open(const char* filename, int32_t flags) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    int32_t new_file_descriptor;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    final_error = FT_ERR_SUCCESS;
    new_file_descriptor = su_open(filename, flags);
    if (new_file_descriptor < 0)
        final_error = cmp_map_system_error_to_ft(errno);
    else
    {
        if (this->_file_descriptor >= 0)
        {
            if (su_close(this->_file_descriptor) == -1)
            {
                final_error = cmp_map_system_error_to_ft(errno);
                su_close(new_file_descriptor);
            }
        }
        if (final_error == FT_ERR_SUCCESS)
            this->_file_descriptor = new_file_descriptor;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error == FT_ERR_SUCCESS)
        return (FT_ERR_SUCCESS);
    return (final_error);
}

int32_t ft_file::get_file_descriptor() const
{
    int32_t lock_error;
    int32_t final_error;
    int32_t file_descriptor;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    file_descriptor = -1;
    if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
        file_descriptor = this->_file_descriptor;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (file_descriptor);
}

ssize_t ft_file::read(char *buffer, int32_t count) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    ssize_t bytes_read;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    bytes_read = -1;
    if (buffer == ft_nullptr || count <= 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        bytes_read = su_read(this->_file_descriptor, buffer, static_cast<ft_size_t>(count));
        if (bytes_read == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    ssize_t result;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    result = -1;
    if (string == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = su_write(this->_file_descriptor, string, ft_strlen(string));
        if (result == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

ssize_t ft_file::write_buffer(const char *buffer, ft_size_t length) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    ssize_t result;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    result = -1;
    if (buffer == ft_nullptr && length != 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else if (length == 0)
        result = 0;
    else
    {
        result = su_write(this->_file_descriptor, buffer, length);
        if (result == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int32_t ft_file::seek(off_t offset, int32_t whence) noexcept
{
    int32_t lock_error;
    int32_t final_error;
    off_t result;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = ::lseek(this->_file_descriptor, offset, whence);
        if (result == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS)
        return (final_error);
    return (FT_ERR_SUCCESS);
}

int32_t ft_file::printf(const char *format, ...)
{
    int32_t lock_error;
    int32_t final_error;
    int32_t printed_chars;

    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    final_error = FT_ERR_SUCCESS;
    printed_chars = 0;
    if (format == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_file_descriptor < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        va_list args;

        va_start(args, format);
        printed_chars = pf_printf_fd_v(this->_file_descriptor, format, args);
        va_end(args);
        if (printed_chars < 0)
            final_error = FT_ERR_IO;
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (final_error != FT_ERR_SUCCESS && printed_chars >= 0)
        printed_chars = 0;
    return (printed_chars);
}

int32_t ft_file::copy_to_with_buffer(const char *destination_path, ft_size_t buffer_size) noexcept
{
    if (destination_path == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (this->_file_descriptor < 0)
        return (FT_ERR_INVALID_HANDLE);
    ft_file destination_file;
    int32_t destination_flags;
    int32_t copy_result;
    uint32_t destination_initialize_error;

    destination_initialize_error = destination_file.initialize();
    if (destination_initialize_error != FT_ERR_SUCCESS)
        return (destination_initialize_error);
    destination_flags = O_WRONLY | O_CREAT | O_TRUNC;
    if (destination_file.open(destination_path, destination_flags, 0644) != FT_ERR_SUCCESS)
        return (FT_ERR_IO);
    copy_result = ft_file_stream_copy(*this, destination_file, buffer_size);
    if (copy_result != FT_ERR_SUCCESS)
        return (copy_result);
    return (FT_ERR_SUCCESS);
}

int32_t ft_file::copy_to(const char *destination_path) noexcept
{
    return (this->copy_to_with_buffer(destination_path, ft_file_default_buffer_size()));
}
