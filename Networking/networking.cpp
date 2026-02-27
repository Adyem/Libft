#include "networking.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

#include <cstring>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

SocketConfig::SocketConfig()
    : _initialized_state(SocketConfig::_state_uninitialized),
      _type(SocketType::SERVER),
      _ip(),
      _port(0),
      _backlog(0),
      _protocol(0),
      _address_family(0),
      _reuse_address(false),
      _non_blocking(false),
      _recv_timeout(0),
      _send_timeout(0),
      _multicast_group(),
      _multicast_interface()
{
    return ;
}

void SocketConfig::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "SocketConfig lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void SocketConfig::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == SocketConfig::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int SocketConfig::initialize() noexcept
{
    if (this->_initialized_state == SocketConfig::_state_initialized)
        this->abort_lifecycle_error("SocketConfig::initialize",
            "initialize called on initialized instance");
    this->_type = SocketType::SERVER;
    std::strncpy(this->_ip, "127.0.0.1", sizeof(this->_ip) - 1);
    this->_ip[sizeof(this->_ip) - 1] = '\0';
    this->_port = 8080;
    this->_backlog = 10;
    this->_protocol = IPPROTO_TCP;
    this->_address_family = AF_INET;
    this->_reuse_address = true;
    this->_non_blocking = false;
    this->_recv_timeout = 5000;
    this->_send_timeout = 5000;
    this->_multicast_group[0] = '\0';
    this->_multicast_interface[0] = '\0';
    this->_initialized_state = SocketConfig::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int SocketConfig::destroy() noexcept
{
    if (this->_initialized_state != SocketConfig::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_ip[0] = '\0';
    this->_port = 0;
    this->_backlog = 0;
    this->_protocol = 0;
    this->_address_family = 0;
    this->_reuse_address = false;
    this->_non_blocking = false;
    this->_recv_timeout = 0;
    this->_send_timeout = 0;
    this->_multicast_group[0] = '\0';
    this->_multicast_interface[0] = '\0';
    this->_initialized_state = SocketConfig::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

SocketConfig::~SocketConfig()
{
    if (this->_initialized_state == SocketConfig::_state_initialized)
        (void)this->destroy();
    return ;
}
