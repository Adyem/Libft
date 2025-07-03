#include "socket_class.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
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
#include <utility>

ft_socket::ft_socket() : _socket_fd(-1), _error(ER_SUCCESS)
{
	ft_bzero(&this->_address, sizeof(this->_address));
	return ;
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags, int fd)
{
	size_t index = 0;
    while (index < this->_connected.size())
    {
        if (_connected[index].get_fd() == fd)
        {
            ssize_t bytes_sent = this->_connected[index].send_data(data, size, flags);
            if (bytes_sent < 0)
			{
				this->_error = errno + ERRNO_OFFSET;
				ft_errno = this->_error;
			}
            return (bytes_sent);
        }
		index++;
    }
    ft_errno = FT_EINVAL;
    this->_error = ft_errno;
    return (-1);
}

int ft_socket::get_fd() const
{
        return (this->_socket_fd);
}

const struct sockaddr_storage &ft_socket::get_address() const
{
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
			ft_errno = errno + ERRNO_OFFSET;
			this->_error = ft_errno;
            continue ;
		}
        total_bytes_sent += bytes_sent;
		index++;
    }
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
			ft_errno = errno + ERRNO_OFFSET;
			_error = ft_errno;
            continue ;
		}
        total_bytes_sent += bytes_sent;
		index++;
    }
    return (total_bytes_sent);
}

int ft_socket::accept_connection()
{
    if (this->_socket_fd < 0)
    {
        ft_errno = FT_EINVAL;
        this->_error = ft_errno;
		return (-1);
    }
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int new_fd = nw_accept(this->_socket_fd,
                           reinterpret_cast<struct sockaddr*>(&client_addr),
                           &addr_len);
    if (new_fd < 0)
    {
        ft_errno = errno + ERRNO_OFFSET;
        this->_error = ft_errno;
        return (-1);
    }
    ft_socket new_socket(new_fd, client_addr);
    this->_connected.push_back(std::move(new_socket));
	if (this->_connected.get_error())
	{
		ft_errno = VECTOR_ALLOC_FAIL;
		this->_error = ft_errno;
	}
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
                    this->_connected[index] = std::move(this->_connected[last]);
            this->_connected.pop_back();
            this->_error = ER_SUCCESS;
            return (true);
        }
        index++;
    }
    ft_errno = FT_EINVAL;
    this->_error = ft_errno;
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
    return ;
}

size_t ft_socket::get_client_count() const
{
    return (this->_connected.size());
}

bool ft_socket::is_client_connected(int fd) const
{
    size_t index = 0;

    while (index < this->_connected.size())
    {
        if (this->_connected[index].get_fd() == fd)
            return (true);
        index++;
    }
    return (false);
}

ft_socket::ft_socket(int fd, const sockaddr_storage &addr) : _address(addr), _socket_fd(fd),
						_error(ER_SUCCESS)
{
	return ;
}

ft_socket::ft_socket(const SocketConfig &config) : _socket_fd(-1), _error(ER_SUCCESS)
{
    if (config.type == SocketType::SERVER)
        setup_server(config);
    else if (config.type == SocketType::CLIENT)
        setup_client(config);
    else
    {
        ft_errno = SOCKET_UNSUPPORTED_TYPE;
        this->_error = ft_errno;
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
        ft_errno = SOCKET_INVALID_CONFIGURATION;
        this->_error = ft_errno;
        return (-1);
    }
    ssize_t bytes_sent = nw_send(this->_socket_fd, data, size, flags);
    if (bytes_sent < 0)
	{
        ft_errno = errno + ERRNO_OFFSET;
        this->_error = ft_errno;
    }
    else
        this->_error = ER_SUCCESS;
    return (bytes_sent);
}

ssize_t ft_socket::send_all(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        ft_errno = SOCKET_INVALID_CONFIGURATION;
        this->_error = ft_errno;
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
            ft_errno = errno + ERRNO_OFFSET;
            this->_error = ft_errno;
            return (-1);
        }
        total_sent += bytes_sent;
    }
    this->_error = ER_SUCCESS;
    return (static_cast<ssize_t>(total_sent));
}

ssize_t ft_socket::receive_data(void *buffer, size_t size, int flags)
{
    if (this->_socket_fd < 0)
	{
        ft_errno = FT_EINVAL;
        this->_error = ft_errno;
        return (-1);
	} 
    ssize_t bytes_received = nw_recv(this->_socket_fd, buffer, size, flags);
    if (bytes_received < 0)
	{
        ft_errno = errno + ERRNO_OFFSET;
        this->_error = ft_errno;
    }
    else
        this->_error = ER_SUCCESS;
    return (bytes_received);
}

bool ft_socket::close_socket()
{
    if (this->_socket_fd >= 0)
	{
        if (FT_CLOSE_SOCKET(this->_socket_fd) == 0)
		{
            this->_socket_fd = -1;
            this->_error = ER_SUCCESS;
            return (true);
        }
        else
		{
            ft_errno = errno + ERRNO_OFFSET;
            this->_error = ft_errno;
            return (false);
        }
    }
    this->_error = ER_SUCCESS;
    return (true);
}

int ft_socket::get_error() const
{
    return (this->_error);
}

const char* ft_socket::get_error_str() const
{
    return (ft_strerror(this->_error));
}

ft_socket::ft_socket(ft_socket &&other) noexcept
    : _address(other._address), _connected(std::move(other._connected)),
	_socket_fd(other._socket_fd), _error(other._error)
{
    other._socket_fd = -1;
	return ;
}

ft_socket &ft_socket::operator=(ft_socket &&other) noexcept
{
    if (this != &other)
	{
        close_socket();
        this->_address = other._address;
        this->_socket_fd = other._socket_fd;
        this->_error = other._error;
        this->_connected = std::move(other._connected);
        other._socket_fd = -1;
    }
    return (*this);
}

int ft_socket::initialize(const SocketConfig &config)
{
	if (this->_socket_fd != -1)
	{
		this->_error = SOCKET_ALRDY_INITIALIZED;
		ft_errno = SOCKET_ALRDY_INITIALIZED;
		return (1);
	}
    if (config.type == SocketType::SERVER)
        setup_server(config);
    else if (config.type == SocketType::CLIENT)
        setup_client(config);
    else
    {
        ft_errno = SOCKET_UNSUPPORTED_TYPE;
        this->_error = ft_errno;
    }
	return (0);
}
