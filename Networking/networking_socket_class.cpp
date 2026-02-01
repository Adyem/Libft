#include "socket_class.hpp"
#include "networking.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
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

static int networking_pop_guard_error(const ft_unique_lock<pt_recursive_mutex> &guard) noexcept
{
    unsigned long long operation_id = guard.last_operation_id();

    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    return (guard.pop_operation_error(operation_id));
}

static int socket_finalize_guard(ft_unique_lock<pt_recursive_mutex> &guard) noexcept
{
    int guard_error;

    if (guard.owns_lock())
        guard.unlock();
    guard_error = networking_pop_guard_error(guard);
    return (guard_error);
}

static int networking_map_socket_error(void) noexcept
{
#ifdef _WIN32
    return (ft_map_system_error(WSAGetLastError()));
#else
    return (ft_map_system_error(errno));
#endif
}

struct networking_error_entry
{
    int                    error_code;
    unsigned long long     operation_id;
};

static networking_error_entry networking_consume_last_error(void) noexcept
{
    networking_error_entry entry;

    entry.error_code = ft_global_error_stack_last_error();
    entry.operation_id = ft_global_error_stack_get_id_at(1);
    ft_global_error_stack_pop_newest();
    return (entry);
}

int ft_socket::lock_pair(const ft_socket &first, const ft_socket &second,
        ft_unique_lock<pt_recursive_mutex> &first_guard,
        ft_unique_lock<pt_recursive_mutex> &second_guard)
{
    const ft_socket *ordered_first;
    const ft_socket *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_recursive_mutex> single_guard(first._mutex);
        int single_error = networking_pop_guard_error(single_guard);
        if (single_error != FT_ERR_SUCCESSS)
            return (single_error);
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_recursive_mutex>();
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_socket *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_recursive_mutex> lower_guard(ordered_first->_mutex);
        int lower_error = networking_pop_guard_error(lower_guard);
        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        ft_unique_lock<pt_recursive_mutex> upper_guard(ordered_second->_mutex);
        int upper_error = networking_pop_guard_error(upper_guard);
        if (upper_error == FT_ERR_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESSS);
        }
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_error);
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_socket::sleep_backoff();
    }
}

ssize_t ft_socket::send_data_locked(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_sent;

    bytes_sent = nw_send(this->_socket_fd, data, size, flags);
    if (bytes_sent < 0)
        this->report_operation_result(networking_map_socket_error());
    else
        this->report_operation_result(FT_ERR_SUCCESSS);
    return (bytes_sent);
}

ssize_t ft_socket::send_all_locked(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_CONFIGURATION);
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
                    this->report_operation_result(entry.error_code, entry.operation_id);
                    return (-1);
                }
                networking_consume_last_error();
                continue ;
            }
            this->report_operation_result(ft_map_system_error(last_error));
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
            {
                ft_socket::sleep_backoff();
                int check_result = networking_check_socket_after_send(this->_socket_fd);
                if (check_result != 0)
                {
                    networking_error_entry entry = networking_consume_last_error();
                    this->report_operation_result(entry.error_code, entry.operation_id);
                    return (-1);
                }
                networking_consume_last_error();
                continue ;
            }
            this->report_operation_result(networking_map_socket_error());
#endif
            return (-1);
        }
        if (bytes_sent == 0)
        {
            if (networking_check_socket_after_send(this->_socket_fd) != 0)
            {
                networking_error_entry entry = networking_consume_last_error();
                this->report_operation_result(entry.error_code, entry.operation_id);
                return (-1);
            }
            this->report_operation_result(FT_ERR_SOCKET_SEND_FAILED);
            return (-1);
        }
        total_sent += bytes_sent;
    }
    if (networking_check_socket_after_send(this->_socket_fd) != 0)
    {
        networking_error_entry entry = networking_consume_last_error();
        this->report_operation_result(entry.error_code, entry.operation_id);
        return (-1);
    }
    networking_consume_last_error();
    this->report_operation_result(FT_ERR_SUCCESSS);
    return (static_cast<ssize_t>(total_sent));
}

ssize_t ft_socket::receive_data_locked(void *buffer, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ssize_t bytes_received;

    bytes_received = nw_recv(this->_socket_fd, buffer, size, flags);
    if (bytes_received < 0)
        this->report_operation_result(networking_map_socket_error());
    else
        this->report_operation_result(FT_ERR_SUCCESSS);
    return (bytes_received);
}

