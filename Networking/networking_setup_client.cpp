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
    SocketConfig *mutable_config;
    bool lock_acquired;
    bool non_blocking;
    bool has_timeout;
    bool has_multicast;
    int address_family;
    socklen_t addr_len;

    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (this->_error_code);
    }
    non_blocking = mutable_config->_non_blocking;
    has_timeout = (mutable_config->_recv_timeout > 0 || mutable_config->_send_timeout > 0);
    has_multicast = (mutable_config->_multicast_group.empty() == false);
    address_family = mutable_config->_address_family;
    socket_config_unlock(mutable_config, lock_acquired);
    if (this->create_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (non_blocking)
        if (this->set_non_blocking(config) != ER_SUCCESS)
            return (this->_error_code);
    if (has_timeout)
        if (this->set_timeouts(config) != ER_SUCCESS)
            return (this->_error_code);
    if (this->configure_address(config) != ER_SUCCESS)
        return (this->_error_code);
    if (address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else if (address_family == AF_INET6)
        addr_len = sizeof(struct sockaddr_in6);
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        this->close_socket();
        return (this->_error_code);
    }
    if (nw_connect(this->_socket_fd,
            reinterpret_cast<const struct sockaddr*>(&this->_address), addr_len) < 0)
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (!(non_blocking && last_error == WSAEWOULDBLOCK))
        {
            this->set_error(ft_map_system_error(last_error));
            this->close_socket();
            return (this->_error_code);
        }
#else
        int last_error;

        last_error = errno;
        if (!(non_blocking && (last_error == EINPROGRESS || last_error == EWOULDBLOCK)))
        {
            this->set_error(ft_map_system_error(last_error));
            this->close_socket();
            return (this->_error_code);
        }
#endif
    }
    if (has_multicast)
        if (this->join_multicast_group(config) != ER_SUCCESS)
            return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (this->_error_code);
}
