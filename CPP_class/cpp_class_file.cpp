#include "class_file.hpp"
#include "class_file_stream.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "class_nullptr.hpp"
#include <cerrno>
#include <cstdarg>
#include <unistd.h>

void ft_file::sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int ft_file::lock_mutex(void) const noexcept
{
    return (pt_recursive_mutex_lock_if_valid(this->_mutex));
}

int ft_file::unlock_mutex(void) const noexcept
{
    return (pt_recursive_mutex_unlock_if_valid(this->_mutex));
}

int ft_file::prepare_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(mutex_error);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_file::teardown_thread_safety(void) noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

int ft_file::enable_thread_safety(void) noexcept
{
    return (this->prepare_thread_safety());
}

void ft_file::disable_thread_safety(void) noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool ft_file::is_thread_safe_enabled(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
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
        return (first.lock_mutex());
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
        int lower_error = lower->lock_mutex();

        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = upper->lock_mutex();

        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
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
        error = upper->unlock_mutex();
        if (error != FT_ERR_SUCCESSS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = lower->unlock_mutex();
        if (error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = error;
    }
    return (final_error);
}

ft_file::ft_file() noexcept
    : _fd(-1), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_file::ft_file(const char* filename, int flags, mode_t mode) noexcept
    : _fd(-1), _mutex(ft_nullptr)
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
    }
    ft_global_error_stack_push(result);
    return ;
}

ft_file::ft_file(const char* filename, int flags) noexcept
    : _fd(-1), _mutex(ft_nullptr)
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
    }
    ft_global_error_stack_push(result);
    return ;
}

ft_file::ft_file(int fd) noexcept
    : _fd(fd), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_file::~ft_file() noexcept
{
    int lock_error;
    int final_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        this->disable_thread_safety();
        return ;
    }
    final_error = FT_ERR_SUCCESSS;
    if (this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
            final_error = ft_map_system_error(errno);
        else
            this->_fd = -1;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    this->disable_thread_safety();
    ft_global_error_stack_push(final_error);
    return ;
}

ft_file::ft_file(ft_file&& other) noexcept
    : _fd(-1), _mutex(ft_nullptr)
{
    int lock_error;
    int final_error;

    lock_error = other.lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
        this->_fd = other._fd;
        other._fd = -1;
        final_error = other.unlock_mutex();
    if (final_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(final_error);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    int final_error;

    final_error = FT_ERR_SUCCESSS;
    if (this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
            final_error = ft_map_system_error(errno);
    }
    this->_fd = other._fd;
    other._fd = -1;
    int unlock_error = this->unlock_pair(lower, upper);
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

void    ft_file::close() noexcept
{
    int lock_error;
    int final_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    final_error = FT_ERR_SUCCESSS;
    if (this->_fd < 0)
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
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    return ;
}

int ft_file::open(const char* filename, int flags, mode_t mode) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
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
        if (this->_fd >= 0)
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
        }
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error == FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

int ft_file::open(const char* filename, int flags) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (1);
    }
    final_error = FT_ERR_SUCCESSS;
    new_fd = su_open(filename, flags);
    if (new_fd < 0)
        final_error = ft_map_system_error(errno);
    else
    {
        if (this->_fd >= 0)
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
        }
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error == FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

int ft_file::get_fd() const
{
    int lock_error;
    int final_error;
    int descriptor;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    descriptor = -1;
    if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
        descriptor = this->_fd;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (descriptor);
}

int ft_file::get_error() const noexcept
{
    return (ft_global_error_stack_peek_last_error());
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

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    bytes_read = -1;
    if (buffer == ft_nullptr || count <= 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        bytes_read = su_read(this->_fd, buffer, static_cast<size_t>(count));
        if (bytes_read == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    result = -1;
    if (string == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = su_write(this->_fd, string, ft_strlen(string));
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (result);
}

ssize_t ft_file::write_buffer(const char *buffer, size_t length) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    result = -1;
    if (buffer == ft_nullptr && length != 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else if (length == 0)
        result = 0;
    else
    {
        result = su_write(this->_fd, buffer, length);
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(final_error);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

int ft_file::seek(off_t offset, int whence) noexcept
{
    int lock_error;
    int final_error;
    off_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (-1);
    }
    final_error = FT_ERR_SUCCESSS;
    if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = ::lseek(this->_fd, offset, whence);
        if (result == -1)
            final_error = ft_map_system_error(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

int ft_file::printf(const char *format, ...)
{
    int lock_error;
    int final_error;
    int printed_chars;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    final_error = FT_ERR_SUCCESSS;
    printed_chars = 0;
    if (format == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        va_list args;

        va_start(args, format);
        printed_chars = pf_printf_fd_v(this->_fd, format, args);
        va_end(args);
        int printf_error = ft_global_error_stack_drop_last_error();

        if (printed_chars < 0)
        {
            if (printf_error != FT_ERR_SUCCESSS)
                final_error = printf_error;
            else
                final_error = FT_ERR_IO;
        }
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
        final_error = unlock_error;
    ft_global_error_stack_push(final_error);
    if (final_error != FT_ERR_SUCCESSS && printed_chars >= 0)
        printed_chars = 0;
    return (printed_chars);
}

int ft_file::copy_to_with_buffer(const char *destination_path, size_t buffer_size) noexcept
{
    if (destination_path == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (this->_fd < 0)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_HANDLE);
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
        ft_global_error_stack_push(destination_file.get_error());
        return (-1);
    }
    copy_result = ft_file_stream_copy(*this, destination_file, buffer_size);
    if (copy_result != 0)
    {
        int source_error = this->get_error();

        if (source_error != FT_ERR_SUCCESSS)
            ft_global_error_stack_push(source_error);
        else
        {
            int destination_error = destination_file.get_error();

            if (destination_error != FT_ERR_SUCCESSS)
                ft_global_error_stack_push(destination_error);
            else
                ft_global_error_stack_push(FT_ERR_IO);
        }
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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

#ifdef LIBFT_TEST_BUILD
ft_recursive_mutex &ft_file::recursive_mutex() noexcept
{
    return (this->_mutex);
}
#endif