bool ft_socket::close_socket_locked()
{
    if (this->_socket_fd >= 0)
    {
        if (nw_close(this->_socket_fd) == 0)
        {
            this->_socket_fd = -1;
            this->report_operation_result(FT_ERR_SUCCESSS);
            return (true);
        }
#ifdef _WIN32
        this->report_operation_result(ft_map_system_error(WSAGetLastError()));
#else
        this->report_operation_result(ft_map_system_error(errno));
#endif
        return (false);
    }
    this->report_operation_result(FT_ERR_SUCCESSS);
    return (true);
}

void ft_socket::reset_to_empty_state_locked()
{
    size_t connection_index;

    connection_index = 0;
    while (connection_index < this->_connected.size())
    {
        ft_socket &client = this->_connected[connection_index];
        ft_unique_lock<pt_recursive_mutex> client_guard(client._mutex);
        if (client_guard.last_operation_error() == FT_ERR_SUCCESSS)
            client.close_socket_locked();
        socket_finalize_guard(client_guard);
        connection_index++;
    }
    this->_connected.clear();
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_socket_fd = -1;
    this->report_operation_result(FT_ERR_SUCCESSS);
    return ;
}

ft_socket::ft_socket()
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);

    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return ;
    }
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return ;
}

ft_socket::ft_socket(int fd, const sockaddr_storage &addr)
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);

    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return ;
    }
    this->_address = addr;
    this->_connected.clear();
    this->_socket_fd = fd;
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return ;
}

ft_socket::ft_socket(const SocketConfig &config)
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);

    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return ;
    }
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    this->initialize(config);
    return ;
}

ft_socket::~ft_socket()
{
    this->disconnect_all_clients();
    this->close_socket();
    return ;
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags)
{
    ssize_t bytes_sent;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);

    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    bytes_sent = this->send_data_locked(data, size, flags);
    socket_finalize_guard(guard);
    return (bytes_sent);
}

ssize_t ft_socket::send_all(const void *data, size_t size, int flags)
{
    ssize_t result;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);

    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    result = this->send_all_locked(data, size, flags);
    socket_finalize_guard(guard);
    return (result);
}

ssize_t ft_socket::receive_data(void *buffer, size_t size, int flags)
{
    int socket_fd;
    int receive_errno;
    ssize_t result;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);

    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_INVALID_ARGUMENT);
        socket_finalize_guard(guard);
        return (-1);
    }
    socket_fd = this->_socket_fd;
    guard.unlock();
    int unlock_error = networking_pop_guard_error(guard);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(unlock_error);
        return (-1);
    }
    result = nw_recv(socket_fd, buffer, size, flags);
    if (result < 0)
        receive_errno = networking_map_socket_error();
    else
        receive_errno = FT_ERR_SUCCESSS;
    guard.lock();
    int lock_error = networking_pop_guard_error(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(lock_error);
        return (-1);
    }
    this->report_operation_result(receive_errno);
    socket_finalize_guard(guard);
    return (result);
}

bool ft_socket::close_socket()
{
    int socket_fd;
    int close_errno;
    int shutdown_errno;
    unsigned long long close_errno_id;
    bool closed;
    bool shutdown_success;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);

    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (false);
    }
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_SUCCESSS);
        socket_finalize_guard(guard);
        return (true);
    }
    socket_fd = this->_socket_fd;
    shutdown_success = true;
    shutdown_errno = FT_ERR_SUCCESSS;
    guard.unlock();
    int unlock_error = networking_pop_guard_error(guard);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(unlock_error);
        return (false);
    }
#ifdef _WIN32
    if (nw_shutdown(socket_fd, SD_BOTH) != 0)
#else
    if (nw_shutdown(socket_fd, SHUT_RDWR) != 0)
