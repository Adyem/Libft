#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "openssl_support.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netinet/in.h>
# include <sys/socket.h>
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
typedef int (*t_nw_socket_hook)(int domain, int type, int protocol);

int nw_socket(int domain, int type, int protocol);
int nw_close(int sockfd);
int nw_shutdown(int sockfd, int how);
void nw_set_socket_hook(t_nw_socket_hook hook);
ssize_t nw_send(int sockfd, const void *buf, size_t len, int flags);
ssize_t nw_recv(int sockfd, void *buf, size_t len, int flags);
ssize_t nw_sendto(int sockfd, const void *buf, size_t len, int flags,
                  const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t nw_recvfrom(int sockfd, void *buf, size_t len, int flags,
                    struct sockaddr *src_addr, socklen_t *addrlen);
int nw_inet_pton(int family, const char *ip_address, void *destination);
int nw_set_nonblocking(int socket_fd);
int nw_poll(int *read_file_descriptors, int read_count,
            int *write_file_descriptors, int write_count,
            int timeout_milliseconds);

class pt_mutex;

struct networking_resolved_address
{
    sockaddr_storage    address;
    socklen_t           length;
};

bool networking_dns_resolve(const char *host, const char *service,
    int family, int socktype, int protocol, int flags,
    ft_vector<networking_resolved_address> &out_addresses) noexcept;

bool networking_dns_resolve_first(const char *host, const char *service,
    int family, int socktype, int protocol, int flags,
    networking_resolved_address &out_address) noexcept;

void networking_dns_clear_cache(void) noexcept;

void networking_dns_set_error(int resolver_status) noexcept;

#if NETWORKING_HAS_OPENSSL
int networking_check_ssl_after_send(SSL *ssl_connection);
#endif

struct event_loop
{
    int *read_file_descriptors;
    int read_count;
    int *write_file_descriptors;
    int write_count;
    pt_mutex *mutex;
    bool thread_safe_enabled;
};

class udp_socket;

void event_loop_init(event_loop *loop);
void event_loop_clear(event_loop *loop);
int event_loop_add_socket(event_loop *loop, int socket_fd, bool is_write);
int event_loop_remove_socket(event_loop *loop, int socket_fd, bool is_write);
int event_loop_run(event_loop *loop, int timeout_milliseconds);
int event_loop_prepare_thread_safety(event_loop *loop);
void event_loop_teardown_thread_safety(event_loop *loop);
int event_loop_lock(event_loop *loop, bool *lock_acquired);
void event_loop_unlock(event_loop *loop, bool lock_acquired);

int udp_event_loop_wait_read(event_loop *loop, udp_socket &socket, int timeout_milliseconds);
int udp_event_loop_wait_write(event_loop *loop, udp_socket &socket, int timeout_milliseconds);
ssize_t udp_event_loop_receive(event_loop *loop, udp_socket &socket, void *buffer, size_t size,
                               int flags, struct sockaddr *source_address,
                               socklen_t *address_length, int timeout_milliseconds);
ssize_t udp_event_loop_send(event_loop *loop, udp_socket &socket, const void *data, size_t size,
                            int flags, const struct sockaddr *destination_address,
                            socklen_t address_length, int timeout_milliseconds);

int networking_check_socket_after_send(int socket_fd);

enum class SocketType
{
    SERVER,
    CLIENT,
    RAW
};

class SocketConfig
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;

    public:
        SocketType _type;
        char _ip[46];
        uint16_t _port;
        int _backlog;
        int _protocol;
        int _address_family;
        bool _reuse_address;
        bool _non_blocking;
        int _recv_timeout;
        int _send_timeout;
        char _multicast_group[46];
        char _multicast_interface[46];

        SocketConfig();
        ~SocketConfig();

        SocketConfig(const SocketConfig& other) noexcept = delete;
        SocketConfig(SocketConfig&& other) noexcept = delete;
        SocketConfig& operator=(const SocketConfig& other) noexcept = delete;
        SocketConfig& operator=(SocketConfig&& other) noexcept = delete;
        int initialize() noexcept;
        int destroy() noexcept;
};

int socket_config_prepare_thread_safety(SocketConfig *config);
void socket_config_teardown_thread_safety(SocketConfig *config);
int socket_config_lock(const SocketConfig *config, bool *lock_acquired);
void socket_config_unlock(const SocketConfig *config, bool lock_acquired);

#endif
