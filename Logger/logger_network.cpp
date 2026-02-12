#include "logger_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include <new>

int ft_log_set_remote_sink(const char *host, unsigned short port, bool use_tcp)
{
    s_network_sink *sink;
    struct sockaddr_in address;
    int socket_fd;
    int socket_type;
    int error_code;

    if (!host)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    socket_type = SOCK_DGRAM;
    if (use_tcp)
        socket_type = SOCK_STREAM;
    socket_fd = nw_socket(AF_INET, socket_type, 0);
    if (socket_fd < 0)
    {
        ft_global_error_stack_push(FT_ERR_SOCKET_CREATION_FAILED);
        return (-1);
    }
    ft_memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (nw_inet_pton(AF_INET, host, &address.sin_addr) != 1)
    {
        su_close(socket_fd);
        ft_global_error_stack_push(FT_ERR_INVALID_IP_FORMAT);
        return (-1);
    }
    if (use_tcp)
    {
        if (nw_connect(socket_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) != 0)
        {
            su_close(socket_fd);
            ft_global_error_stack_push(FT_ERR_SOCKET_CONNECT_FAILED);
            return (-1);
        }
    }
    else
    {
        nw_connect(socket_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address));
    }
    sink = new(std::nothrow) s_network_sink;
    if (!sink)
    {
        su_close(socket_fd);
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (-1);
    }
    sink->socket_fd = socket_fd;
    sink->send_function = nw_send;
    sink->port = port;
    sink->use_tcp = use_tcp;
    sink->host = host;
    if (sink->host.get_error() != FT_ERR_SUCCESS)
    {
        su_close(socket_fd);
        delete sink;
        ft_global_error_stack_push(sink->host.get_error());
        return (-1);
    }
    error_code = network_sink_prepare_thread_safety(sink);
    if (error_code != FT_ERR_SUCCESS)
    {
        su_close(socket_fd);
        delete sink;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (ft_log_add_sink(ft_network_sink, sink) != 0)
    {
        error_code = ft_global_error_stack_drop_last_error();
        su_close(socket_fd);
        network_sink_teardown_thread_safety(sink);
        delete sink;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

void ft_network_sink(const char *message, void *user_data)
{
    s_network_sink *sink;
    size_t message_length;
    size_t total_bytes_sent;
    bool   lock_acquired;
    int    final_errno;

    sink = static_cast<s_network_sink *>(user_data);
    if (!sink || !message)
        return ;
    lock_acquired = false;
    if (network_sink_lock(sink, &lock_acquired) != 0)
        return ;
    final_errno = FT_ERR_SUCCESS;
    if (sink->socket_fd < 0)
    {
        final_errno = FT_ERR_SUCCESS;
        goto cleanup;
    }
    if (!sink->send_function)
        sink->send_function = nw_send;
    if (!sink->send_function)
    {
        final_errno = FT_ERR_INVALID_STATE;
        goto cleanup;
    }
    message_length = ft_strlen(message);
    total_bytes_sent = 0;
    while (total_bytes_sent < message_length)
    {
        ssize_t send_result;

        send_result = sink->send_function(sink->socket_fd, message + total_bytes_sent, message_length - total_bytes_sent, 0);
        if (send_result <= 0)
        {
            if (sink->socket_fd >= 0)
                su_close(sink->socket_fd);
            sink->socket_fd = -1;
            sink->send_function = ft_nullptr;
            final_errno = FT_ERR_SOCKET_SEND_FAILED;
            goto cleanup;
        }
        total_bytes_sent += static_cast<size_t>(send_result);
    }
    final_errno = FT_ERR_SUCCESS;

cleanup:
    if (lock_acquired)
        network_sink_unlock(sink, lock_acquired);
    return ;
}