#endif
    {
        shutdown_success = false;
        shutdown_errno = networking_map_socket_error();
    }
    if (nw_close(socket_fd) == 0)
    {
        closed = true;
        close_errno = FT_ERR_SUCCESSS;
        networking_consume_last_error();
    }
    else
    {
        closed = false;
        networking_error_entry close_entry = networking_consume_last_error();
        close_errno = close_entry.error_code;
        close_errno_id = close_entry.operation_id;
    }
    guard.lock();
    int lock_error = networking_pop_guard_error(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(lock_error);
        return (closed);
    }
    if (closed)
    {
        if (this->_socket_fd == socket_fd)
            this->_socket_fd = -1;
        if (shutdown_success)
            this->report_operation_result(FT_ERR_SUCCESSS);
        else
            this->report_operation_result(shutdown_errno);
    }
    else
        this->report_operation_result(close_errno, close_errno_id);
    socket_finalize_guard(guard);
    if (closed && shutdown_success)
        return (true);
    return (false);
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags, int fd)
{    size_t index;
    bool found;
    ssize_t result;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    index = 0;
    found = false;
    result = -1;
    while (index < this->_connected.size())
    {
        ft_socket &client = this->_connected[index];

        if (client._socket_fd == fd)
        {            ft_unique_lock<pt_recursive_mutex> client_guard(client._mutex);

            int client_error = networking_pop_guard_error(client_guard);
            if (client_error != FT_ERR_SUCCESSS)
            {
                this->report_operation_result(client_error);
                socket_finalize_guard(client_guard);
                socket_finalize_guard(guard);
                return (-1);
            }
            result = client.send_data_locked(data, size, flags);
            this->report_operation_result(client._error_code);
            socket_finalize_guard(client_guard);
            found = true;
            break;
        }
        index++;
    }
    if (!found)
    {
        this->report_operation_result(FT_ERR_INVALID_ARGUMENT);
        result = -1;
    }
    socket_finalize_guard(guard);
    return (result);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags, int exception)
{    ssize_t total_bytes_sent;
    bool send_failed;
    size_t index;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    total_bytes_sent = 0;
    send_failed = false;
    index = 0;
    while (index < this->_connected.size())
    {
        ft_socket &client = this->_connected[index];

        if (exception == client._socket_fd)
        {
            index++;
            continue ;
        }        ft_unique_lock<pt_recursive_mutex> client_guard(client._mutex);

        int client_error = networking_pop_guard_error(client_guard);
        if (client_error != FT_ERR_SUCCESSS)
        {
            this->report_operation_result(client_error);
            socket_finalize_guard(client_guard);
            send_failed = true;
            index++;
            continue ;
        }
        ssize_t bytes_sent;

        bytes_sent = client.send_data_locked(data, size, flags);
        if (bytes_sent < 0)
        {
            this->report_operation_result(client._error_code);
            send_failed = true;
        }
        else
            total_bytes_sent += bytes_sent;
        socket_finalize_guard(client_guard);
        index++;
    }
    if (!send_failed)
        this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (total_bytes_sent);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags)
{
    return (this->broadcast_data(data, size, flags, -1));
}

int ft_socket::accept_connection()
{    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (-1);
    }
    if (this->_socket_fd < 0)
    {
        this->report_operation_result(FT_ERR_INVALID_ARGUMENT);
        socket_finalize_guard(guard);
        return (-1);
    }
    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    int new_fd;

    addr_len = sizeof(client_addr);
    new_fd = nw_accept(this->_socket_fd,
            reinterpret_cast<struct sockaddr*>(&client_addr),
            &addr_len);
    if (new_fd < 0)
    {
        this->report_operation_result(networking_map_socket_error());
        socket_finalize_guard(guard);
        return (-1);
    }
    ft_socket new_socket(new_fd, client_addr);
    size_t previous_size;
    size_t current_size;

    previous_size = this->_connected.size();
    this->_connected.push_back(ft_move(new_socket));
    current_size = this->_connected.size();
    if (current_size != previous_size + 1)
    {
        int push_error;

        push_error = this->_connected.get_error();
        if (push_error == FT_ERR_SUCCESSS)
            push_error = FT_ERR_NO_MEMORY;
        this->report_operation_result(push_error);
        socket_finalize_guard(guard);
        return (-1);
    }
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (new_fd);
}

bool ft_socket::disconnect_client(int fd)
{    size_t index;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (false);
    }
    index = 0;
    while (index < this->_connected.size())
    {
        if (this->_connected[index]._socket_fd == fd)
        {
            size_t last;

            last = this->_connected.size() - 1;
            if (index != last)
                this->_connected[index] = ft_move(this->_connected[last]);
            this->_connected.pop_back();
            this->report_operation_result(FT_ERR_SUCCESSS);
            socket_finalize_guard(guard);
            return (true);
        }
        index++;
    }
    this->report_operation_result(FT_ERR_INVALID_ARGUMENT);
    socket_finalize_guard(guard);
    return (false);
}

void ft_socket::disconnect_all_clients()
{    ft_vector<ft_socket> owned_clients;
    size_t index;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return ;
    }
    owned_clients = ft_move(this->_connected);
    this->_connected.clear();
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    index = 0;
    while (index < owned_clients.size())
    {
        ft_socket &client = owned_clients[index];        ft_unique_lock<pt_recursive_mutex> client_guard(client._mutex);

        if (client_guard.last_operation_error() == FT_ERR_SUCCESSS)
            client.close_socket_locked();
        socket_finalize_guard(client_guard);
        index++;
    }
    owned_clients.clear();
    return ;
}

