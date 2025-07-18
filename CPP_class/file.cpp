#include "file.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf_internal.hpp"
#include "../Printf/printf.hpp"
#include <cerrno>
#include <cstdarg>
#include <unistd.h>

ft_file::ft_file() noexcept
{
    this->_fd = -1;
    this->_error_code = 0;
    return ;
}

ft_file::ft_file(const char* filename, int flags, mode_t mode) noexcept 
    : _fd(-1), _error_code(0)
{
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    this->_fd = ::open(filename, flags, mode);
    if (this->_fd < 0)
        this->set_error(errno + ERRNO_OFFSET);
    return ;
}

ft_file::ft_file(const char* filename, int flags) noexcept 
    : _fd(-1), _error_code(0)
{
    this->_fd = ::open(filename, flags);
    if (this->_fd < 0)
        this->set_error(errno + ERRNO_OFFSET);
    return ;
}

ft_file::ft_file(int fd) noexcept : _fd(fd), _error_code(0)
{
    return ;
}

ft_file::~ft_file() noexcept
{
    if (this->_fd >= 0)
    {
        if (::close(this->_fd) == -1)
            ft_errno = errno + ERRNO_OFFSET;
    }
    return ;
}

ft_file::ft_file(ft_file&& other) noexcept 
    : _fd(other._fd), _error_code(other._error_code)
{
    other._fd = -1;
    other._error_code = 0;
    return ;
}

ft_file& ft_file::operator=(ft_file&& other) noexcept
{
    if (this != &other)
    {
        if (this->_fd >= 0)
        {
            if (::close(this->_fd) == -1 && this->_error_code == 0)
                this->_error_code = errno;
        }
        this->_fd = other._fd;
        this->_error_code = other._error_code;
        other._fd = -1;
        other._error_code = 0;
    }
    return (*this);
}

void    ft_file::close() noexcept
{
    if (this->_fd >= 0)
    {
        ::close(this->_fd);
        this->_fd = -1;
    }
    return ;
}

int ft_file::open(const char* filename, int flags, mode_t mode) noexcept
{
    if (DEBUG == 1)
        pf_printf("Opening %s\n", filename);
    if (this->_fd != -1)
        this->close();
    this->_fd = ::open(filename, flags, mode);
    if (this->_fd < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    return (0);
}

int ft_file::open(const char* filename, int flags) noexcept
{
    if (this->_fd != -1)
        this->close();
    this->_fd = ::open(filename, flags);
    if (this->_fd < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    return (0);
}

void    ft_file::set_error(int error_code)
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_file::get_fd() const
{
    return (this->_fd);
}

int ft_file::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_file::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

ssize_t ft_file::read(char *buffer, int count) noexcept
{
    if (buffer == NULL || count <= 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    if (this->_fd < 0)
    {
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
    ssize_t bytes_read = ::read(this->_fd, buffer, count);
    if (bytes_read == -1)
        this->set_error(errno + ERRNO_OFFSET);
    return (bytes_read);
}

ssize_t ft_file::write(const char *string) noexcept
{
    if (string == nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    ssize_t result = ::write(this->_fd, string, ft_strlen(string));
    if (result == -1)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }

    return (result);
}

int ft_file::seek(off_t offset, int whence) noexcept
{
    off_t result = ::lseek(this->_fd, offset, whence);
    if (result == -1)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    return (0);
}

int ft_file::printf(const char *format, ...)
{
    va_list args;

    if (!format)
        return (0);
    if (this->_fd == -1)
    {
        this->set_error(FILE_INVALID_FD);
        return (0);
    }
    va_start(args, format);
    int printed_chars = pf_printf_fd_v(this->_fd, format, args);
    va_end(args);
    return (printed_chars);
}

ft_file::operator int() const
{
    return (this->_fd);
}
