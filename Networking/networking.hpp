#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include "../CPP_class/class_string_class.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netinet/in.h>
#endif
#include <cstdint>

int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int nw_listen(int sockfd, int backlog);
int nw_socket(int domain, int type, int protocol);

enum class SocketType
{
    SERVER,
    CLIENT,
    RAW
};

class SocketConfig
{
    private:
        int _error;

    public:
        SocketType _type;
        ft_string _ip;
        uint16_t _port;
        int _backlog;
        int _protocol;
        int _address_family;
        bool _reuse_address;
        bool _non_blocking;
        int _recv_timeout;
        int _send_timeout;
        ft_string _multicast_group;
        ft_string _multicast_interface;

        SocketConfig();
        ~SocketConfig();

        SocketConfig(const SocketConfig& other) noexcept;
        SocketConfig(SocketConfig&& other) noexcept;
        SocketConfig& operator=(const SocketConfig& other) noexcept;
        SocketConfig& operator=(SocketConfig&& other) noexcept;

        int get_error();
        const char *get_error_str();
};

#endif
