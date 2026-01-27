#include "class_file.hpp"
#include "class_file_stream.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "class_nullptr.hpp"
#include <cerrno>
#include <cstdarg>
#include <unistd.h>

namespace
{
    static int ft_file_lock_mutex(const pt_recursive_mutex &mutex)
    {
        int error;

        error = mutex.lock(THREAD_ID);
        ft_global_error_stack_pop_newest();
        return (error);
    }

    static int ft_file_unlock_mutex(const pt_recursive_mutex &mutex)
    {
        int error;

        error = mutex.unlock(THREAD_ID);
        ft_global_error_stack_pop_newest();
        return (error);
    }
}

void ft_file::sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int ft_file::lock_pair(const ft_file &first, const ft_file &second,
        const ft_file *&lower, const ft_file *&upper)
{
    const ft_file *ordered_first;
    const ft_file *ordered_second;

    if (&first == &second)
    {
        lower = &first;
        upper = &first;
        return (ft_file_lock_mutex(first._mutex));
    }
    ordered_first = &first;
    ordered_second = &second;
    if (ordered_first > ordered_second)
    {
        const ft_file *temporary = ordered_first;

        ordered_first = ordered_second;
        ordered_second = temporary;
    }
    lower = ordered_first;
    upper = ordered_second;
    while (true)
    {
        int lower_error = ft_file_lock_mutex(lower->_mutex);

        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = ft_file_lock_mutex(upper->_mutex);

        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_file_unlock_mutex(lower->_mutex);
            return (upper_error);
        }
        ft_file_unlock_mutex(lower->_mutex);
        ft_file::sleep_backoff();
    }
}

