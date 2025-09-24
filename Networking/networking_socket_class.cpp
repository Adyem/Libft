#include "socket_class.hpp"
#include "networking.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Template/move.hpp"
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <arpa/inet.h>
# include <sys/types.h>
# include <unistd.h>
# include <sys/socket.h>
#endif

ft_socket::ft_socket() : _socket_fd(-1), _error_code(ER_SUCCESS)
{
    ft_bzero(&this->_address, sizeof(this->_address));
    this->set_error(ER_SUCCESS);
    return ;
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags, int fd)
{
    size_t index = 0;
    while (index < this->_connected.size())
    {
        if (this->_connected[index].get_fd() == fd)
        {
            ssize_t bytes_sent = this->_connected[index].send_data(data, size, flags);
            if (bytes_sent < 0)
                this->set_error(errno + ERRNO_OFFSET);
            else
                this->set_error(ER_SUCCESS);
            return (bytes_sent);
        }
        index++;
    }
    this->set_error(FT_EINVAL);
    return (-1);
}

int ft_socket::get_fd() const
{
        return (this->_socket_fd);
}

const struct sockaddr_storage &ft_socket::get_address() const
{
        this->set_error(ER_SUCCESS);
        return (this->_address);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags, int exception)
{
    ssize_t total_bytes_sent = 0;
    size_t index = 0;

    while (index < this->_connected.size())
    {
        if (exception == this->_connected[index].get_fd())
        {
            index++;
            continue ;
        }
        ssize_t bytes_sent = this->_connected[index].send_data(data, size, flags);
        if (bytes_sent < 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            continue ;
        }
        total_bytes_sent += bytes_sent;
        index++;
    }
    if (total_bytes_sent >= 0)
        this->set_error(ER_SUCCESS);
    return (total_bytes_sent);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags)
{
    ssize_t total_bytes_sent = 0;
    size_t index = 0;

    while (index < this->_connected.size())
    {
        ssize_t bytes_sent = this->_connected[index].send_data(data, size, flags);
        if (bytes_sent < 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            continue ;
        }
        total_bytes_sent += bytes_sent;
        index++;
    }
    if (total_bytes_sent >= 0)
        this->set_error(ER_SUCCESS);
    return (total_bytes_sent);
}

int ft_socket::accept_connection()
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_fd = nw_accept(this->_socket_fd,
                           reinterpret_cast<struct sockaddr*>(&client_addr),
                           &addr_len);
    if (new_fd < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    ft_socket new_socket(new_fd, client_addr);
    this->_connected.push_back(ft_move(new_socket));
    if (this->_connected.get_error())
    {
        this->set_error(VECTOR_ALLOC_FAIL);
    }
    else
        this->set_error(ER_SUCCESS);
    return (new_fd);
}

bool ft_socket::disconnect_client(int fd)
{
    size_t index = 0;

    while (index < this->_connected.size())
    {
        if (this->_connected[index].get_fd() == fd)
        {
            size_t last = this->_connected.size() - 1;
            if (index != last)
                    this->_connected[index] = ft_move(this->_connected[last]);
            this->_connected.pop_back();
            this->set_error(ER_SUCCESS);
            return (true);
        }
        index++;
    }
    this->set_error(FT_EINVAL);
    return (false);
}

void ft_socket::disconnect_all_clients()
{
    size_t index = 0;
    while (index < this->_connected.size())
    {
        this->_connected[index].close_socket();
        index++;
    }
    this->_connected.clear();
    this->set_error(ER_SUCCESS);
    return ;
}

size_t ft_socket::get_client_count() const
{
    this->set_error(ER_SUCCESS);
    return (this->_connected.size());
}

bool ft_socket::is_client_connected(int fd) const
{
    size_t index = 0;

    while (index < this->_connected.size())
    {
        if (this->_connected[index].get_fd() == fd)
        {
            this->set_error(ER_SUCCESS);
            return (true);
        }
        index++;
    }
    this->set_error(ER_SUCCESS);
    return (false);
}

ft_socket::ft_socket(int fd, const sockaddr_storage &addr) : _address(addr), _socket_fd(fd),
                        _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_socket::ft_socket(const SocketConfig &config) : _socket_fd(-1), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    if (config._type == SocketType::SERVER)
        setup_server(config);
    else if (config._type == SocketType::CLIENT)
        setup_client(config);
    else
    {
        this->set_error(SOCKET_UNSUPPORTED_TYPE);
    }
    return ;
}

ft_socket::~ft_socket()
{
    FT_CLOSE_SOCKET(this->_socket_fd);
    return ;
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_sent = nw_send(this->_socket_fd, data, size, flags);
    if (bytes_sent < 0)
        this->set_error(errno + ERRNO_OFFSET);
    else
        this->set_error(ER_SUCCESS);
    return (bytes_sent);
}

ssize_t ft_socket::send_all(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return (-1);
    }
    size_t total_sent = 0;
    const char *buffer = static_cast<const char *>(data);
    while (total_sent < size)
    {
        ssize_t bytes_sent = nw_send(this->_socket_fd,
                                     buffer + total_sent,
                                     size - total_sent,
                                     flags);
        if (bytes_sent < 0)
        {
            this->set_error(errno + ERRNO_OFFSET);
            return (-1);
        }
        if (bytes_sent == 0)
        {
            this->set_error(SOCKET_SEND_FAILED);
            return (-1);
        }
        total_sent += bytes_sent;
    }
    if (networking_check_socket_after_send(this->_socket_fd) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (static_cast<ssize_t>(total_sent));
}

ssize_t ft_socket::receive_data(void *buffer, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    ssize_t bytes_received = nw_recv(this->_socket_fd, buffer, size, flags);
    if (bytes_received < 0)
        this->set_error(errno + ERRNO_OFFSET);
    else
        this->set_error(ER_SUCCESS);
    return (bytes_received);
}

bool ft_socket::close_socket()
{
    if (this->_socket_fd >= 0)
    {
        if (FT_CLOSE_SOCKET(this->_socket_fd) == 0)
        {
            this->_socket_fd = -1;
            this->set_error(ER_SUCCESS);
            return (true);
        }
        else
        {
            this->set_error(errno + ERRNO_OFFSET);
            return (false);
        }
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

int ft_socket::get_error() const
{
    return (this->_error_code);
}

const char* ft_socket::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

ft_socket::ft_socket(ft_socket &&other) noexcept
    : _address(other._address), _connected(ft_move(other._connected)),
    _socket_fd(other._socket_fd), _error_code(other._error_code)
{
    other._socket_fd = -1;
    other.set_error(ER_SUCCESS);
    return ;
}

ft_socket &ft_socket::operator=(ft_socket &&other) noexcept
{
    if (this != &other)
    {
        close_socket();
        this->_address = other._address;
        this->_socket_fd = other._socket_fd;
        this->set_error(other._error_code);
        this->_connected = ft_move(other._connected);
        other._socket_fd = -1;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

int ft_socket::initialize(const SocketConfig &config)
{
    if (this->_socket_fd != -1)
    {
        this->set_error(SOCKET_ALRDY_INITIALIZED);
        return (1);
    }
    if (config._type == SocketType::SERVER)
        setup_server(config);
    else if (config._type == SocketType::CLIENT)
        setup_client(config);
    else
    {
        this->set_error(SOCKET_UNSUPPORTED_TYPE);
    }
    return (0);
}
