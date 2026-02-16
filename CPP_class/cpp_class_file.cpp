#include "class_file.hpp"
#include "class_file_stream.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../PThread/pthread.hpp"
#include "class_nullptr.hpp"
#include <cerrno>
#include <cstdarg>
#include <new>
#include <unistd.h>

int ft_file::lock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int ft_file::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_file::enable_thread_safety(void) noexcept
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_file::disable_thread_safety(void) noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int destroy_error = this->_mutex->destroy();

    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_file::is_thread_safe(void) const noexcept
{
    return (this->_mutex != ft_nullptr);
}

ft_file::ft_file() noexcept
    : _fd(-1), _mutex(ft_nullptr)
{
    return ;
}

ft_file::~ft_file() noexcept
{
    int lock_error;
    int final_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->disable_thread_safety();
        return ;
    }
    final_error = FT_ERR_SUCCESS;
    if (this->_fd >= 0)
    {
        if (su_close(this->_fd) == -1)
            final_error = cmp_map_system_error_to_ft(errno);
        else
            this->_fd = -1;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    this->disable_thread_safety();
    return ;
}

void    ft_file::close() noexcept
{
    int lock_error;
    int final_error;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    final_error = FT_ERR_SUCCESS;
    if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else if (su_close(this->_fd) == -1)
        final_error = cmp_map_system_error_to_ft(errno);
    else
        this->_fd = -1;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    return ;
}

int ft_file::open(const char* filename, int flags, mode_t mode) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (1);
    final_error = FT_ERR_SUCCESS;
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    new_fd = su_open(filename, flags, mode);
    if (new_fd < 0)
        final_error = cmp_map_system_error_to_ft(errno);
    else
    {
        if (this->_fd >= 0)
        {
            if (su_close(this->_fd) == -1)
            {
                final_error = cmp_map_system_error_to_ft(errno);
                su_close(new_fd);
            }
        }
        if (final_error == FT_ERR_SUCCESS)
            this->_fd = new_fd;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error == FT_ERR_SUCCESS)
        return (0);
    return (1);
}

int ft_file::open(const char* filename, int flags) noexcept
{
    int lock_error;
    int final_error;
    int new_fd;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (1);
    final_error = FT_ERR_SUCCESS;
    new_fd = su_open(filename, flags);
    if (new_fd < 0)
        final_error = cmp_map_system_error_to_ft(errno);
    else
    {
        if (this->_fd >= 0)
        {
            if (su_close(this->_fd) == -1)
            {
                final_error = cmp_map_system_error_to_ft(errno);
                su_close(new_fd);
            }
        }
        if (final_error == FT_ERR_SUCCESS)
            this->_fd = new_fd;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error == FT_ERR_SUCCESS)
        return (0);
    return (1);
}

int ft_file::get_fd() const
{
    int lock_error;
    int final_error;
    int descriptor;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    descriptor = -1;
    if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
        descriptor = this->_fd;
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (descriptor);
}

ssize_t ft_file::read(char *buffer, int count) noexcept
{
    int lock_error;
    int final_error;
    ssize_t bytes_read;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    bytes_read = -1;
    if (buffer == ft_nullptr || count <= 0)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        bytes_read = su_read(this->_fd, buffer, static_cast<size_t>(count));
        if (bytes_read == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    result = -1;
    if (string == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = su_write(this->_fd, string, ft_strlen(string));
        if (result == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

ssize_t ft_file::write_buffer(const char *buffer, size_t length) noexcept
{
    int lock_error;
    int final_error;
    ssize_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
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
            final_error = cmp_map_system_error_to_ft(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (result);
}

int ft_file::seek(off_t offset, int whence) noexcept
{
    int lock_error;
    int final_error;
    off_t result;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    final_error = FT_ERR_SUCCESS;
    if (this->_fd < 0)
        final_error = FT_ERR_INVALID_HANDLE;
    else
    {
        result = ::lseek(this->_fd, offset, whence);
        if (result == -1)
            final_error = cmp_map_system_error_to_ft(errno);
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int ft_file::printf(const char *format, ...)
{
    int lock_error;
    int final_error;
    int printed_chars;

    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    final_error = FT_ERR_SUCCESS;
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
        if (printed_chars < 0)
            final_error = FT_ERR_IO;
    }
    int unlock_error = this->unlock_mutex();
    if (unlock_error != FT_ERR_SUCCESS && final_error == FT_ERR_SUCCESS)
        final_error = unlock_error;
    if (final_error != FT_ERR_SUCCESS && printed_chars >= 0)
        printed_chars = 0;
    return (printed_chars);
}

int ft_file::copy_to_with_buffer(const char *destination_path, size_t buffer_size) noexcept
{
    if (destination_path == ft_nullptr)
        return (-1);
    if (this->_fd < 0)
        return (-1);
    ft_file destination_file;
    int destination_flags;
    int copy_result;

    destination_flags = O_WRONLY | O_CREAT | O_TRUNC;
    if (destination_file.open(destination_path, destination_flags, 0644) != 0)
        return (-1);
    copy_result = ft_file_stream_copy(*this, destination_file, buffer_size);
    if (copy_result != 0)
        return (-1);
    return (0);
}

int ft_file::copy_to(const char *destination_path) noexcept
{
    return (this->copy_to_with_buffer(destination_path, ft_file_default_buffer_size()));
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex &ft_file::recursive_mutex(void) noexcept
{
    return (*this->_mutex);
}
#endif
