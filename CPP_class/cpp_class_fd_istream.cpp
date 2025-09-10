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
        this->set_error(FILE_INVALID_FD);
        return (0);
    }
    return (static_cast<std::size_t>(result));
}
