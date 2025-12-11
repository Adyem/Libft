#include "socket_class.hpp"
#include "networking.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
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

void ft_socket::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_socket::lock_pair(const ft_socket &first, const ft_socket &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_socket *ordered_first;
    const ft_socket *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);
        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
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
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);
        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
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
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
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
        this->set_error(FT_ERR_CONFIGURATION);
        return (-1);
    }
    ssize_t bytes_sent;

    bytes_sent = nw_send(this->_socket_fd, data, size, flags);
    if (bytes_sent < 0)
        this->set_error(ft_errno);
    else
        this->set_error(FT_ERR_SUCCESSS);
    return (bytes_sent);
}

ssize_t ft_socket::send_all_locked(const void *data, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_CONFIGURATION);
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
                if (networking_check_socket_after_send(this->_socket_fd) != 0)
                {
                    this->set_error(ft_errno);
                    return (-1);
                }
                continue ;
            }
            this->set_error(ft_map_system_error(last_error));
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
            {
                ft_socket::sleep_backoff();
                if (networking_check_socket_after_send(this->_socket_fd) != 0)
                {
                    this->set_error(ft_errno);
                    return (-1);
                }
                continue ;
            }
            this->set_error(ft_errno);
#endif
            return (-1);
        }
        if (bytes_sent == 0)
        {
            if (networking_check_socket_after_send(this->_socket_fd) != 0)
            {
                this->set_error(ft_errno);
                return (-1);
            }
            this->set_error(FT_ERR_SOCKET_SEND_FAILED);
            return (-1);
        }
        total_sent += bytes_sent;
    }
    if (networking_check_socket_after_send(this->_socket_fd) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (static_cast<ssize_t>(total_sent));
}

ssize_t ft_socket::receive_data_locked(void *buffer, size_t size, int flags)
{
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ssize_t bytes_received;

    bytes_received = nw_recv(this->_socket_fd, buffer, size, flags);
    if (bytes_received < 0)
        this->set_error(ft_errno);
    else
        this->set_error(FT_ERR_SUCCESSS);
    return (bytes_received);
}

bool ft_socket::close_socket_locked()
{
    if (this->_socket_fd >= 0)
    {
        if (nw_close(this->_socket_fd) == 0)
        {
            this->_socket_fd = -1;
            this->set_error(FT_ERR_SUCCESSS);
            return (true);
        }
#ifdef _WIN32
        this->set_error(ft_map_system_error(WSAGetLastError()));
#else
        this->set_error(ft_map_system_error(errno));
#endif
        return (false);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

void ft_socket::reset_to_empty_state_locked()
{
    size_t connection_index;

    connection_index = 0;
    while (connection_index < this->_connected.size())
    {
        ft_socket &client = this->_connected[connection_index];
        int client_entry_errno;

        client_entry_errno = ft_errno;
        ft_unique_lock<pt_mutex> client_guard(client._mutex);
        if (client_guard.get_error() == FT_ERR_SUCCESSS)
            client.close_socket_locked();
        ft_socket::restore_errno(client_guard, client_entry_errno);
        connection_index++;
    }
    this->_connected.clear();
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_socket_fd = -1;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_socket::ft_socket()
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return ;
}

ft_socket::ft_socket(int fd, const sockaddr_storage &addr)
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    this->_address = addr;
    this->_connected.clear();
    this->_socket_fd = fd;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return ;
}

ft_socket::ft_socket(const SocketConfig &config)
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    ft_bzero(&this->_address, sizeof(this->_address));
    this->_connected.clear();
    this->_socket_fd = -1;
    this->_error_code = FT_ERR_SUCCESSS;
    this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
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
    int entry_errno;
    ssize_t bytes_sent;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    bytes_sent = this->send_data_locked(data, size, flags);
    ft_socket::restore_errno(guard, entry_errno);
    return (bytes_sent);
}

ssize_t ft_socket::send_all(const void *data, size_t size, int flags)
{
    int entry_errno;
    ssize_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    result = this->send_all_locked(data, size, flags);
    ft_socket::restore_errno(guard, entry_errno);
    return (result);
}

ssize_t ft_socket::receive_data(void *buffer, size_t size, int flags)
{
    int entry_errno;
    int socket_fd;
    int receive_errno;
    ssize_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        ft_socket::restore_errno(guard, entry_errno);
        return (-1);
    }
    socket_fd = this->_socket_fd;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    result = nw_recv(socket_fd, buffer, size, flags);
    receive_errno = ft_errno;
    guard.lock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (result < 0)
        this->set_error(receive_errno);
    else
        this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (result);
}

