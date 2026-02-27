#include "networking.hpp"
#include "socket_class.hpp"
#include <cerrno>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
#endif

int ft_socket::setup_client(const SocketConfig &config)
{
    bool non_blocking;
    bool has_timeout;
    bool has_multicast;
    int address_family;
    socklen_t address_length;
    int setup_error;

    non_blocking = config._non_blocking;
    has_timeout = (config._recv_timeout > 0 || config._send_timeout > 0);
    has_multicast = (config._multicast_group[0] != '\0');
    address_family = config._address_family;
    setup_error = this->create_socket(config);
    if (setup_error != FT_ERR_SUCCESS)
        return (setup_error);
    if (non_blocking != false)
    {
        setup_error = this->set_non_blocking(config);
        if (setup_error != FT_ERR_SUCCESS)
            return (setup_error);
    }
    if (has_timeout != false)
    {
        setup_error = this->set_timeouts(config);
        if (setup_error != FT_ERR_SUCCESS)
            return (setup_error);
    }
    setup_error = this->configure_address(config);
    if (setup_error != FT_ERR_SUCCESS)
        return (setup_error);
    if (address_family == AF_INET)
        address_length = sizeof(struct sockaddr_in);
    else if (address_family == AF_INET6)
        address_length = sizeof(struct sockaddr_in6);
    else
    {
        (void)this->close_socket();
        return (FT_ERR_CONFIGURATION);
    }
    if (nw_connect(this->_socket_file_descriptor,
            reinterpret_cast<const struct sockaddr *>(&this->_address), address_length) < 0)
    {
#ifdef _WIN32
        if (!(non_blocking != false && WSAGetLastError() == WSAEWOULDBLOCK))
#else
        if (!(non_blocking != false && (errno == EINPROGRESS || errno == EWOULDBLOCK)))
#endif
        {
            (void)this->close_socket();
            return (FT_ERR_SOCKET_CONNECT_FAILED);
        }
    }
    if (has_multicast != false)
    {
        setup_error = this->join_multicast_group(config);
        if (setup_error != FT_ERR_SUCCESS)
            return (setup_error);
    }
    return (FT_ERR_SUCCESS);
}
