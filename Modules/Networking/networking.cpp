#include "networking.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

#include <cstring>

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

SocketConfig::SocketConfig() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _type(SocketType::SERVER),
      _ip(),
      _port(0),
      _backlog(0),
      _protocol(IPPROTO_TCP),
      _address_family(AF_INET),
      _reuse_address(FT_FALSE),
      _non_blocking(FT_FALSE),
      _recv_timeout(0),
      _send_timeout(0),
      _multicast_group(),
      _multicast_interface()
{
    return ;
}

int32_t SocketConfig::move(SocketConfig &other) noexcept
{
    return (this->initialize(static_cast<SocketConfig &&>(other)));
}

int32_t SocketConfig::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "SocketConfig::initialize",
            "initialize called on initialised instance");
    this->_type = SocketType::SERVER;
    ft_strncpy(this->_ip, "127.0.0.1", sizeof(this->_ip) - 1);
    this->_ip[sizeof(this->_ip) - 1] = '\0';
    this->_port = 8080;
    this->_backlog = 10;
    this->_protocol = IPPROTO_TCP;
    this->_address_family = AF_INET;
    this->_reuse_address = FT_TRUE;
    this->_non_blocking = FT_FALSE;
    this->_recv_timeout = 5000;
    this->_send_timeout = 5000;
    this->_multicast_group[0] = '\0';
    this->_multicast_interface[0] = '\0';
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t SocketConfig::initialize(const SocketConfig &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "SocketConfig::initialize(const SocketConfig &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_type = other._type;
    ft_strncpy(this->_ip, other._ip, sizeof(this->_ip) - 1);
    this->_ip[sizeof(this->_ip) - 1] = '\0';
    this->_port = other._port;
    this->_backlog = other._backlog;
    this->_protocol = other._protocol;
    this->_address_family = other._address_family;
    this->_reuse_address = other._reuse_address;
    this->_non_blocking = other._non_blocking;
    this->_recv_timeout = other._recv_timeout;
    this->_send_timeout = other._send_timeout;
    ft_strncpy(this->_multicast_group, other._multicast_group, sizeof(this->_multicast_group) - 1);
    this->_multicast_group[sizeof(this->_multicast_group) - 1] = '\0';
    ft_strncpy(this->_multicast_interface, other._multicast_interface, sizeof(this->_multicast_interface) - 1);
    this->_multicast_interface[sizeof(this->_multicast_interface) - 1] = '\0';
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t SocketConfig::initialize(SocketConfig &&other) noexcept
{
    int32_t initialize_error;

    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t SocketConfig::destroy() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    this->_ip[0] = '\0';
    this->_port = 0;
    this->_backlog = 0;
    this->_protocol = 0;
    this->_address_family = 0;
    this->_reuse_address = FT_FALSE;
    this->_non_blocking = FT_FALSE;
    this->_recv_timeout = 0;
    this->_send_timeout = 0;
    this->_multicast_group[0] = '\0';
    this->_multicast_interface[0] = '\0';
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

SocketConfig::~SocketConfig() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}