bool ft_socket::close_socket()
{
    int entry_errno;
    int socket_fd;
    int close_errno;
    int shutdown_errno;
    bool closed;
    bool shutdown_success;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (false);
    }
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_SUCCESSS);
        ft_socket::restore_errno(guard, entry_errno);
        return (true);
    }
    socket_fd = this->_socket_fd;
    shutdown_success = true;
    shutdown_errno = FT_ERR_SUCCESSS;
    guard.unlock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (false);
    }
#ifdef _WIN32
    if (nw_shutdown(socket_fd, SD_BOTH) != 0)
#else
    if (nw_shutdown(socket_fd, SHUT_RDWR) != 0)
#endif
    {
        shutdown_success = false;
        shutdown_errno = ft_errno;
    }
    if (nw_close(socket_fd) == 0)
    {
        closed = true;
        close_errno = FT_ERR_SUCCESSS;
    }
    else
    {
        closed = false;
        close_errno = ft_errno;
    }
    guard.lock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (closed);
    }
    if (closed)
    {
        if (this->_socket_fd == socket_fd)
            this->_socket_fd = -1;
        if (shutdown_success)
            this->set_error(FT_ERR_SUCCESSS);
        else
            this->set_error(shutdown_errno);
    }
    else
        this->set_error(close_errno);
    ft_socket::restore_errno(guard, entry_errno);
    if (closed && shutdown_success)
        return (true);
    return (false);
}

ssize_t ft_socket::send_data(const void *data, size_t size, int flags, int fd)
{
    int entry_errno;
    size_t index;
    bool found;
    ssize_t result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    index = 0;
    found = false;
    result = -1;
    while (index < this->_connected.size())
    {
        ft_socket &client = this->_connected[index];

        if (client._socket_fd == fd)
        {
            int client_entry_errno;
            ft_unique_lock<pt_mutex> client_guard(client._mutex);

            client_entry_errno = ft_errno;
            if (client_guard.get_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(client_guard.get_error());
                ft_socket::restore_errno(client_guard, client_entry_errno);
                ft_socket::restore_errno(guard, entry_errno);
                return (-1);
            }
            result = client.send_data_locked(data, size, flags);
            if (result < 0)
                this->set_error(ft_errno);
            else
                this->set_error(FT_ERR_SUCCESSS);
            ft_socket::restore_errno(client_guard, client_entry_errno);
            found = true;
            break;
        }
        index++;
    }
    if (!found)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        result = -1;
    }
    ft_socket::restore_errno(guard, entry_errno);
    return (result);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags, int exception)
{
    int entry_errno;
    ssize_t total_bytes_sent;
    bool send_failed;
    size_t index;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
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
        }
        int client_entry_errno;
        ft_unique_lock<pt_mutex> client_guard(client._mutex);

        client_entry_errno = ft_errno;
        if (client_guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(client_guard.get_error());
            ft_socket::restore_errno(client_guard, client_entry_errno);
            send_failed = true;
            index++;
            continue ;
        }
        ssize_t bytes_sent;

        bytes_sent = client.send_data_locked(data, size, flags);
        if (bytes_sent < 0)
        {
            this->set_error(ft_errno);
            send_failed = true;
        }
        else
            total_bytes_sent += bytes_sent;
        ft_socket::restore_errno(client_guard, client_entry_errno);
        index++;
    }
    if (!send_failed)
        this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (total_bytes_sent);
}

ssize_t ft_socket::broadcast_data(const void *data, size_t size, int flags)
{
    return (this->broadcast_data(data, size, flags, -1));
}

int ft_socket::accept_connection()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (-1);
    }
    if (this->_socket_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        ft_socket::restore_errno(guard, entry_errno);
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
        this->set_error(ft_errno);
        ft_socket::restore_errno(guard, entry_errno);
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
        this->set_error(push_error);
        ft_socket::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (new_fd);
}

bool ft_socket::disconnect_client(int fd)
{
    int entry_errno;
    size_t index;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
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
            this->set_error(FT_ERR_SUCCESSS);
            ft_socket::restore_errno(guard, entry_errno);
            return (true);
        }
        index++;
    }
    this->set_error(FT_ERR_INVALID_ARGUMENT);
    ft_socket::restore_errno(guard, entry_errno);
    return (false);
}

