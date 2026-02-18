#include "udp_socket.hpp"
#include "networking.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
#endif

#ifdef _WIN32
static int setsockopt_reuse(int file_descriptor, int option_value)
{
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char *>(&option_value), sizeof(option_value)) == SOCKET_ERROR)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#else
static int setsockopt_reuse(int file_descriptor, int option_value)
{
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_REUSEADDR,
            &option_value, sizeof(option_value)) < 0)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#endif

#ifdef _WIN32
static int set_timeout_recv(int file_descriptor, int timeout_milliseconds)
{
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char *>(&timeout_milliseconds), sizeof(timeout_milliseconds)) == SOCKET_ERROR)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#else
static int set_timeout_recv(int file_descriptor, int timeout_milliseconds)
{
    struct timeval timeout_value;

    timeout_value.tv_sec = timeout_milliseconds / 1000;
    timeout_value.tv_usec = (timeout_milliseconds % 1000) * 1000;
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_RCVTIMEO,
            &timeout_value, sizeof(timeout_value)) < 0)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#endif

#ifdef _WIN32
static int set_timeout_send(int file_descriptor, int timeout_milliseconds)
{
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char *>(&timeout_milliseconds), sizeof(timeout_milliseconds)) == SOCKET_ERROR)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#else
static int set_timeout_send(int file_descriptor, int timeout_milliseconds)
{
    struct timeval timeout_value;

    timeout_value.tv_sec = timeout_milliseconds / 1000;
    timeout_value.tv_usec = (timeout_milliseconds % 1000) * 1000;
    if (setsockopt(file_descriptor, SOL_SOCKET, SO_SNDTIMEO,
            &timeout_value, sizeof(timeout_value)) < 0)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}
#endif

udp_socket::udp_socket()
    : _initialized_state(_state_uninitialized), _address(), _socket_fd(-1), _mutex()
{
    ft_bzero(&this->_address, sizeof(this->_address));
    return ;
}

udp_socket::~udp_socket()
{
    if (this->_initialized_state == _state_uninitialized)
    {
        pf_printf_fd(2, "udp_socket lifecycle error: %s\n",
            "destructor called on uninitialized instance");
        su_abort();
    }
    if (this->_initialized_state == _state_initialized)
    {
        (void)this->close_socket();
        (void)this->_mutex.destroy();
        this->_initialized_state = _state_destroyed;
    }
    return ;
}

