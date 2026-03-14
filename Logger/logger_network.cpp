#include "logger_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include <new>

int32_t ft_log_set_remote_sink(const char *host, uint16_t port, ft_bool use_tcp)
{
    s_network_sink *sink;
    struct sockaddr_in address;
    int32_t socket_fd;
    int32_t socket_type;
    int32_t error_code_value;

    if (!host)
    {
        return (FT_ERR_INTERNAL);
    }
    socket_type = SOCK_DGRAM;
    if (use_tcp)
        socket_type = SOCK_STREAM;
    socket_fd = nw_socket(AF_INET, socket_type, 0);
    if (socket_fd < 0)
    {
        return (FT_ERR_INTERNAL);
    }
    ft_memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (nw_inet_pton(AF_INET, host, &address.sin_addr) != 1)
    {
        su_close(socket_fd);
        return (FT_ERR_INTERNAL);
    }
    if (use_tcp)
    {
        if (nw_connect(socket_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) != 0)
        {
            su_close(socket_fd);
            return (FT_ERR_INTERNAL);
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
        return (FT_ERR_INTERNAL);
    }
    sink->socket_fd = socket_fd;
    sink->send_function = nw_send;
    sink->port = port;
    sink->use_tcp = use_tcp;
    sink->host = host;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
    {
        su_close(socket_fd);
        delete sink;
        return (FT_ERR_INTERNAL);
    }
    error_code_value = network_sink_prepare_thread_safety(sink);
    if (error_code_value != FT_ERR_SUCCESS)
    {
        su_close(socket_fd);
        delete sink;
        return (FT_ERR_INTERNAL);
    }
    if (ft_log_add_sink(ft_network_sink, sink) != 0)
    {
        error_code_value = FT_ERR_SUCCESS;
        su_close(socket_fd);
        network_sink_teardown_thread_safety(sink);
        delete sink;
        if (error_code_value == FT_ERR_SUCCESS)
            error_code_value = FT_ERR_INVALID_ARGUMENT;
        return (FT_ERR_INTERNAL);
    }
    return (FT_ERR_SUCCESS);
}

void ft_network_sink(const char *message, void *user_data)
{
    s_network_sink *sink;
    ft_size_t message_length;
    ft_size_t total_bytes_sent;
    ft_bool   lock_acquired;

    sink = static_cast<s_network_sink *>(user_data);
    if (!sink || !message)
        return ;
    lock_acquired = FT_FALSE;
    if (network_sink_lock(sink, &lock_acquired) != 0)
        return ;
    if (sink->socket_fd < 0)
        goto cleanup;
    if (!sink->send_function)
        sink->send_function = nw_send;
    if (!sink->send_function)
    {
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
            goto cleanup;
        }
        total_bytes_sent += static_cast<ft_size_t>(send_result);
    }

cleanup:
    if (lock_acquired)
        network_sink_unlock(sink, lock_acquired);
    return ;
}
