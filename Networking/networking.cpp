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
    : _error_code(ER_SUCCESS),
      _type(SocketType::SERVER),
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
    this->set_error(ER_SUCCESS);
    if (this->_error_code == ER_SUCCESS && _ip.get_error())
        this->set_error(_ip.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_group.get_error())
        this->set_error(_multicast_group.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_interface.get_error())
        this->set_error(_multicast_interface.get_error());
    return ;
}

SocketConfig::SocketConfig(const SocketConfig& other) noexcept
    : _error_code(other._error_code),
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
    this->set_error(other._error_code);
    if (this->_error_code == ER_SUCCESS && _ip.get_error())
        this->set_error(_ip.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_group.get_error())
        this->set_error(_multicast_group.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_interface.get_error())
        this->set_error(_multicast_interface.get_error());
    return ;
}

SocketConfig& SocketConfig::operator=(const SocketConfig& other) noexcept
{
    if (this != &other)
    {
        this->set_error(other._error_code);
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
    if (this->_error_code == ER_SUCCESS && _ip.get_error())
        this->set_error(_ip.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_group.get_error())
        this->set_error(_multicast_group.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_interface.get_error())
        this->set_error(_multicast_interface.get_error());
    return (*this);
}

SocketConfig::SocketConfig(SocketConfig&& other) noexcept
    : _error_code(other._error_code),
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
    other.set_error(ER_SUCCESS);
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
    if (this->_error_code == ER_SUCCESS && _ip.get_error())
        this->set_error(_ip.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_group.get_error())
        this->set_error(_multicast_group.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_interface.get_error())
        this->set_error(_multicast_interface.get_error());
    return ;
}

SocketConfig& SocketConfig::operator=(SocketConfig&& other) noexcept
{
    if (this != &other)
    {
        this->set_error(other._error_code);
        this->_type = other._type;
        this->_ip.move(other._ip);
        this->_port = other._port;
        this->_backlog = other._backlog;
        this->_protocol = other._protocol;
        this->_address_family = other._address_family;
        this->_reuse_address = other._reuse_address;
        this->_non_blocking = other._non_blocking;
        this->_recv_timeout = other._recv_timeout;
        this->_send_timeout = other._send_timeout;
        this->_multicast_group.move(other._multicast_group);
        this->_multicast_interface.move(other._multicast_interface);
        other._type = SocketType::CLIENT;
        other._ip.clear();
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
        other.set_error(ER_SUCCESS);
    }
    if (this->_error_code == ER_SUCCESS && _ip.get_error())
        this->set_error(_ip.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_group.get_error())
        this->set_error(_multicast_group.get_error());
    if (this->_error_code == ER_SUCCESS && _multicast_interface.get_error())
        this->set_error(_multicast_interface.get_error());
    return (*this);
}

SocketConfig::~SocketConfig()
{
    return ;
}

int SocketConfig::get_error()
{
    return (this->_error_code);
}

const char *SocketConfig::get_error_str()
{
    return (ft_strerror(this->_error_code));
}

void SocketConfig::set_error(int error_code) noexcept
{
    ft_errno = error_code;
    this->_error_code = ft_errno;
    return ;
}
