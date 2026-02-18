#include "socket_class.hpp"
#include "networking.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <thread>
#include <chrono>
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <arpa/inet.h>
# include <sys/types.h>
# include <unistd.h>
# include <sys/socket.h>
#endif

void ft_socket::sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

struct networking_error_entry
{
    int                    error_code;
};

static networking_error_entry networking_consume_last_error(void) noexcept
{
    networking_error_entry entry;

    entry.error_code = FT_ERR_SUCCESS;
    return (entry);
}


ssize_t ft_socket::send_data_locked(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_sent;

    bytes_sent = nw_send(this->_socket_fd, data, size, flags);
    return (bytes_sent);
}

ssize_t ft_socket::send_all_locked(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_CONFIGURATION);
        return (-1);
    }
    size_t total_sent;
    const char *buffer;

    total_sent = 0;
    buffer = static_cast<const char *>(data);
    while (total_sent < size)
    {
        ssize_t bytes_sent;

        bytes_sent = nw_send(this->_socket_fd, buffer + total_sent,
                size - total_sent, flags);
        if (bytes_sent < 0)
        {
#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEWOULDBLOCK || last_error == WSAEINTR)
            {
                ft_socket::sleep_backoff();
                int check_result = networking_check_socket_after_send(this->_socket_fd);
                if (check_result != 0)
                {
                    networking_error_entry entry = networking_consume_last_error();
                    (void)(entry.error_code);
                    return (-1);
                }
                networking_consume_last_error();
                continue ;
            }
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
            {
                ft_socket::sleep_backoff();
                int check_result = networking_check_socket_after_send(this->_socket_fd);
                if (check_result != 0)
                {
                    networking_error_entry entry = networking_consume_last_error();
                    (void)(entry.error_code);
                    return (-1);
                }
                networking_consume_last_error();
                continue ;
            }
#endif
            return (-1);
        }
        if (bytes_sent == 0)
        {
            if (networking_check_socket_after_send(this->_socket_fd) != 0)
            {
                networking_error_entry entry = networking_consume_last_error();
                (void)(entry.error_code);
                return (-1);
            }
            (void)(FT_ERR_SOCKET_SEND_FAILED);
            return (-1);
        }
        total_sent += bytes_sent;
    }
    if (networking_check_socket_after_send(this->_socket_fd) != 0)
    {
        networking_error_entry entry = networking_consume_last_error();
        (void)(entry.error_code);
        return (-1);
    }
    networking_consume_last_error();
    (void)(FT_ERR_SUCCESS);
    return (static_cast<ssize_t>(total_sent));
}

ssize_t ft_socket::receive_data_locked(void *buffer, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ssize_t bytes_received;

    bytes_received = nw_recv(this->_socket_fd, buffer, size, flags);
    return (bytes_received);
}

bool ft_socket::close_socket_locked()
{
    if (this->_socket_fd >= 0)
    {
        if (nw_close(this->_socket_fd) == 0)
        {
            this->_socket_fd = -1;
            (void)(FT_ERR_SUCCESS);
            return (true);
        }
        return (false);
    }
    (void)(FT_ERR_SUCCESS);
    return (true);
}

void ft_socket::reset_to_empty_state_locked()
{
    size_t connection_index;

    connection_index = 0;
    while (connection_index < this->_connected.size())
    {
        int client_fd;

        client_fd = this->_connected[connection_index];
        if (client_fd >= 0)
            (void)nw_close(client_fd);
        connection_index++;
    }
    this->_connected.clear();
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_socket_fd = -1;
    (void)(FT_ERR_SUCCESS);
    return ;
}

ft_socket::ft_socket()
    : _initialized_state(ft_socket::_state_uninitialized), _address(), _connected(), _socket_fd(-1), _mutex()
{
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    return ;
}

ft_socket::ft_socket(int fd, const sockaddr_storage &addr)
    : _initialized_state(ft_socket::_state_uninitialized), _address(), _connected(), _socket_fd(-1), _mutex()
{
    this->_address = addr;
    this->_connected.clear();
    this->_socket_fd = fd;
    return ;
}

