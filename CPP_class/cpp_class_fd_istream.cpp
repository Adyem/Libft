#include "class_fd_istream.hpp"

ft_fd_istream::ft_fd_istream(int fd) noexcept
: _fd(fd)
{
    return ;
}

ft_fd_istream::~ft_fd_istream() noexcept
{
    return ;
}

std::size_t ft_fd_istream::do_read(char *buffer, std::size_t count)
{
    ssize_t result;

    result = su_read(this->_fd, buffer, count);
    if (result < 0)
    {
        int read_error;

        read_error = ft_errno;
        if (read_error == ER_SUCCESS)
            read_error = FT_ERR_INVALID_HANDLE;
        this->set_error(read_error);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (static_cast<std::size_t>(result));
}
