#include "class_file.hpp"
#include "class_file_stream.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"
#include "class_nullptr.hpp"
#include <cerrno>
#include <cstdarg>
#include <unistd.h>

void ft_file::sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

int ft_file::lock_pair(const ft_file &first, const ft_file &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_file *ordered_first;
    const ft_file *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);
        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_file *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);
        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_file::sleep_backoff();
    }
}

ft_file::ft_file() noexcept
    : _fd(-1), _error_code(0), _mutex(), _is_open(false)
{
    return ;
}

ft_file::ft_file(const char* filename, int flags, mode_t mode) noexcept
    : _fd(-1), _error_code(0), _mutex(), _is_open(false)
{
    int new_fd;

    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_fd = su_open(filename, flags, mode);
    if (new_fd < 0)
    {
        this->set_error(ft_map_system_error(errno));
        return ;
    }
    this->_fd = new_fd;
    this->_is_open = true;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file::ft_file(const char* filename, int flags) noexcept
    : _fd(-1), _error_code(0), _mutex(), _is_open(false)
{
    int new_fd;

    new_fd = su_open(filename, flags);
    if (new_fd < 0)
    {
        this->set_error(ft_map_system_error(errno));
        return ;
    }
    this->_fd = new_fd;
    this->_is_open = true;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file::ft_file(int fd) noexcept
    : _fd(fd), _error_code(0), _mutex(), _is_open(fd >= 0)
{
    return ;
}

ft_file::~ft_file() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (this->_is_open && this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
            this->set_error(ft_map_system_error(errno));
        else
            this->set_error(FT_ERR_SUCCESSS);
        this->_is_open = false;
    }
    else
        this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file::ft_file(ft_file&& other) noexcept
    : _fd(-1), _error_code(0), _mutex(), _is_open(false)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->_error_code = other_guard.get_error();
        ft_errno = other_guard.get_error();
        return ;
    }
    this->_fd = other._fd;
    this->_error_code = FT_ERR_SUCCESSS;
    this->_is_open = other._is_open;
    other._fd = -1;
    other._error_code = FT_ERR_SUCCESSS;
    other._is_open = false;
    other.set_error(FT_ERR_SUCCESSS);
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_file& ft_file::operator=(ft_file&& other) noexcept
{
    if (this != &other)
    {
        ft_unique_lock<pt_mutex> this_guard;
        ft_unique_lock<pt_mutex> other_guard;
        int lock_error;

        lock_error = ft_file::lock_pair(*this, other, this_guard, other_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (*this);
        }
        int final_error;

        final_error = FT_ERR_SUCCESSS;
        if (this->_is_open && this->_fd >= 0)
        {
            if (su_close(this->_fd) == -1)
            {
                final_error = ft_map_system_error(errno);
            }
        }
        this->_fd = other._fd;
        this->_error_code = final_error;
        this->_is_open = other._is_open;
        other._fd = -1;
        other._error_code = FT_ERR_SUCCESSS;
        other._is_open = false;
        this->set_error(final_error);
        other.set_error(FT_ERR_SUCCESSS);
    }
    return (*this);
}

void    ft_file::close() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return ;
    }
    if (this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
        {
            this->set_error(ft_map_system_error(errno));
            return ;
        }
        this->_is_open = false;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_file::open(const char* filename, int flags, mode_t mode) noexcept
{
    int new_fd;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (1);
    }
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_fd = su_open(filename, flags, mode);
    if (new_fd < 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (1);
    }
    if (this->_is_open && this->_fd != -1)
    {
        if (su_close(this->_fd) == -1)
        {
            int close_error;

            close_error = ft_map_system_error(errno);
            su_close(new_fd);
            this->set_error(close_error);
            return (1);
        }
    }
    this->_fd = new_fd;
    this->_is_open = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_file::open(const char* filename, int flags) noexcept
{
    int new_fd;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (1);
    }
    new_fd = su_open(filename, flags);
    if (new_fd < 0)
    {
        this->set_error(ft_map_system_error(errno));
        return (1);
    }
    if (this->_is_open && this->_fd != -1)
    {
        if (su_close(this->_fd) == -1)
        {
            int close_error;

            close_error = ft_map_system_error(errno);
            su_close(new_fd);
            this->set_error(close_error);
            return (1);
        }
    }
    this->_fd = new_fd;
    this->_is_open = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

void    ft_file::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_file::get_fd() const
{
    int descriptor;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    descriptor = this->_fd;
    this->set_error(FT_ERR_SUCCESSS);
    return (descriptor);
}

int ft_file::get_error() const noexcept
{
    int error;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (guard.get_error());
    }
    error = this->_error_code;
    ft_errno = FT_ERR_SUCCESSS;
    return (error);
}

const char *ft_file::get_error_str() const noexcept
{
    const char *message;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (ft_strerror(guard.get_error()));
    }
    message = ft_strerror(this->_error_code);
    ft_errno = FT_ERR_SUCCESSS;
    return (message);
}

ssize_t ft_file::read(char *buffer, int count) noexcept
{
    ssize_t bytes_read;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (buffer == ft_nullptr || count <= 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    bytes_read = su_read(this->_fd, buffer, static_cast<size_t>(count));
    if (bytes_read == -1)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    ssize_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    result = su_write(this->_fd, string, ft_strlen(string));
    if (result == -1)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

ssize_t ft_file::write_buffer(const char *buffer, size_t length) noexcept
{
    ssize_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (buffer == ft_nullptr && length != 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    if (length == 0)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    result = su_write(this->_fd, buffer, length);
    if (result == -1)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

int ft_file::seek(off_t offset, int whence) noexcept
{
    off_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    result = ::lseek(this->_fd, offset, whence);
    if (result == -1)
    {
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

int ft_file::printf(const char *format, ...)
{
    int printed_chars;
    int error_code;
    va_list args;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (0);
    }
    if (format == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (!this->_is_open || this->_fd == -1)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    va_start(args, format);
    printed_chars = pf_printf_fd_v(this->_fd, format, args);
    va_end(args);
    error_code = ft_global_error_stack_pop_newest();
    if (printed_chars < 0)
    {
        if (error_code != FT_ERR_SUCCESSS)
            this->set_error(error_code);
        return (printed_chars);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (printed_chars);
}

int ft_file::copy_to(const char *destination_path) noexcept
{
    return (this->copy_to_with_buffer(destination_path, ft_file_default_buffer_size()));
}

int ft_file::copy_to_with_buffer(const char *destination_path, size_t buffer_size) noexcept
{
    int destination_flags;

    if (destination_path == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (!this->_is_open || this->_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (-1);
    }
    ft_file destination_file;
    destination_flags = O_WRONLY | O_CREAT | O_TRUNC;
#if defined(_WIN32) || defined(_WIN64)
    destination_flags |= O_BINARY;
#endif
    if (destination_file.open(destination_path, destination_flags, 0644) != 0)
    {
        this->set_error(destination_file.get_error());
        return (-1);
    }
    if (ft_file_stream_copy(*this, destination_file, buffer_size) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

ft_file::operator int() const
{
    return (this->get_fd());
}