void udp_socket::abort_lifecycle_error(const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "udp_socket lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void udp_socket::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == _state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int udp_socket::create_socket(const SocketConfig &config)
{
    this->_socket_fd = nw_socket(config._address_family, SOCK_DGRAM, config._protocol);
    if (this->_socket_fd < 0)
        return (FT_ERR_SOCKET_CREATION_FAILED);
    return (FT_ERR_SUCCESS);
}

int udp_socket::set_non_blocking(const SocketConfig &config)
{
    if (config._non_blocking == false)
        return (FT_ERR_SUCCESS);
    if (nw_set_nonblocking(this->_socket_fd) != 0)
        return (FT_ERR_CONFIGURATION);
    return (FT_ERR_SUCCESS);
}

int udp_socket::set_timeouts(const SocketConfig &config)
{
    if (config._recv_timeout > 0)
    {
        if (set_timeout_recv(this->_socket_fd, config._recv_timeout) != FT_ERR_SUCCESS)
            return (FT_ERR_CONFIGURATION);
    }
    if (config._send_timeout > 0)
    {
        if (set_timeout_send(this->_socket_fd, config._send_timeout) != FT_ERR_SUCCESS)
            return (FT_ERR_CONFIGURATION);
    }
    return (FT_ERR_SUCCESS);
}

int udp_socket::configure_address(const SocketConfig &config)
{
    struct addrinfo hints;
    struct addrinfo *address_info;
    char port_string[16];
    const char *host_value;

    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = config._address_family;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = config._protocol;
    if (config._type == SocketType::SERVER)
        hints.ai_flags = AI_PASSIVE;
    std::snprintf(port_string, sizeof(port_string), "%u", config._port);
    if (config._ip[0] == '\0')
        host_value = ft_nullptr;
    else
        host_value = config._ip;
    if (getaddrinfo(host_value, port_string, &hints, &address_info) != 0)
        return (FT_ERR_SOCKET_RESOLVE_FAILED);
    ft_memcpy(&this->_address, address_info->ai_addr, address_info->ai_addrlen);
    freeaddrinfo(address_info);
    return (FT_ERR_SUCCESS);
}

int udp_socket::bind_socket(const SocketConfig &config)
{
    int reuse_option;

    reuse_option = 1;
    if (config._type != SocketType::SERVER)
        return (FT_ERR_SUCCESS);
    if (setsockopt_reuse(this->_socket_fd, reuse_option) != FT_ERR_SUCCESS)
        return (FT_ERR_CONFIGURATION);
    if (nw_bind(this->_socket_fd, reinterpret_cast<const struct sockaddr *>(&this->_address),
            sizeof(this->_address)) < 0)
        return (FT_ERR_SOCKET_BIND_FAILED);
    return (FT_ERR_SUCCESS);
}

int udp_socket::connect_socket(const SocketConfig &config)
{
    if (config._type != SocketType::CLIENT)
        return (FT_ERR_SUCCESS);
    if (nw_connect(this->_socket_fd, reinterpret_cast<const struct sockaddr *>(&this->_address),
            sizeof(this->_address)) < 0)
        return (FT_ERR_SOCKET_CONNECT_FAILED);
    return (FT_ERR_SUCCESS);
}

int udp_socket::initialize(const SocketConfig &config)
{
    int lock_error;
    int step_error;

    if (this->_initialized_state == _state_initialized)
        this->abort_lifecycle_error("udp_socket::initialize",
            "initialize called on initialized instance");
    if (this->_initialized_state != _state_initialized)
    {
        if (this->_mutex.initialize() != FT_ERR_SUCCESS)
            return (FT_ERR_INITIALIZATION_FAILED);
        this->_initialized_state = _state_initialized;
    }
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_socket_fd >= 0)
    {
        (void)this->_mutex.unlock();
        return (FT_ERR_ALREADY_INITIALIZED);
    }
    step_error = this->create_socket(config);
    if (step_error == FT_ERR_SUCCESS)
        step_error = this->set_non_blocking(config);
    if (step_error == FT_ERR_SUCCESS)
        step_error = this->set_timeouts(config);
    if (step_error == FT_ERR_SUCCESS)
        step_error = this->configure_address(config);
    if (step_error == FT_ERR_SUCCESS)
        step_error = this->bind_socket(config);
    if (step_error == FT_ERR_SUCCESS)
        step_error = this->connect_socket(config);
    if (step_error != FT_ERR_SUCCESS)
    {
        if (this->_socket_fd >= 0)
        {
            (void)nw_close(this->_socket_fd);
            this->_socket_fd = -1;
        }
    }
    (void)this->_mutex.unlock();
    return (step_error);
}

ssize_t udp_socket::send_to(const void *data, size_t size, int flags,
    const struct sockaddr *destination_address, socklen_t address_length)
{
    int lock_error;
    ssize_t send_result;

    this->abort_if_not_initialized("udp_socket::send_to");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_socket_fd < 0)
    {
        (void)this->_mutex.unlock();
        return (-1);
    }
    send_result = nw_sendto(this->_socket_fd, data, size, flags, destination_address, address_length);
    (void)this->_mutex.unlock();
    return (send_result);
}

ssize_t udp_socket::receive_from(void *buffer, size_t size, int flags,
    struct sockaddr *source_address, socklen_t *address_length)
{
    int lock_error;
    ssize_t receive_result;

    this->abort_if_not_initialized("udp_socket::receive_from");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_socket_fd < 0)
    {
        (void)this->_mutex.unlock();
        return (-1);
    }
    receive_result = nw_recvfrom(this->_socket_fd, buffer, size, flags, source_address, address_length);
    (void)this->_mutex.unlock();
    return (receive_result);
}

bool udp_socket::close_socket()
{
    int lock_error;

    this->abort_if_not_initialized("udp_socket::close_socket");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    if (this->_socket_fd < 0)
    {
        (void)this->_mutex.unlock();
        return (true);
    }
    if (nw_close(this->_socket_fd) != 0)
    {
        (void)this->_mutex.unlock();
        return (false);
    }
    this->_socket_fd = -1;
    (void)this->_mutex.unlock();
    return (true);
}

int udp_socket::get_fd() const
{
    this->abort_if_not_initialized("udp_socket::get_fd");
    return (this->_socket_fd);
}

const struct sockaddr_storage &udp_socket::get_address() const
{
    this->abort_if_not_initialized("udp_socket::get_address");
    return (this->_address);
}
