#include "udp_socket.hpp"
#include "../Libft/libft.hpp"
#include <cerrno>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
#endif

#ifdef _WIN32
static inline int setsockopt_reuse(int fd, int opt)
{
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int set_timeout_recv(int fd, int ms)
{
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&ms), sizeof(ms)) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int set_timeout_send(int fd, int ms)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&ms), sizeof(ms)) == SOCKET_ERROR)
    {
        ft_errno = ft_map_system_error(WSAGetLastError());
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#else
static inline int setsockopt_reuse(int fd, int opt)
{
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int set_timeout_recv(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

static inline int set_timeout_send(int fd, int ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    {
        ft_errno = ft_map_system_error(errno);
        return (-1);
    }
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif

udp_socket::udp_socket() : _socket_fd(-1), _error_code(ER_SUCCESS)
{
    ft_bzero(&this->_address, sizeof(this->_address));
    this->set_error(ER_SUCCESS);
    return ;
}

udp_socket::~udp_socket()
{
    close_socket();
    return ;
}

void udp_socket::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = ft_errno;
    return ;
}

int udp_socket::create_socket(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;

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
    this->_socket_fd = nw_socket(mutable_config->_address_family, SOCK_DGRAM, mutable_config->_protocol);
    socket_config_unlock(mutable_config, lock_acquired);
    if (this->_socket_fd < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_CREATION_FAILED;
        this->set_error(error_code);
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int udp_socket::set_non_blocking(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;

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
    if (mutable_config->_non_blocking == false)
    {
        socket_config_unlock(mutable_config, lock_acquired);
        return (ER_SUCCESS);
    }
    if (nw_set_nonblocking(this->_socket_fd) != 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_CONFIGURATION;
        this->set_error(error_code);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        socket_config_unlock(mutable_config, lock_acquired);
        return (this->_error_code);
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int udp_socket::set_timeouts(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;

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
    if (mutable_config->_recv_timeout > 0)
    {
        if (set_timeout_recv(this->_socket_fd, mutable_config->_recv_timeout) < 0)
        {
            int error_code;

            error_code = ft_errno;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_CONFIGURATION;
            this->set_error(error_code);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            socket_config_unlock(mutable_config, lock_acquired);
            return (this->_error_code);
        }
    }
    if (mutable_config->_send_timeout > 0)
    {
        if (set_timeout_send(this->_socket_fd, mutable_config->_send_timeout) < 0)
        {
            int error_code;

            error_code = ft_errno;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_CONFIGURATION;
            this->set_error(error_code);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            socket_config_unlock(mutable_config, lock_acquired);
            return (this->_error_code);
        }
    }
    socket_config_unlock(mutable_config, lock_acquired);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int udp_socket::configure_address(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;
    ft_string     host_copy;
    uint16_t      port_value;
    int           address_family;
    int           protocol_value;

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
    ft_bzero(&this->_address, sizeof(this->_address));
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
            return (ER_SUCCESS);
        }
        if (nw_inet_pton(AF_INET, host_copy.c_str(), &addr_in->sin_addr) > 0)
        {
            this->set_error(ER_SUCCESS);
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
            return (ER_SUCCESS);
        }
        if (nw_inet_pton(AF_INET6, host_copy.c_str(), &addr_in6->sin6_addr) > 0)
        {
            this->set_error(ER_SUCCESS);
            return (ER_SUCCESS);
        }
    }
    else
    {
        this->set_error(FT_ERR_CONFIGURATION);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
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
        return (this->_error_code);
    }
    ft_memset(&resolved_address, 0, sizeof(resolved_address));
    if (!networking_dns_resolve_first(host_copy.c_str(), port_string.c_str(), address_family,
            SOCK_DGRAM, protocol_value, 0, resolved_address))
    {
        resolver_error = ft_errno;
        if (resolver_error == ER_SUCCESS)
            resolver_error = FT_ERR_SOCKET_RESOLVE_FAILED;
        this->set_error(resolver_error);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    if (resolved_address.length > sizeof(this->_address))
    {
        this->set_error(FT_ERR_SOCKET_RESOLVE_FAILED);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
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
    return (ER_SUCCESS);
}

int udp_socket::bind_socket(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;
    SocketType    type;
    int           address_family;
    socklen_t     addr_len;

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
    type = mutable_config->_type;
    address_family = mutable_config->_address_family;
    socket_config_unlock(mutable_config, lock_acquired);
    if (type != SocketType::SERVER)
        return (ER_SUCCESS);
    if (address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else
        addr_len = sizeof(struct sockaddr_in6);
    if (nw_bind(this->_socket_fd,
            reinterpret_cast<const struct sockaddr*>(&this->_address),
            addr_len) < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_BIND_FAILED;
        this->set_error(error_code);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int udp_socket::connect_socket(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;
    SocketType    type;
    int           address_family;
    socklen_t     addr_len;

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
    type = mutable_config->_type;
    address_family = mutable_config->_address_family;
    socket_config_unlock(mutable_config, lock_acquired);
    if (type != SocketType::CLIENT)
        return (ER_SUCCESS);
    if (address_family == AF_INET)
        addr_len = sizeof(struct sockaddr_in);
    else
        addr_len = sizeof(struct sockaddr_in6);
    if (nw_connect(this->_socket_fd,
            reinterpret_cast<const struct sockaddr*>(&this->_address),
            addr_len) < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_CONNECT_FAILED;
        this->set_error(error_code);
        nw_close(this->_socket_fd);
        this->_socket_fd = -1;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int udp_socket::initialize(const SocketConfig &config)
{
    SocketConfig *mutable_config;
    bool          lock_acquired;
    bool          non_blocking;
    bool          has_timeout;
    bool          reuse_address;

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
    reuse_address = mutable_config->_reuse_address;
    socket_config_unlock(mutable_config, lock_acquired);
    if (create_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (non_blocking)
        if (set_non_blocking(config) != ER_SUCCESS)
            return (this->_error_code);
    if (has_timeout)
        if (set_timeouts(config) != ER_SUCCESS)
            return (this->_error_code);
    if (configure_address(config) != ER_SUCCESS)
        return (this->_error_code);
    if (reuse_address)
        if (setsockopt_reuse(this->_socket_fd, 1) < 0)
        {
            int error_code;

            error_code = ft_errno;
            if (error_code == ER_SUCCESS)
                error_code = FT_ERR_CONFIGURATION;
            this->set_error(error_code);
            nw_close(this->_socket_fd);
            this->_socket_fd = -1;
            return (this->_error_code);
        }
    if (bind_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    if (connect_socket(config) != ER_SUCCESS)
        return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (this->_error_code);
}

ssize_t udp_socket::send_to(const void *data, size_t size, int flags,
                            const struct sockaddr *dest_addr, socklen_t addr_len)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_sent;
    bytes_sent = nw_sendto(this->_socket_fd, data, size, flags, dest_addr, addr_len);
    if (bytes_sent < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_SEND_FAILED;
        this->set_error(error_code);
    }
    else
        this->set_error(ER_SUCCESS);
    return (bytes_sent);
}

ssize_t udp_socket::receive_from(void *buffer, size_t size, int flags,
                                 struct sockaddr *src_addr, socklen_t *addr_len)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_received;
    bytes_received = nw_recvfrom(this->_socket_fd, buffer, size, flags, src_addr, addr_len);
    if (bytes_received < 0)
    {
        int error_code;

        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_SOCKET_RECEIVE_FAILED;
        this->set_error(error_code);
    }
    else
        this->set_error(ER_SUCCESS);
    return (bytes_received);
}

bool udp_socket::close_socket()
{
    if (this->_socket_fd >= 0)
    {
        if (nw_close(this->_socket_fd) == 0)
        {
            this->_socket_fd = -1;
            this->set_error(ER_SUCCESS);
            return (true);
        }
#ifdef _WIN32
        this->set_error(ft_map_system_error(WSAGetLastError()));
#else
        this->set_error(ft_map_system_error(errno));
#endif
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

int udp_socket::get_error() const
{
    return (this->_error_code);
}

const char *udp_socket::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

int udp_socket::get_fd() const
{
    this->set_error(ER_SUCCESS);
    return (this->_socket_fd);
}

const struct sockaddr_storage &udp_socket::get_address() const
{
    this->set_error(ER_SUCCESS);
    return (this->_address);
}