void ft_socket::disconnect_all_clients()
{
    int entry_errno;
    ft_vector<ft_socket> owned_clients;
    size_t index;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    owned_clients = ft_move(this->_connected);
    this->_connected.clear();
    this->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    index = 0;
    while (index < owned_clients.size())
    {
        ft_socket &client = owned_clients[index];
        int client_entry_errno;
        ft_unique_lock<pt_mutex> client_guard(client._mutex);

        client_entry_errno = ft_errno;
        if (client_guard.get_error() == FT_ERR_SUCCESSS)
            client.close_socket_locked();
        ft_socket::restore_errno(client_guard, client_entry_errno);
        index++;
    }
    owned_clients.clear();
    return ;
}

size_t ft_socket::get_client_count() const
{
    int entry_errno;
    size_t count;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (0);
    }
    count = this->_connected.size();
    const_cast<ft_socket *>(this)->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (count);
}

bool ft_socket::is_client_connected(int fd) const
{
    int entry_errno;
    size_t index;
    bool connected;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
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
    const_cast<ft_socket *>(this)->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (connected);
}

int ft_socket::get_fd() const
{
    int entry_errno;
    int descriptor;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (-1);
    }
    descriptor = this->_socket_fd;
    const_cast<ft_socket *>(this)->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (descriptor);
}

const struct sockaddr_storage &ft_socket::get_address() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_address);
    }
    const_cast<ft_socket *>(this)->set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(guard, entry_errno);
    return (this->_address);
}

int ft_socket::get_error() const
{
    int entry_errno;
    int error_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (const_cast<ft_socket *>(this)->_error_code);
    }
    error_value = this->_error_code;
    ft_socket::restore_errno(guard, entry_errno);
    return (error_value);
}

const char* ft_socket::get_error_str() const
{
    int entry_errno;
    int error_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_socket *>(this)->set_error(guard.get_error());
        ft_socket::restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_value = this->_error_code;
    ft_socket::restore_errno(guard, entry_errno);
    return (ft_strerror(error_value));
}

void ft_socket::reset_to_empty_state()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    this->reset_to_empty_state_locked();
    ft_socket::restore_errno(guard, entry_errno);
    return ;
}

ft_socket::ft_socket(ft_socket &&other) noexcept
    : _address(), _connected(), _socket_fd(-1), _error_code(FT_ERR_SUCCESSS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> this_guard(this->_mutex);

    entry_errno = ft_errno;
    if (this_guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this_guard.get_error();
        return ;
    }
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = other_guard.get_error();
        ft_socket::restore_errno(this_guard, entry_errno);
        return ;
    }
    this->_address = other._address;
    this->_connected = ft_move(other._connected);
    this->_socket_fd = other._socket_fd;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    other.reset_to_empty_state_locked();
    other.set_error(FT_ERR_SUCCESSS);
    ft_socket::restore_errno(other_guard, entry_errno);
    ft_socket::restore_errno(this_guard, entry_errno);
    return ;
}

ft_socket &ft_socket::operator=(ft_socket &&other) noexcept
{
    if (this != &other)
    {
        int entry_errno;
        ft_unique_lock<pt_mutex> this_guard;
        ft_unique_lock<pt_mutex> other_guard;
        int lock_error;

        entry_errno = ft_errno;
        lock_error = ft_socket::lock_pair(*this, other, this_guard, other_guard);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            this->set_error(lock_error);
            return (*this);
        }
        this->reset_to_empty_state_locked();
        this->_address = other._address;
        this->_connected = ft_move(other._connected);
        this->_socket_fd = other._socket_fd;
        this->_error_code = other._error_code;
        this->set_error(other._error_code);
        other.reset_to_empty_state_locked();
        other.set_error(FT_ERR_SUCCESSS);
        ft_socket::restore_errno(other_guard, entry_errno);
        ft_socket::restore_errno(this_guard, entry_errno);
    }
    return (*this);
}

int ft_socket::initialize(const SocketConfig &config)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (this->_error_code);
    }
    if (this->_socket_fd != -1)
    {
        this->set_error(FT_ERR_ALREADY_INITIALIZED);
        ft_socket::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    ft_socket::restore_errno(guard, entry_errno);
    if (config._type == SocketType::SERVER)
        return (this->setup_server(config));
    if (config._type == SocketType::CLIENT)
        return (this->setup_client(config));
    this->set_error(FT_ERR_UNSUPPORTED_TYPE);
    return (this->_error_code);
}
