#include "networking.hpp"
#include "socket_class.hpp"
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
# include <netinet/in.h>
# include <unistd.h>
# include <sys/socket.h>
#endif

int ft_socket::setup_client(const SocketConfig &config)
{
    if (create_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (config._non_blocking)
        if (set_non_blocking(config) != ER_SUCCESS)
            return (this->_error_code);
    if (config._recv_timeout > 0 || config._send_timeout > 0)
        if (set_timeouts(config) != ER_SUCCESS)
            return (this->_error_code);
    if (configure_address(config) != ER_SUCCESS)
        return (this->_error_code);
    socklen_t addr_len;
    if (config._address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else if (config._address_family == AF_INET6)
        addr_len = sizeof(struct sockaddr_in6);
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        FT_CLOSE_SOCKET(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    if (nw_connect(this->_socket_fd, reinterpret_cast<const struct sockaddr*>
                (&this->_address), addr_len) < 0)
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (!(config._non_blocking && last_error == WSAEWOULDBLOCK))
        {
            this->set_error(ft_map_system_error(last_error));
            FT_CLOSE_SOCKET(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
#else
        int last_error;

        last_error = errno;
        if (!(config._non_blocking && (last_error == EINPROGRESS
            || last_error == EWOULDBLOCK)))
        {
            this->set_error(ft_map_system_error(last_error));
            FT_CLOSE_SOCKET(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
#endif
    }
    if (!config._multicast_group.empty())
        if (join_multicast_group(config) != ER_SUCCESS)
            return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (this->_error_code);
}
