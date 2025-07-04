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
    : type(SocketType::SERVER),
      ip("127.0.0.1"),
      port(8080),
      backlog(10),
      protocol(IPPROTO_TCP),
      address_family(AF_INET),
      reuse_address(true),
      non_blocking(false),
      recv_timeout(5000),
      send_timeout(5000),
      multicast_group(""),
      multicast_interface("")
{
    if (!_error && ip.get_error())
        _error = ip.get_error();
    if (!_error && multicast_group.get_error())
        _error = multicast_group.get_error();
    if (!_error && multicast_interface.get_error())
        _error = multicast_interface.get_error();
    return ;
}

SocketConfig::SocketConfig(const SocketConfig& other) noexcept
    : _error(other._error),
      type(other.type),
      ip(other.ip),
      port(other.port),
      backlog(other.backlog),
      protocol(other.protocol),
      address_family(other.address_family),
      reuse_address(other.reuse_address),
      non_blocking(other.non_blocking),
      recv_timeout(other.recv_timeout),
      send_timeout(other.send_timeout),
      multicast_group(other.multicast_group),
      multicast_interface(other.multicast_interface)
{
	if (!_error && ip.get_error())
        _error = ip.get_error();
	if (!_error && multicast_group.get_error())
        _error = multicast_group.get_error();
	if (!_error && multicast_interface.get_error())
        _error = multicast_interface.get_error();
	return ;
}

SocketConfig& SocketConfig::operator=(const SocketConfig& other) noexcept
{
    if (this != &other)
    {
        _error = other._error;
        type = other.type;
        ip = other.ip;
        port = other.port;
        backlog = other.backlog;
        protocol = other.protocol;
        address_family = other.address_family;
        reuse_address = other.reuse_address;
        non_blocking = other.non_blocking;
        recv_timeout = other.recv_timeout;
        send_timeout = other.send_timeout;
        multicast_group = other.multicast_group;
        multicast_interface = other.multicast_interface;
    }
    if (!_error && ip.get_error())
        _error = ip.get_error();
    if (!_error && multicast_group.get_error())
        _error = multicast_group.get_error();
    if (!_error && multicast_interface.get_error())
        _error = multicast_interface.get_error();
    return (*this);
}

SocketConfig::SocketConfig(SocketConfig&& other) noexcept
    : _error(other._error),
      type(other.type),
      ip(other.ip),
      port(other.port),
      backlog(other.backlog),
      protocol(other.protocol),
      address_family(other.address_family),
      reuse_address(other.reuse_address),
      non_blocking(other.non_blocking),
      recv_timeout(other.recv_timeout),
      send_timeout(other.send_timeout),
      multicast_group(other.multicast_group),
      multicast_interface(other.multicast_interface)
{
    other._error = 0;
    other.type = SocketType::CLIENT;
    other.port = 0;
    other.backlog = 0;
    other.protocol = 0;
    other.address_family = 0;
    other.reuse_address = false;
    other.non_blocking = false;
    other.recv_timeout = 0;
    other.send_timeout = 0;
    other.multicast_group.clear();
    other.multicast_interface.clear();
	if (!_error && ip.get_error())
        _error = ip.get_error();
	if (!_error && multicast_group.get_error())
        _error = multicast_group.get_error();
	if (!_error && multicast_interface.get_error())
        _error = multicast_interface.get_error();
	return ;
}

SocketConfig& SocketConfig::operator=(SocketConfig&& other) noexcept
{
    if (this != &other)
    {
        _error = other._error;
        type = other.type;
        other.ip = this->ip;
        port = other.port;
        backlog = other.backlog;
        protocol = other.protocol;
        address_family = other.address_family;
        reuse_address = other.reuse_address;
        non_blocking = other.non_blocking;
        recv_timeout = other.recv_timeout;
        send_timeout = other.send_timeout;
        other.multicast_group = this->multicast_group;
        other.multicast_interface = this->multicast_interface;
        other._error = 0;
        other.type = SocketType::CLIENT;
        other.port = 0;
        other.backlog = 0;
        other.protocol = 0;
        other.address_family = 0;
        other.reuse_address = false;
        other.non_blocking = false;
        other.recv_timeout = 0;
        other.send_timeout = 0;
        other.multicast_group.clear();
        other.multicast_interface.clear();
    }
	if (!_error && ip.get_error())
        _error = ip.get_error();
    if (!_error && multicast_group.get_error())
        _error = multicast_group.get_error();
    if (!_error && multicast_interface.get_error())
        _error = multicast_interface.get_error();
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
