#include "networking.hpp"
#include "../Errno/errno.hpp"
#include <cstring>

static int networking_consume_global_error(void) noexcept
{
    int error_code;

    error_code = ft_global_error_stack_peek_last_error();
    ft_global_error_stack_drop_last_error();
    return (error_code);
}

static void networking_update_operation_result(int &operation_result) noexcept
{
    int error_code;

    error_code = networking_consume_global_error();
    if (error_code != FT_ERR_SUCCESS)
        operation_result = error_code;
    return ;
}

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

SocketConfig::SocketConfig()
    : _error_code(FT_ERR_SUCCESS),
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
    int operation_result;

    operation_result = FT_ERR_SUCCESS;
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    socket_config_prepare_thread_safety(this);
    networking_update_operation_result(operation_result);
    this->report_operation_result(operation_result);
    return ;
}

SocketConfig::SocketConfig(const SocketConfig& other) noexcept
    : _error_code(FT_ERR_SUCCESS),
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
    int           operation_result;

    mutable_other = const_cast<SocketConfig*>(&other);
    other_locked = false;
    operation_result = FT_ERR_SUCCESS;
    bool lock_failed = socket_config_lock(mutable_other, &other_locked) != 0;
    networking_update_operation_result(operation_result);
    if (!lock_failed)
    {
        operation_result = other._error_code;
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
        networking_consume_global_error();
    }
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    socket_config_prepare_thread_safety(this);
    networking_update_operation_result(operation_result);
    this->report_operation_result(operation_result);
    return ;
}

SocketConfig& SocketConfig::operator=(const SocketConfig& other) noexcept
{
    SocketConfig *mutable_other;
    SocketConfig *first;
    SocketConfig *second;
    bool          first_locked;
    bool          second_locked;
    bool          proceed;
    int           operation_result;

    mutable_other = const_cast<SocketConfig*>(&other);
    first = this;
    second = mutable_other;
    first_locked = false;
    second_locked = false;
    operation_result = this->_error_code;
    if (this != &other)
    {
        proceed = true;
        bool prepare_failed = socket_config_prepare_thread_safety(this) != 0;
        networking_update_operation_result(operation_result);
        if (prepare_failed)
            proceed = false;
        if (proceed && first > second)
        {
            first = mutable_other;
            second = this;
        }
        if (proceed)
        {
            bool first_lock_failed = socket_config_lock(first, &first_locked) != 0;
            networking_update_operation_result(operation_result);
            if (first_lock_failed)
                proceed = false;
        }
        if (proceed)
        {
            bool second_lock_failed = socket_config_lock(second, &second_locked) != 0;
            networking_update_operation_result(operation_result);
            if (second_lock_failed)
                proceed = false;
        }
        if (proceed)
        {
            operation_result = other._error_code;
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
        }
        if (second_locked)
        {
            socket_config_unlock(second, second_locked);
            networking_consume_global_error();
        }
        if (first_locked)
        {
            socket_config_unlock(first, first_locked);
            networking_consume_global_error();
        }
    }
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    socket_config_prepare_thread_safety(this);
    networking_update_operation_result(operation_result);
    this->report_operation_result(operation_result);
    return (*this);
}

SocketConfig::SocketConfig(SocketConfig&& other) noexcept
    : _error_code(FT_ERR_SUCCESS),
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
    int  operation_result;

    other_locked = false;
    operation_result = FT_ERR_SUCCESS;
    bool lock_failed = socket_config_lock(&other, &other_locked) != 0;
    networking_update_operation_result(operation_result);
    if (!lock_failed)
    {
        operation_result = other._error_code;
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
        other.report_operation_result(FT_ERR_SUCCESS);
        socket_config_unlock(&other, other_locked);
        networking_consume_global_error();
    }
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    socket_config_prepare_thread_safety(this);
    networking_update_operation_result(operation_result);
    this->report_operation_result(operation_result);
    return ;
}

SocketConfig& SocketConfig::operator=(SocketConfig&& other) noexcept
{
    SocketConfig *first;
    SocketConfig *second;
    bool          first_locked;
    bool          second_locked;
    bool          proceed;
    int           operation_result;

    first = this;
    second = &other;
    first_locked = false;
    second_locked = false;
    operation_result = this->_error_code;
    if (this != &other)
    {
        proceed = true;
        bool prepare_failed = socket_config_prepare_thread_safety(this) != 0;
        networking_update_operation_result(operation_result);
        if (prepare_failed)
            proceed = false;
        if (proceed && first > second)
        {
            first = &other;
            second = this;
        }
        if (proceed)
        {
            bool first_lock_failed = socket_config_lock(first, &first_locked) != 0;
            networking_update_operation_result(operation_result);
            if (first_lock_failed)
                proceed = false;
        }
        if (proceed)
        {
            bool second_lock_failed = socket_config_lock(second, &second_locked) != 0;
            networking_update_operation_result(operation_result);
            if (second_lock_failed)
                proceed = false;
        }
        if (proceed)
        {
            operation_result = other._error_code;
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
            other.report_operation_result(FT_ERR_SUCCESS);
        }
        if (second_locked)
        {
            socket_config_unlock(second, second_locked);
            networking_consume_global_error();
        }
        if (first_locked)
        {
            socket_config_unlock(first, first_locked);
            networking_consume_global_error();
        }
    }
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    if (operation_result == FT_ERR_SUCCESS
        && ft_string::last_operation_error() != FT_ERR_SUCCESS)
        operation_result = ft_string::last_operation_error();
    socket_config_prepare_thread_safety(this);
    networking_update_operation_result(operation_result);
    this->report_operation_result(operation_result);
    return (*this);
}

SocketConfig::~SocketConfig()
{
    socket_config_teardown_thread_safety(this);
    return ;
}

void SocketConfig::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return ;
}

void SocketConfig::report_operation_result(int error_code) const noexcept
{
    this->_error_code = error_code;
    this->record_operation_error(error_code);
    return ;
}