int ft_file::unlock_pair(const ft_file *lower, const ft_file *upper)
{
    int error;
    int final_error;

    final_error = FT_ERR_SUCCESSS;
    if (upper != ft_nullptr)
    {
        error = ft_file_unlock_mutex(upper->_mutex);
        if (error != FT_ERR_SUCCESSS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = ft_file_unlock_mutex(lower->_mutex);
        if (error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = error;
    }
    return (final_error);
}

ft_file::ft_file() noexcept
    : _fd(-1), _mutex(), _is_open(false)
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file::ft_file(const char* filename, int flags, mode_t mode) noexcept
    : _fd(-1), _mutex(), _is_open(false)
{
    int result;
    int new_fd;

    result = FT_ERR_SUCCESSS;
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_fd = su_open(filename, flags, mode);
    if (new_fd < 0)
        result = ft_map_system_error(errno);
    else
    {
        this->_fd = new_fd;
        this->_is_open = true;
    }
    this->record_operation_error(result);
    return ;
}

ft_file::ft_file(const char* filename, int flags) noexcept
    : _fd(-1), _mutex(), _is_open(false)
{
    int result;
    int new_fd;

    result = FT_ERR_SUCCESSS;
    new_fd = su_open(filename, flags);
    if (new_fd < 0)
        result = ft_map_system_error(errno);
    else
    {
        this->_fd = new_fd;
        this->_is_open = true;
    }
    this->record_operation_error(result);
    return ;
}

ft_file::ft_file(int fd) noexcept
    : _fd(fd), _mutex(), _is_open(fd >= 0)
{
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file::~ft_file() noexcept
{
    int lock_error;
    int final_error;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    final_error = FT_ERR_SUCCESSS;
    if (this->_is_open && this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
        {
            final_error = ft_map_system_error(errno);
        }
        else
        {
            this->_fd = -1;
            this->_is_open = false;
        }
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return ;
}

ft_file::ft_file(ft_file&& other) noexcept
    : _fd(-1), _mutex(), _is_open(false)
{
    int lock_error;
    int final_error;

    lock_error = ft_file_lock_mutex(other._mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    this->_fd = other._fd;
    this->_is_open = other._is_open;
    other._fd = -1;
    other._is_open = false;
    final_error = ft_file_unlock_mutex(other._mutex);
    if (final_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(final_error);
        other.record_operation_error(FT_ERR_SUCCESSS);
        return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    other.record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file& ft_file::operator=(ft_file&& other) noexcept
{
    if (this == &other)
        return (*this);
    const ft_file *lower;
    const ft_file *upper;
    int lock_error;

    lock_error = ft_file::lock_pair(*this, other, lower, upper);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (*this);
    }
    int final_error;

    final_error = FT_ERR_SUCCESSS;
    if (this->_is_open && this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
            final_error = ft_map_system_error(errno);
    }
    this->_fd = other._fd;
    this->_is_open = other._is_open;
    other._fd = -1;
    other._is_open = false;
    int unlock_error = ft_file::unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    other.record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

void    ft_file::close() noexcept
{
    int lock_error;
    int final_error;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return ;
    }
    final_error = FT_ERR_SUCCESSS;
    if (!this->_is_open || this->_fd < 0)
    {
        final_error = FT_ERR_INVALID_HANDLE;
    }
    else if (su_close(this->_fd) == -1)
    {
        final_error = ft_map_system_error(errno);
    }
    else
    {
        this->_fd = -1;
        this->_is_open = false;
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return ;
}

int ft_file::open(const char* filename, int flags, mode_t mode) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (1);
    }
    final_error = FT_ERR_SUCCESSS;
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_fd = su_open(filename, flags, mode);
    if (new_fd < 0)
        final_error = ft_map_system_error(errno);
    else
    {
        if (this->_is_open && this->_fd >= 0)
        {
            if (su_close(this->_fd) == -1)
            {
                final_error = ft_map_system_error(errno);
                su_close(new_fd);
            }
        }
        if (final_error == FT_ERR_SUCCESSS)
        {
            this->_fd = new_fd;
            this->_is_open = true;
        }
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (final_error == FT_ERR_SUCCESSS ? 0 : 1);
}

int ft_file::open(const char* filename, int flags) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (1);
    }
    final_error = FT_ERR_SUCCESSS;
    new_fd = su_open(filename, flags);
    if (new_fd < 0)
        final_error = ft_map_system_error(errno);
    else
    {
        if (this->_is_open && this->_fd >= 0)
        {
            if (su_close(this->_fd) == -1)
            {
                final_error = ft_map_system_error(errno);
                su_close(new_fd);
            }
        }
        if (final_error == FT_ERR_SUCCESSS)
        {
            this->_fd = new_fd;
            this->_is_open = true;
        }
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    return (final_error == FT_ERR_SUCCESSS ? 0 : 1);
}

int ft_file::get_fd() const
{
    int lock_error;
    int final_error;
    int descriptor;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    descriptor = -1;
    if (!this->_is_open || this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
        descriptor = this->_fd;
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (descriptor);
}

int ft_file::get_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

const char *ft_file::get_error_str() const noexcept
{
    const char *message;

    message = ft_strerror(this->get_error());
    if (message == ft_nullptr)
        message = "unknown error";
    return (message);
}

ssize_t ft_file::read(char *buffer, int count) noexcept
{
    int lock_error;
    int final_error;
    ssize_t bytes_read;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    bytes_read = -1;
    if (buffer == ft_nullptr || count <= 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (!this->_is_open || this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        bytes_read = su_read(this->_fd, buffer, static_cast<size_t>(count));
        if (bytes_read == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    result = -1;
    if (string == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (!this->_is_open || this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = su_write(this->_fd, string, ft_strlen(string));
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (result);
}

ssize_t ft_file::write_buffer(const char *buffer, size_t length) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    result = -1;
    if (buffer == ft_nullptr && length != 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (!this->_is_open || this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else if (length == 0)
        result = 0;
    else
    {
        result = su_write(this->_fd, buffer, length);
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(final_error);
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

int ft_file::seek(off_t offset, int whence) noexcept
{
    int lock_error;
    int final_error;
    off_t result;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    if (!this->_is_open || this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = ::lseek(this->_fd, offset, whence);
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

int ft_file::printf(const char *format, ...)
{
    int lock_error;
    int final_error;
    int printed_chars;

    lock_error = ft_file_lock_mutex(this->_mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(lock_error);
        return (0);
    }
    final_error = FT_ERR_SUCCESSS;
    printed_chars = 0;
    if (format == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (!this->_is_open || this->_fd == -1)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        va_list args;

        va_start(args, format);
        printed_chars = pf_printf_fd_v(this->_fd, format, args);
        va_end(args);
        int printf_error = ft_global_error_stack_pop_newest();

        if (printed_chars < 0)
        {
            if (printf_error != FT_ERR_SUCCESSS)
                final_error = printf_error;
            else
                final_error = FT_ERR_IO;
        }
    }
    int unlock_error = ft_file_unlock_mutex(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->record_operation_error(final_error);
    if (final_error != FT_ERR_SUCCESSS && printed_chars >= 0)
        printed_chars = 0;
    return (printed_chars);
}

int ft_file::copy_to_with_buffer(const char *destination_path, size_t buffer_size) noexcept
{
    if (destination_path == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->record_operation_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    ft_file destination_file;
    int destination_flags;
    int copy_result;

    destination_flags = O_WRONLY | O_CREAT | O_TRUNC;
#if defined(_WIN32) || defined(_WIN64)
    destination_flags |= O_BINARY;
#endif
    if (destination_file.open(destination_path, destination_flags, 0644) != 0)
    {
        this->record_operation_error(destination_file.get_error());
        return (-1);
    }
    copy_result = ft_file_stream_copy(*this, destination_file, buffer_size);
    if (copy_result != 0)
    {
        int source_error = this->get_error();

        if (source_error != FT_ERR_SUCCESSS)
            this->record_operation_error(source_error);
        else
        {
            int destination_error = destination_file.get_error();

            if (destination_error != FT_ERR_SUCCESSS)
                this->record_operation_error(destination_error);
            else
                this->record_operation_error(FT_ERR_IO);
        }
        return (-1);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_file::copy_to(const char *destination_path) noexcept
{
    return (this->copy_to_with_buffer(destination_path, ft_file_default_buffer_size()));
}

ft_file::operator int() const
{
    return (this->get_fd());
}

pt_recursive_mutex &ft_file::recursive_mutex() noexcept
{
    return (this->_mutex);
}

ft_operation_error_stack &ft_file::operation_error_stack() const noexcept
{
    return (this->_operation_errors);
}

void ft_file::record_operation_error(int error_code) const
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}