ft_socket::ft_socket(const SocketConfig &)
    : _initialized_state(ft_socket::_state_uninitialized), _address(), _connected(), _socket_fd(-1), _mutex()
{
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    return ;
}

ft_socket::~ft_socket()
{
    if (this->_initialized_state == ft_socket::_state_uninitialized)
    {
        pf_printf_fd(2, "ft_socket lifecycle error: %s\n",
            "destructor called on uninitialized instance");
        su_abort();
    }
    if (this->_initialized_state == ft_socket::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_socket::abort_lifecycle_error(const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_socket lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_socket::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_socket::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags)
{
    ssize_t bytes_sent;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::send_data");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    bytes_sent = this->send_data_locked(data, size, flags);
    (void)this->_mutex.unlock();
    return (bytes_sent);
}

ssize_t ft_socket::send_all(const void *data, size_t size, int flags)
{
    ssize_t result;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::send_all");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = this->send_all_locked(data, size, flags);
    (void)this->_mutex.unlock();
    return (result);
}

ssize_t ft_socket::receive_data(void *buffer, size_t size, int flags)
{
    int socket_fd;
    ssize_t result;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::receive_data");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        (void)this->_mutex.unlock();
        return (-1);
    }
    socket_fd = this->_socket_fd;
    (void)this->_mutex.unlock();
    result = nw_recv(socket_fd, buffer, size, flags);
    lock_error = this->_mutex.lock();
    if (lock_error == FT_ERR_SUCCESS)
        (void)this->_mutex.unlock();
    return (result);
}

bool ft_socket::close_socket()
{
    int socket_fd;
    bool closed;
    bool shutdown_success;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::close_socket");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_SUCCESS);
        (void)this->_mutex.unlock();
        return (true);
    }
    socket_fd = this->_socket_fd;
    shutdown_success = true;
    (void)this->_mutex.unlock();
#ifdef _WIN32
    if (nw_shutdown(socket_fd, SD_BOTH) != 0)
#else
    if (nw_shutdown(socket_fd, SHUT_RDWR) != 0)
#endif
    {
        shutdown_success = false;
    }
    if (nw_close(socket_fd) == 0)
    {
        closed = true;
        networking_consume_last_error();
    }
    else
    {
        closed = false;
        networking_consume_last_error();
    }
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (closed);
    if (closed)
    {
        if (this->_socket_fd == socket_fd)
            this->_socket_fd = -1;
    }
    (void)this->_mutex.unlock();
    if (closed && shutdown_success)
        return (true);
    return (false);
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags, int fd)
{
    size_t index;
    bool found;
    ssize_t result;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::send_data(fd)");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    index = 0;
    found = false;
    result = -1;
    while (index < this->_connected.size())
    {
        if (this->_connected[index] == fd)
        {
            result = nw_send(fd, data, size, flags);
            found = true;
            break;
        }
        index++;
    }
    if (!found)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        result = -1;
    }
    (void)this->_mutex.unlock();
    return (result);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags, int exception)
{
    ssize_t total_bytes_sent;
    bool send_failed;
    size_t index;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::broadcast_data");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    total_bytes_sent = 0;
    send_failed = false;
    index = 0;
    while (index < this->_connected.size())
    {
        int client_fd;
        ssize_t bytes_sent;

        client_fd = this->_connected[index];
        if (exception == client_fd)
        {
            index++;
            continue ;
        }
        bytes_sent = nw_send(client_fd, data, size, flags);
        if (bytes_sent < 0)
        {
            send_failed = true;
        }
        else
            total_bytes_sent += bytes_sent;
        index++;
    }
    if (!send_failed)
        (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (total_bytes_sent);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags)
{
    return (this->broadcast_data(data, size, flags, -1));
}

int ft_socket::accept_connection()
{
    int lock_error;

    this->abort_if_not_initialized("ft_socket::accept_connection");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (this->_socket_fd < 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        (void)this->_mutex.unlock();
        return (-1);
    }
    int new_fd;

    new_fd = nw_accept(this->_socket_fd, ft_nullptr, ft_nullptr);
    if (new_fd < 0)
    {
        (void)this->_mutex.unlock();
        return (-1);
    }
    size_t previous_size;
    size_t current_size;

    previous_size = this->_connected.size();
    this->_connected.push_back(new_fd);
    current_size = this->_connected.size();
    if (current_size != previous_size + 1)
    {
        (void)nw_close(new_fd);
        (void)this->_mutex.unlock();
        return (-1);
    }
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (new_fd);
}

bool ft_socket::disconnect_client(int fd)
{
    size_t index;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::disconnect_client");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    index = 0;
    while (index < this->_connected.size())
    {
        if (this->_connected[index] == fd)
        {
            size_t last;

            last = this->_connected.size() - 1;
            if (index != last)
                this->_connected[index] = this->_connected[last];
            if (fd >= 0)
                (void)nw_close(fd);
            this->_connected.pop_back();
            (void)(FT_ERR_SUCCESS);
            (void)this->_mutex.unlock();
            return (true);
        }
        index++;
    }
    (void)(FT_ERR_INVALID_ARGUMENT);
    (void)this->_mutex.unlock();
    return (false);
}

void ft_socket::disconnect_all_clients()
{
    size_t index;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::disconnect_all_clients");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    index = 0;
    while (index < this->_connected.size())
    {
        if (this->_connected[index] >= 0)
            (void)nw_close(this->_connected[index]);
        index++;
    }
    this->_connected.clear();
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return ;
}

size_t ft_socket::get_client_count() const
{
    size_t count;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::get_client_count");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    count = this->_connected.size();
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (count);
}

bool ft_socket::is_client_connected(int fd) const
{
    size_t index;
    bool connected;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::is_client_connected");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    index = 0;
    connected = false;
    while (index < this->_connected.size())
    {
        if (this->_connected[index] == fd)
        {
            connected = true;
            break;
        }
        index++;
    }
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (connected);
}

int ft_socket::get_fd() const
{
    int descriptor;
    int lock_error;

    this->abort_if_not_initialized("ft_socket::get_fd");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    descriptor = this->_socket_fd;
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (descriptor);
}

const struct sockaddr_storage &ft_socket::get_address() const
{
    int lock_error;

    this->abort_if_not_initialized("ft_socket::get_address");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (this->_address);
    (void)(FT_ERR_SUCCESS);
    (void)this->_mutex.unlock();
    return (this->_address);
}

void ft_socket::reset_to_empty_state()
{
    int lock_error;

    this->abort_if_not_initialized("ft_socket::reset_to_empty_state");
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->reset_to_empty_state_locked();
    (void)this->_mutex.unlock();
    return ;
}

int ft_socket::initialize(const SocketConfig &config)
{
    int lock_error;
    int initialize_result;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (this->_initialized_state == ft_socket::_state_initialized)
        this->abort_lifecycle_error("ft_socket::initialize",
            "initialize called on initialized instance");
    this->_initialized_state = ft_socket::_state_initialized;
    (void)this->_mutex.unlock();
    initialize_result = FT_ERR_UNSUPPORTED_TYPE;
    if (config._type == SocketType::SERVER)
        initialize_result = this->setup_server(config);
    else if (config._type == SocketType::CLIENT)
        initialize_result = this->setup_client(config);
    if (initialize_result != FT_ERR_SUCCESS)
        this->_initialized_state = ft_socket::_state_destroyed;
    return (initialize_result);
}

int ft_socket::destroy()
{
    if (this->_initialized_state != ft_socket::_state_initialized)
        this->abort_lifecycle_error("ft_socket::destroy",
            "destroy called on non-initialized instance");
    this->disconnect_all_clients();
    (void)this->close_socket();
    this->_initialized_state = ft_socket::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_socket::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

#endif
