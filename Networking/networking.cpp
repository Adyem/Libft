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
    : _error_code(FT_ERR_SUCCESSS),
      _thread_safe_enabled(false),
      _mutex(ft_nullptr),
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
    this->set_error(FT_ERR_SUCCESSS);
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (socket_config_prepare_thread_safety(this) != 0)
        this->set_error(ft_errno);
    return ;
}

SocketConfig::SocketConfig(const SocketConfig& other) noexcept
    : _error_code(FT_ERR_SUCCESSS),
      _thread_safe_enabled(false),
      _mutex(ft_nullptr),
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
    bool          other_locked;
    SocketConfig *mutable_other;

    mutable_other = const_cast<SocketConfig*>(&other);
    other_locked = false;
    if (socket_config_lock(mutable_other, &other_locked) != 0)
        this->set_error(ft_errno);
    else
    {
        this->set_error(other._error_code);
        this->_type = other._type;
        this->_ip = other._ip;
        this->_port = other._port;
        this->_backlog = other._backlog;
        this->_protocol = other._protocol;
        this->_address_family = other._address_family;
        this->_reuse_address = other._reuse_address;
        this->_non_blocking = other._non_blocking;
        this->_recv_timeout = other._recv_timeout;
        this->_send_timeout = other._send_timeout;
        this->_multicast_group = other._multicast_group;
        this->_multicast_interface = other._multicast_interface;
        socket_config_unlock(mutable_other, other_locked);
    }
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (socket_config_prepare_thread_safety(this) != 0)
        this->set_error(ft_errno);
    return ;
}

SocketConfig& SocketConfig::operator=(const SocketConfig& other) noexcept
{
    if (this != &other)
    {
        SocketConfig *mutable_other;
        SocketConfig *first;
        SocketConfig *second;
        bool          first_locked;
        bool          second_locked;

        mutable_other = const_cast<SocketConfig*>(&other);
        first = this;
        second = mutable_other;
        first_locked = false;
        second_locked = false;
        if (socket_config_prepare_thread_safety(this) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        if (first > second)
        {
            first = mutable_other;
            second = this;
        }
        if (socket_config_lock(first, &first_locked) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        if (socket_config_lock(second, &second_locked) != 0)
        {
            socket_config_unlock(first, first_locked);
            this->set_error(ft_errno);
            return (*this);
        }
        this->set_error(other._error_code);
        this->_type = other._type;
        this->_ip = other._ip;
        this->_port = other._port;
        this->_backlog = other._backlog;
        this->_protocol = other._protocol;
        this->_address_family = other._address_family;
        this->_reuse_address = other._reuse_address;
        this->_non_blocking = other._non_blocking;
        this->_recv_timeout = other._recv_timeout;
        this->_send_timeout = other._send_timeout;
        this->_multicast_group = other._multicast_group;
        this->_multicast_interface = other._multicast_interface;
        socket_config_unlock(second, second_locked);
        socket_config_unlock(first, first_locked);
    }
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (socket_config_prepare_thread_safety(this) != 0)
        this->set_error(ft_errno);
    return (*this);
}

SocketConfig::SocketConfig(SocketConfig&& other) noexcept
    : _error_code(FT_ERR_SUCCESSS),
      _thread_safe_enabled(false),
      _mutex(ft_nullptr),
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
    bool other_locked;

    other_locked = false;
    if (socket_config_lock(&other, &other_locked) != 0)
        this->set_error(ft_errno);
    else
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
        other.set_error(FT_ERR_SUCCESSS);
        socket_config_unlock(&other, other_locked);
    }
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (socket_config_prepare_thread_safety(this) != 0)
        this->set_error(ft_errno);
    return ;
}

SocketConfig& SocketConfig::operator=(SocketConfig&& other) noexcept
{
    if (this != &other)
    {
        SocketConfig *first;
        SocketConfig *second;
        bool          first_locked;
        bool          second_locked;

        first = this;
        second = &other;
        first_locked = false;
        second_locked = false;
        if (socket_config_prepare_thread_safety(this) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        if (first > second)
        {
            first = &other;
            second = this;
        }
        if (socket_config_lock(first, &first_locked) != 0)
        {
            this->set_error(ft_errno);
            return (*this);
        }
        if (socket_config_lock(second, &second_locked) != 0)
        {
            socket_config_unlock(first, first_locked);
            this->set_error(ft_errno);
            return (*this);
        }
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
        other.set_error(FT_ERR_SUCCESSS);
        socket_config_unlock(second, second_locked);
        socket_config_unlock(first, first_locked);
    }
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (this->_error_code == FT_ERR_SUCCESSS
        && ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        this->set_error(ft_string::last_operation_error());
    if (socket_config_prepare_thread_safety(this) != 0)
        this->set_error(ft_errno);
    return (*this);
}

SocketConfig::~SocketConfig()
{
    socket_config_teardown_thread_safety(this);
    return ;
}

int SocketConfig::get_error()
{
    bool lock_acquired;
    int  error_code;

    lock_acquired = false;
    if (socket_config_lock(this, &lock_acquired) != 0)
        return (ft_errno);
    error_code = this->_error_code;
    socket_config_unlock(this, lock_acquired);
    return (error_code);
}

const char *SocketConfig::get_error_str()
{
    bool        lock_acquired;
    const char *message;

    lock_acquired = false;
    if (socket_config_lock(this, &lock_acquired) != 0)
        return (ft_strerror(ft_errno));
    message = ft_strerror(this->_error_code);
    socket_config_unlock(this, lock_acquired);
    return (message);
}

void SocketConfig::set_error(int error_code) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (socket_config_lock(this, &lock_acquired) != 0)
    {
        ft_errno = error_code;
        this->_error_code = ft_errno;
        return ;
    }
    ft_errno = error_code;
    this->_error_code = ft_errno;
    socket_config_unlock(this, lock_acquired);
    return ;
}
