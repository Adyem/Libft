#include "networking.hpp"
#include "socket_class.hpp"
#include "../Errno/errno.hpp"
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
#include "../Libft/libft.hpp"

#ifdef _WIN32
static inline int translate_platform_error()
{
    return (ft_map_system_error(WSAGetLastError()));
}
#else
static inline int translate_platform_error()
{
    return (ft_map_system_error(errno));
}
#endif

#ifdef _WIN32
static inline int setsockopt_reuse(int fd, int opt)
{
    return (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                      reinterpret_cast<const char*>(&opt), sizeof(opt)));
}

static inline int set_nonblocking_platform(int fd)
{
    u_long mode = 1;
    return (ioctlsocket(static_cast<SOCKET>(fd), FIONBIO, &mode));
}

static inline int set_timeout_recv(int fd, int ms)
{
    return (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                      reinterpret_cast<const char*>(&ms), sizeof(ms)));
}

static inline int set_timeout_send(int fd, int ms)
{
    return (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                      reinterpret_cast<const char*>(&ms), sizeof(ms)));
}
#else
static inline int setsockopt_reuse(int fd, int opt)
{
    return (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)));
}

static inline int set_nonblocking_platform(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return (-1);
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

static inline int set_timeout_recv(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)));
}

static inline int set_timeout_send(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    return (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)));
}
#endif

int ft_socket::create_socket(const SocketConfig &config)
{
    this->_socket_fd = nw_socket(config._address_family, SOCK_STREAM, config._protocol);
    if (this->_socket_fd < 0)
    {
        this->set_error(translate_platform_error());
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_socket::set_reuse_address(const SocketConfig &config)
{
    if (!config._reuse_address)
        return (ER_SUCCESS);
    int opt = 1;
    if (setsockopt_reuse(this->_socket_fd, opt) < 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_socket::set_non_blocking(const SocketConfig &config)
{
    if (!config._non_blocking)
        return (ER_SUCCESS);
    if (set_nonblocking_platform(this->_socket_fd) != 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_socket::set_timeouts(const SocketConfig &config)
{
    if (config._recv_timeout > 0)
    {
        if (set_timeout_recv(this->_socket_fd, config._recv_timeout) < 0)
        {
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    if (config._send_timeout > 0)
    {
        if (set_timeout_send(this->_socket_fd, config._send_timeout) < 0)
        {
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_socket::configure_address(const SocketConfig &config)
{
    ft_memset(&this->_address, 0, sizeof(this->_address));

    if (config._address_family == AF_INET)
    {
        struct sockaddr_in *addr_in = reinterpret_cast<struct sockaddr_in*>(&this->_address);
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(config._port);
        if (config._ip.empty())
        {
            addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else if (nw_inet_pton(AF_INET, config._ip.c_str(), &addr_in->sin_addr) <= 0)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    else if (config._address_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6 = reinterpret_cast<struct sockaddr_in6*>(&this->_address);
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(config._port);
        if (config._ip.empty())
        {
            addr_in6->sin6_addr = in6addr_any;
        }
        else if (nw_inet_pton(AF_INET6, config._ip.c_str(), &addr_in6->sin6_addr) <= 0)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_socket::bind_socket(const SocketConfig &config)
{
    socklen_t addr_len;

    if (config._address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else if (config._address_family == AF_INET6)
        addr_len = sizeof(struct sockaddr_in6);
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    if (nw_bind(this->_socket_fd, reinterpret_cast<const struct sockaddr*>(&this->_address),
                addr_len) < 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}


int ft_socket::listen_socket(const SocketConfig &config)
{
    if (nw_listen(this->_socket_fd, config._backlog) < 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void ft_socket::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = ft_errno;
    return ;
}

int ft_socket::setup_server(const SocketConfig &config)
{
    if (create_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (config._reuse_address)
        if (set_reuse_address(config) != ER_SUCCESS)
            return (this->_error_code);
    if (config._non_blocking)
        if (set_non_blocking(config) != ER_SUCCESS)
            return (this->_error_code);
    if (config._recv_timeout > 0 || config._send_timeout > 0)
        if (set_timeouts(config) != ER_SUCCESS)
            return (this->_error_code);
    if (configure_address(config) != ER_SUCCESS)
        return (this->_error_code);
    if (bind_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (listen_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (!config._multicast_group.empty())
        if (join_multicast_group(config) != ER_SUCCESS)
            return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (this->_error_code);
}

int ft_socket::join_multicast_group(const SocketConfig &config)
{
    if (config._multicast_group.empty())
        return (ER_SUCCESS);
    if (config._address_family == AF_INET)
    {
        struct ip_mreq mreq;
        ft_bzero(&mreq, sizeof(mreq));
        if (nw_inet_pton(AF_INET, config._multicast_group.c_str(), &mreq.imr_multiaddr) <= 0)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
        if (config._multicast_interface.empty())
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        else if (nw_inet_pton(AF_INET, config._multicast_interface.c_str(), &mreq.imr_interface) <= 0)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
        if (setsockopt(this->_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0)
        {
            this->set_error(FT_ERR_SOCKET_JOIN_GROUP_FAILED);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    else if (config._address_family == AF_INET6)
    {
        struct ipv6_mreq mreq6;
        ft_bzero(&mreq6, sizeof(mreq6));
        if (nw_inet_pton(AF_INET6, config._multicast_group.c_str(), &mreq6.ipv6mr_multiaddr) <= 0)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
        mreq6.ipv6mr_interface = 0;
        if (setsockopt(this->_socket_fd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                       reinterpret_cast<const char*>(&mreq6), sizeof(mreq6)) < 0)
        {
            this->set_error(FT_ERR_SOCKET_JOIN_GROUP_FAILED);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    }
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}
