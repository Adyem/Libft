#include "networking.hpp"
#include "socket_class.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <cerrno>
#include <fcntl.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/socket.h>
#endif

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
    u_long mode;

    mode = 1;
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
    int flags;

    flags = fcntl(fd, F_GETFL, 0);
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
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->_socket_fd = nw_socket(mutable_config->_address_family, SOCK_STREAM,
            mutable_config->_protocol);
    if (this->_socket_fd < 0)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(translate_platform_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::set_reuse_address(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;
    int opt;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (mutable_config->_reuse_address == false)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(ER_SUCCESS);
        ft_socket::restore_errno(guard, entry_errno);
        return (ER_SUCCESS);
    }
    opt = 1;
    if (setsockopt_reuse(this->_socket_fd, opt) < 0)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::set_non_blocking(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (mutable_config->_non_blocking == false)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(ER_SUCCESS);
        ft_socket::restore_errno(guard, entry_errno);
        return (ER_SUCCESS);
    }
    if (set_nonblocking_platform(this->_socket_fd) != 0)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::set_timeouts(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (mutable_config->_recv_timeout > 0)
    {
        if (set_timeout_recv(this->_socket_fd, mutable_config->_recv_timeout) < 0)
        {
            socket_config_unlock(mutable_config, lock_acquired);
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
    }
    if (mutable_config->_send_timeout > 0)
    {
        if (set_timeout_send(this->_socket_fd, mutable_config->_send_timeout) < 0)
        {
            socket_config_unlock(mutable_config, lock_acquired);
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::configure_address(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;
    ft_string host_copy;
    uint16_t port_value;
    int address_family;
    int protocol_value;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    ft_memset(&this->_address, 0, sizeof(this->_address));
    host_copy = mutable_config->_ip;
    port_value = mutable_config->_port;
    address_family = mutable_config->_address_family;
    protocol_value = mutable_config->_protocol;
    socket_config_unlock(mutable_config, lock_acquired);
    if (host_copy.get_error() != ER_SUCCESS)
    {
        this->set_error(host_copy.get_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (address_family == AF_INET)
    {
        struct sockaddr_in *addr_in;

        addr_in = reinterpret_cast<struct sockaddr_in*>(&this->_address);
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(port_value);
        if (host_copy.empty())
        {
            addr_in->sin_addr.s_addr = htonl(INADDR_ANY);
            this->set_error(ER_SUCCESS);
            ft_socket::restore_errno(guard, entry_errno);
            return (ER_SUCCESS);
        }
        if (nw_inet_pton(AF_INET, host_copy.c_str(), &addr_in->sin_addr) > 0)
        {
            this->set_error(ER_SUCCESS);
            ft_socket::restore_errno(guard, entry_errno);
            return (ER_SUCCESS);
        }
    }
    else if (address_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6;

        addr_in6 = reinterpret_cast<struct sockaddr_in6*>(&this->_address);
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(port_value);
        if (host_copy.empty())
        {
            addr_in6->sin6_addr = in6addr_any;
            this->set_error(ER_SUCCESS);
            ft_socket::restore_errno(guard, entry_errno);
            return (ER_SUCCESS);
        }
        if (nw_inet_pton(AF_INET6, host_copy.c_str(), &addr_in6->sin6_addr) > 0)
        {
            this->set_error(ER_SUCCESS);
            ft_socket::restore_errno(guard, entry_errno);
            return (ER_SUCCESS);
        }
    }
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    ft_string port_string;
    networking_resolved_address resolved_address;
    int resolver_error;

    port_string = ft_to_string(static_cast<long>(port_value));
    if (port_string.get_error() != ER_SUCCESS)
    {
        this->set_error(port_string.get_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    ft_memset(&resolved_address, 0, sizeof(resolved_address));
    if (!networking_dns_resolve_first(host_copy.c_str(), port_string.c_str(),
            address_family, SOCK_STREAM, protocol_value, 0, resolved_address))
    {
        resolver_error = ft_errno;
        if (resolver_error == ER_SUCCESS)
            resolver_error = FT_ERR_SOCKET_RESOLVE_FAILED;
        this->set_error(resolver_error);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (resolved_address.length > sizeof(this->_address))
    {
        this->set_error(FT_ERR_SOCKET_RESOLVE_FAILED);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    ft_memcpy(&this->_address, &resolved_address.address, resolved_address.length);
    if (address_family == AF_INET)
    {
        struct sockaddr_in *addr_in;

        addr_in = reinterpret_cast<struct sockaddr_in*>(&this->_address);
        addr_in->sin_family = AF_INET;
        addr_in->sin_port = htons(port_value);
    }
    else
    {
        struct sockaddr_in6 *addr_in6;

        addr_in6 = reinterpret_cast<struct sockaddr_in6*>(&this->_address);
        addr_in6->sin6_family = AF_INET6;
        addr_in6->sin6_port = htons(port_value);
    }
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::bind_socket(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    socklen_t addr_len;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    if (config._address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else if (config._address_family == AF_INET6)
        addr_len = sizeof(struct sockaddr_in6);
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (nw_bind(this->_socket_fd,
            reinterpret_cast<const struct sockaddr*>(&this->_address), addr_len) < 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_socket::listen_socket(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;
    int backlog;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    backlog = mutable_config->_backlog;
    socket_config_unlock(mutable_config, lock_acquired);
    if (nw_listen(this->_socket_fd, backlog) < 0)
    {
        this->set_error(translate_platform_error());
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
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
    SocketConfig *mutable_config;
    bool lock_acquired;
    bool reuse_address;
    bool non_blocking;
    bool has_timeout;
    bool has_multicast;

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
    reuse_address = mutable_config->_reuse_address;
    non_blocking = mutable_config->_non_blocking;
    has_timeout = (mutable_config->_recv_timeout > 0 || mutable_config->_send_timeout > 0);
    has_multicast = (mutable_config->_multicast_group.empty() == false);
    socket_config_unlock(mutable_config, lock_acquired);
    if (this->create_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (reuse_address)
        if (this->set_reuse_address(config) != ER_SUCCESS)
            return (this->_error_code);
    if (non_blocking)
        if (this->set_non_blocking(config) != ER_SUCCESS)
            return (this->_error_code);
    if (has_timeout)
        if (this->set_timeouts(config) != ER_SUCCESS)
            return (this->_error_code);
    if (this->configure_address(config) != ER_SUCCESS)
        return (this->_error_code);
    if (this->bind_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (this->listen_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (has_multicast)
        if (this->join_multicast_group(config) != ER_SUCCESS)
            return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (this->_error_code);
}

int ft_socket::join_multicast_group(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    SocketConfig *mutable_config;
    bool lock_acquired;

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    mutable_config = const_cast<SocketConfig*>(&config);
    lock_acquired = false;
    if (socket_config_prepare_thread_safety(mutable_config) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (socket_config_lock(mutable_config, &lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    if (mutable_config->_multicast_group.empty())
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(ER_SUCCESS);
        ft_socket::restore_errno(guard, entry_errno);
        return (ER_SUCCESS);
    }
    if (mutable_config->_address_family == AF_INET)
    {
        struct ip_mreq mreq;

        ft_bzero(&mreq, sizeof(mreq));
        if (nw_inet_pton(AF_INET, mutable_config->_multicast_group.c_str(),
                &mreq.imr_multiaddr) <= 0)
        {
            socket_config_unlock(mutable_config, lock_acquired);
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        if (mutable_config->_multicast_interface.empty())
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        else if (nw_inet_pton(AF_INET,
                mutable_config->_multicast_interface.c_str(), &mreq.imr_interface) <= 0)
        {
            socket_config_unlock(mutable_config, lock_acquired);
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        socket_config_unlock(mutable_config, lock_acquired);
        if (setsockopt(this->_socket_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                reinterpret_cast<const char*>(&mreq), sizeof(mreq)) < 0)
        {
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
    }
    else if (mutable_config->_address_family == AF_INET6)
    {
        struct ipv6_mreq mreq6;

        ft_bzero(&mreq6, sizeof(mreq6));
        if (nw_inet_pton(AF_INET6, mutable_config->_multicast_group.c_str(),
                &mreq6.ipv6mr_multiaddr) <= 0)
        {
            socket_config_unlock(mutable_config, lock_acquired);
            this->set_error(FT_ERR_CONFIGURATION);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
        mreq6.ipv6mr_interface = 0;
        socket_config_unlock(mutable_config, lock_acquired);
        if (setsockopt(this->_socket_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
                reinterpret_cast<const char*>(&mreq6), sizeof(mreq6)) < 0)
        {
            this->set_error(translate_platform_error());
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            ft_socket::restore_errno(guard, entry_errno);
            return (this->_error_code);
        }
    }
    else
    {
        socket_config_unlock(mutable_config, lock_acquired);
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    ft_socket::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}
