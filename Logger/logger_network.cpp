#include "logger_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../Compatebility/compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"

int ft_log_set_remote_sink(const char *host, unsigned short port, bool use_tcp)
{
    s_network_sink *sink;
    struct sockaddr_in address;
    int socket_fd;
    int socket_type;

    if (!host)
        return (-1);
    socket_type = SOCK_DGRAM;
    if (use_tcp)
        socket_type = SOCK_STREAM;
    socket_fd = nw_socket(AF_INET, socket_type, 0);
    if (socket_fd < 0)
        return (-1);
    ft_memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    if (nw_inet_pton(AF_INET, host, &address.sin_addr) != 1)
    {
        cmp_close(socket_fd);
        return (-1);
    }
    if (use_tcp)
    {
        if (nw_connect(socket_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address)) != 0)
        {
            cmp_close(socket_fd);
            return (-1);
        }
    }
    else
    {
        nw_connect(socket_fd, reinterpret_cast<struct sockaddr *>(&address), sizeof(address));
    }
    sink = new s_network_sink;
    if (!sink)
    {
        cmp_close(socket_fd);
        return (-1);
    }
    sink->socket_fd = socket_fd;
    if (ft_log_add_sink(ft_network_sink, sink) != 0)
    {
        cmp_close(socket_fd);
        delete sink;
        return (-1);
    }
    return (0);
}

void ft_network_sink(const char *message, void *user_data)
{
    s_network_sink *sink;
    size_t length;

    sink = static_cast<s_network_sink *>(user_data);
    if (!sink)
        return ;
    length = ft_strlen(message);
    nw_send(sink->socket_fd, message, length, 0);
    return ;
}