size_t ft_socket::get_client_count() const
{    size_t count;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        socket_finalize_guard(guard);
        return (0);
    }
    count = this->_connected.size();
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (count);
}

bool ft_socket::is_client_connected(int fd) const
{    size_t index;
    bool connected;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        socket_finalize_guard(guard);
        return (false);
    }
    index = 0;
    connected = false;
    while (index < this->_connected.size())
    {
        if (this->_connected[index]._socket_fd == fd)
        {
            connected = true;
            break;
        }
        index++;
    }
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (connected);
}

int ft_socket::get_fd() const
{    int descriptor;
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        socket_finalize_guard(guard);
        return (-1);
    }
    descriptor = this->_socket_fd;
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (descriptor);
}

const struct sockaddr_storage &ft_socket::get_address() const
{    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        socket_finalize_guard(guard);
        return (this->_address);
    }
    this->report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(guard);
    return (this->_address);
}

void ft_socket::reset_to_empty_state()
{    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return ;
    }
    this->reset_to_empty_state_locked();
    socket_finalize_guard(guard);
    return ;
}

ft_socket::ft_socket(ft_socket &&other) noexcept
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{    ft_unique_lock<pt_recursive_mutex> this_guard(this->_mutex);
    int this_guard_error = networking_pop_guard_error(this_guard);
    if (this_guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(this_guard_error);
        return ;
    }
    ft_unique_lock<pt_recursive_mutex> other_guard(other._mutex);
    int other_guard_error = networking_pop_guard_error(other_guard);
    if (other_guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(other_guard_error);
        socket_finalize_guard(this_guard);
        return ;
    }
    this->_address = other._address;
    this->_connected = ft_move(other._connected);
    this->_socket_fd = other._socket_fd;
    this->_error_code = other._error_code;
    this->report_operation_result(other._error_code);
    other.reset_to_empty_state_locked();
    other.report_operation_result(FT_ERR_SUCCESSS);
    socket_finalize_guard(other_guard);
    socket_finalize_guard(this_guard);
    return ;
}

ft_socket &ft_socket::operator=(ft_socket &&other) noexcept
{
    if (this != &other)
    {        ft_unique_lock<pt_recursive_mutex> this_guard;
        ft_unique_lock<pt_recursive_mutex> other_guard;
        int lock_error;        lock_error = ft_socket::lock_pair(*this, other, this_guard, other_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->report_operation_result(lock_error);
            return (*this);
        }
        this->reset_to_empty_state_locked();
        this->_address = other._address;
        this->_connected = ft_move(other._connected);
        this->_socket_fd = other._socket_fd;
        this->_error_code = other._error_code;
        this->report_operation_result(other._error_code);
        other.reset_to_empty_state_locked();
        other.report_operation_result(FT_ERR_SUCCESSS);
        socket_finalize_guard(other_guard);
        socket_finalize_guard(this_guard);
    }
    return (*this);
}

int ft_socket::initialize(const SocketConfig &config)
{    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    int guard_error = networking_pop_guard_error(guard);
    if (guard_error != FT_ERR_SUCCESSS)
    {
        this->report_operation_result(guard_error);
        return (this->_error_code);
    }
    if (this->_socket_fd != -1)
    {
        this->report_operation_result(FT_ERR_ALREADY_INITIALIZED);
        socket_finalize_guard(guard);
        return (this->_error_code);
    }
    socket_finalize_guard(guard);
    if (config._type == SocketType::SERVER)
        return (this->setup_server(config));
    if (config._type == SocketType::CLIENT)
        return (this->setup_client(config));
    this->report_operation_result(FT_ERR_UNSUPPORTED_TYPE);
    return (this->_error_code);
}

void ft_socket::record_operation_error(int error_code,
        unsigned long long operation_id) const noexcept
{
    if (operation_id == 0)
        operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

void ft_socket::report_operation_result(int error_code,
        unsigned long long operation_id) const noexcept
{
    this->_error_code = error_code;
    this->record_operation_error(error_code, operation_id);
    return ;
}

void ft_socket::finalize_mutex_guard(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept
{
    int guard_error;

    guard_error = socket_finalize_guard(guard);
    if (guard_error != FT_ERR_SUCCESSS)
        this->report_operation_result(guard_error);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_socket::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

ft_operation_error_stack *ft_socket::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}
#endif
