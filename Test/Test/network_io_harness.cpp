#include "network_io_harness.hpp"
#include "../../Template/move.hpp"
#include "../../Libft/libft.hpp"
#include <cerrno>
#include <cstring>
#include <new>
#ifdef _WIN32
# include <windows.h>
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <unistd.h>
# include <fcntl.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif

network_io_harness::network_io_harness()
    : _error_code(ER_SUCCESS), _listener_socket(), _client_socket(), _accepted_fd(-1),
    _accepted_address(), _reader_thread(ft_nullptr), _stop_reader(0),
    _throttle_bytes(0), _throttle_delay_us(0), _reader_fd(-1)
{
    ft_bzero(&this->_accepted_address, sizeof(this->_accepted_address));
    this->set_error(ER_SUCCESS);
    return ;
}

network_io_harness::~network_io_harness()
{
    this->stop_throttled_reads();
    this->shutdown();
    return ;
}

void network_io_harness::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = ft_errno;
    return ;
}

int network_io_harness::configure_listener(uint16_t port)
{
    SocketConfig server_configuration;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = port;
    server_configuration._backlog = 1;
    server_configuration._protocol = 0;
    server_configuration._address_family = AF_INET;
    server_configuration._reuse_address = true;
    server_configuration._non_blocking = false;
    server_configuration._recv_timeout = 0;
    server_configuration._send_timeout = 0;
    ft_socket listener(server_configuration);
    if (listener.get_error() != ER_SUCCESS)
    {
        this->set_error(listener.get_error());
        return (this->_error_code);
    }
    this->_listener_socket = ft_move(listener);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int network_io_harness::connect_client(uint16_t port)
{
    SocketConfig client_configuration;

    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "127.0.0.1";
    client_configuration._port = port;
    client_configuration._protocol = 0;
    client_configuration._address_family = AF_INET;
    client_configuration._reuse_address = false;
    client_configuration._non_blocking = false;
    client_configuration._recv_timeout = 0;
    client_configuration._send_timeout = 0;
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
    {
        this->set_error(client.get_error());
        return (this->_error_code);
    }
    this->_client_socket = ft_move(client);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int network_io_harness::accept_client()
{
    socklen_t address_length;
    int accepted_fd;

    address_length = sizeof(this->_accepted_address);
    accepted_fd = nw_accept(this->_listener_socket.get_fd(),
        reinterpret_cast<struct sockaddr *>(&this->_accepted_address),
        &address_length);
    if (accepted_fd < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (this->_error_code);
    }
    this->_accepted_fd = accepted_fd;
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int network_io_harness::initialize(uint16_t port)
{
    uint16_t connection_port;

    this->stop_throttled_reads();
    this->shutdown();
    if (this->configure_listener(port) != ER_SUCCESS)
        return (this->_error_code);
    connection_port = port;
    if (connection_port == 0)
    {
        int listener_fd;
        int result;
        struct sockaddr_in address;
        socklen_t address_length;

        listener_fd = this->_listener_socket.get_fd();
        address_length = sizeof(address);
        result = getsockname(listener_fd, reinterpret_cast<struct sockaddr *>(&address), &address_length);
        if (result != 0)
        {
#ifdef _WIN32
            this->set_error(WSAGetLastError() + ERRNO_OFFSET);
#else
            this->set_error(errno + ERRNO_OFFSET);
#endif
            return (this->_error_code);
        }
        connection_port = ntohs(address.sin_port);
    }
    if (this->connect_client(connection_port) != ER_SUCCESS)
        return (this->_error_code);
    if (this->accept_client() != ER_SUCCESS)
        return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void network_io_harness::shutdown()
{
    if (this->_reader_thread != ft_nullptr)
    {
        this->stop_throttled_reads();
    }
    if (this->_accepted_fd >= 0)
    {
#ifdef _WIN32
        closesocket(static_cast<SOCKET>(this->_accepted_fd));
#else
        close(this->_accepted_fd);
#endif
        this->_accepted_fd = -1;
    }
    this->_client_socket.close_socket();
    this->_listener_socket.close_socket();
    this->set_error(ER_SUCCESS);
    return ;
}

int network_io_harness::get_client_fd() const
{
    return (this->_client_socket.get_fd());
}

int network_io_harness::get_server_fd() const
{
    if (this->_accepted_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (this->_accepted_fd);
}

ft_socket &network_io_harness::get_client_socket()
{
    this->set_error(ER_SUCCESS);
    return (this->_client_socket);
}

int network_io_harness::set_blocking_flag(int file_descriptor, bool should_block)
{
#ifdef _WIN32
    u_long mode;

    if (should_block)
        mode = 0;
    else
        mode = 1;
    if (ioctlsocket(static_cast<SOCKET>(file_descriptor), FIONBIO, &mode) != 0)
    {
        this->set_error(WSAGetLastError() + ERRNO_OFFSET);
        return (this->_error_code);
    }
#else
    int flags;

    flags = fcntl(file_descriptor, F_GETFL, 0);
    if (flags < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (this->_error_code);
    }
    if (should_block)
        flags = flags & ~O_NONBLOCK;
    else
        flags = flags | O_NONBLOCK;
    if (fcntl(file_descriptor, F_SETFL, flags) < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (this->_error_code);
    }
#endif
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int network_io_harness::set_blocking(bool should_block)
{
    int client_fd;
    int server_fd;

    client_fd = this->_client_socket.get_fd();
    server_fd = this->_accepted_fd;
    if (client_fd < 0 || server_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (this->_error_code);
    }
    if (this->set_blocking_flag(client_fd, should_block) != ER_SUCCESS)
        return (this->_error_code);
    if (this->set_blocking_flag(server_fd, should_block) != ER_SUCCESS)
        return (this->_error_code);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int network_io_harness::enable_non_blocking()
{
    return (this->set_blocking(false));
}

int network_io_harness::enable_blocking()
{
    return (this->set_blocking(true));
}

void network_io_harness::cleanup_reader()
{
    if (this->_reader_thread == ft_nullptr)
        return ;
    this->_stop_reader = 1;
    if (this->_reader_thread->joinable())
        this->_reader_thread->join();
    delete this->_reader_thread;
    this->_reader_thread = ft_nullptr;
    this->_stop_reader = 0;
    return ;
}

int network_io_harness::start_throttled_reads(size_t throttle_bytes, size_t delay_microseconds)
{
    int reader_fd;

    this->stop_throttled_reads();
    reader_fd = this->_client_socket.get_fd();
    if (reader_fd < 0)
    {
        this->set_error(FT_EINVAL);
        return (this->_error_code);
    }
    this->_throttle_bytes = throttle_bytes;
    this->_throttle_delay_us = delay_microseconds;
    this->_reader_fd = reader_fd;
    this->_stop_reader = 0;
    if (this->set_blocking_flag(reader_fd, false) != ER_SUCCESS)
        return (this->_error_code);
    this->_reader_thread = new (std::nothrow) ft_thread(&network_io_harness::reader_entry, this);
    if (this->_reader_thread == ft_nullptr)
    {
        this->set_error(FT_EALLOC);
        return (this->_error_code);
    }
    if (this->_reader_thread->get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reader_thread->get_error());
        delete this->_reader_thread;
        this->_reader_thread = ft_nullptr;
        return (this->_error_code);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void network_io_harness::reader_entry(network_io_harness *harness)
{
    if (harness == ft_nullptr)
        return ;
    harness->reader_loop();
    return ;
}

void network_io_harness::reader_loop()
{
    char buffer[1024];

    while (this->_stop_reader == 0)
    {
        size_t bytes_to_read;
        ssize_t read_result;

        if (this->_throttle_bytes == 0)
            bytes_to_read = sizeof(buffer);
        else if (this->_throttle_bytes < sizeof(buffer))
            bytes_to_read = this->_throttle_bytes;
        else
            bytes_to_read = sizeof(buffer);
        read_result = nw_recv(this->_reader_fd, buffer, bytes_to_read, 0);
        if (read_result > 0)
        {
            if (this->_throttle_delay_us > 0)
#ifdef _WIN32
                Sleep(static_cast<DWORD>(this->_throttle_delay_us / 1000));
#else
                usleep(this->_throttle_delay_us);
#endif
            continue ;
        }
        if (read_result == 0)
            break;
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error == WSAEWOULDBLOCK)
        {
            if (this->_throttle_delay_us > 0)
                Sleep(static_cast<DWORD>(this->_throttle_delay_us / 1000));
            continue ;
        }
        break;
#else
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            if (this->_throttle_delay_us > 0)
                usleep(this->_throttle_delay_us);
            continue ;
        }
        if (errno == EINTR)
            continue ;
        break;
#endif
    }
    return ;
}

void network_io_harness::stop_throttled_reads()
{
    this->cleanup_reader();
    if (this->_reader_fd >= 0)
    {
        int descriptor;

        descriptor = this->_reader_fd;
        if (this->set_blocking_flag(descriptor, true) != ER_SUCCESS)
            return ;
        this->_reader_fd = -1;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

void network_io_harness::close_client()
{
    this->stop_throttled_reads();
    this->_client_socket.close_socket();
    this->set_error(ER_SUCCESS);
    return ;
}

int network_io_harness::get_error() const
{
    return (this->_error_code);
}

const char *network_io_harness::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
