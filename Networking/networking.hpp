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

#if defined(__linux__)
# define NETWORKING_USE_EPOLL 1
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
# define NETWORKING_USE_KQUEUE 1
#else
# define NETWORKING_USE_SELECT 1
#endif

int nw_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int nw_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int nw_listen(int sockfd, int backlog);
int nw_socket(int domain, int type, int protocol);
ssize_t nw_sendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t nw_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);
int nw_inet_pton(int family, const char *ip_address, void *destination);
int nw_set_nonblocking(int socket_fd);
int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds);

struct event_loop
{
    int *read_file_descriptors;
    int read_count;
    int *write_file_descriptors;
    int write_count;
};

void event_loop_init(event_loop *loop);
void event_loop_clear(event_loop *loop);
int event_loop_add_socket(event_loop *loop, int socket_fd, bool is_write);
int event_loop_remove_socket(event_loop *loop, int socket_fd, bool is_write);
int event_loop_run(event_loop *loop, int timeout_milliseconds);

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

#include "websocket_client.hpp"
#include "websocket_server.hpp"

#endif
