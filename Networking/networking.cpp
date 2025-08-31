#include "networking.hpp"
#include "../Errno/errno.hpp"
#include <cstring>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

SocketConfig::SocketConfig()
    : _type(SocketType::SERVER),
      _ip("127.0.0.1"),
      _port(8080),
      _backlog(10),
      _protocol(IPPROTO_TCP),
      _address_family(AF_INET),
      _reuse_address(true),
      _non_blocking(false),
      _recv_timeout(5000),
      _send_timeout(5000),
      _multicast_group(""),
      _multicast_interface("")
{
    if (!_error && _ip.get_error())
        _error = _ip.get_error();
    if (!_error && _multicast_group.get_error())
        _error = _multicast_group.get_error();
    if (!_error && _multicast_interface.get_error())
        _error = _multicast_interface.get_error();
    return ;
}

SocketConfig::SocketConfig(const SocketConfig& other) noexcept
    : _error(other._error),
      _type(other._type),
      _ip(other._ip),
      _port(other._port),
      _backlog(other._backlog),
      _protocol(other._protocol),
      _address_family(other._address_family),
      _reuse_address(other._reuse_address),
      _non_blocking(other._non_blocking),
      _recv_timeout(other._recv_timeout),
      _send_timeout(other._send_timeout),
      _multicast_group(other._multicast_group),
      _multicast_interface(other._multicast_interface)
{
    if (!_error && _ip.get_error())
        _error = _ip.get_error();
    if (!_error && _multicast_group.get_error())
        _error = _multicast_group.get_error();
    if (!_error && _multicast_interface.get_error())
        _error = _multicast_interface.get_error();
    return ;
}

SocketConfig& SocketConfig::operator=(const SocketConfig& other) noexcept
{
    if (this != &other)
    {
        _error = other._error;
        _type = other._type;
        _ip = other._ip;
        _port = other._port;
        _backlog = other._backlog;
        _protocol = other._protocol;
        _address_family = other._address_family;
        _reuse_address = other._reuse_address;
        _non_blocking = other._non_blocking;
        _recv_timeout = other._recv_timeout;
        _send_timeout = other._send_timeout;
        _multicast_group = other._multicast_group;
        _multicast_interface = other._multicast_interface;
    }
    if (!_error && _ip.get_error())
        _error = _ip.get_error();
    if (!_error && _multicast_group.get_error())
        _error = _multicast_group.get_error();
    if (!_error && _multicast_interface.get_error())
        _error = _multicast_interface.get_error();
    return (*this);
}

SocketConfig::SocketConfig(SocketConfig&& other) noexcept
    : _error(other._error),
      _type(other._type),
      _ip(other._ip),
      _port(other._port),
      _backlog(other._backlog),
      _protocol(other._protocol),
      _address_family(other._address_family),
      _reuse_address(other._reuse_address),
      _non_blocking(other._non_blocking),
      _recv_timeout(other._recv_timeout),
      _send_timeout(other._send_timeout),
      _multicast_group(other._multicast_group),
      _multicast_interface(other._multicast_interface)
{
    other._error = 0;
    other._type = SocketType::CLIENT;
    other._port = 0;
    other._backlog = 0;
    other._protocol = 0;
    other._address_family = 0;
    other._reuse_address = false;
    other._non_blocking = false;
    other._recv_timeout = 0;
    other._send_timeout = 0;
    other._multicast_group.clear();
    other._multicast_interface.clear();
    if (!_error && _ip.get_error())
        _error = _ip.get_error();
    if (!_error && _multicast_group.get_error())
        _error = _multicast_group.get_error();
    if (!_error && _multicast_interface.get_error())
        _error = _multicast_interface.get_error();
    return ;
}

SocketConfig& SocketConfig::operator=(SocketConfig&& other) noexcept
{
    if (this != &other)
    {
        _error = other._error;
        _type = other._type;
        other._ip = this->_ip;
        _port = other._port;
        _backlog = other._backlog;
        _protocol = other._protocol;
        _address_family = other._address_family;
        _reuse_address = other._reuse_address;
        _non_blocking = other._non_blocking;
        _recv_timeout = other._recv_timeout;
        _send_timeout = other._send_timeout;
        other._multicast_group = this->_multicast_group;
        other._multicast_interface = this->_multicast_interface;
        other._error = 0;
        other._type = SocketType::CLIENT;
        other._port = 0;
        other._backlog = 0;
        other._protocol = 0;
        other._address_family = 0;
        other._reuse_address = false;
        other._non_blocking = false;
        other._recv_timeout = 0;
        other._send_timeout = 0;
        other._multicast_group.clear();
        other._multicast_interface.clear();
    }
    if (!_error && _ip.get_error())
        _error = _ip.get_error();
    if (!_error && _multicast_group.get_error())
        _error = _multicast_group.get_error();
    if (!_error && _multicast_interface.get_error())
        _error = _multicast_interface.get_error();
    return (*this);
}

SocketConfig::~SocketConfig()
{
    return ;
}

int SocketConfig::get_error()
{
    return (_error);
}

const char *SocketConfig::get_error_str()
{
    return (ft_strerror(_error));
}
