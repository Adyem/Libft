#ifndef NETWORKING_UDP_SOCKET_HPP
#define NETWORKING_UDP_SOCKET_HPP

#include "networking.hpp"
#include "../PThread/mutex.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <unistd.h>
# include <arpa/inet.h>
#endif

class udp_socket
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name, const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int     create_socket(const SocketConfig &config);
        int     set_non_blocking(const SocketConfig &config);
        int     set_timeouts(const SocketConfig &config);
        int     configure_address(const SocketConfig &config);
        int     bind_socket(const SocketConfig &config);
        int     connect_socket(const SocketConfig &config);

        struct sockaddr_storage _address;
        int     _socket_fd;
        mutable pt_mutex _mutex;

    public:
        udp_socket();
        ~udp_socket();
        udp_socket(const udp_socket &other) = delete;
        udp_socket &operator=(const udp_socket &other) = delete;
        udp_socket(udp_socket &&other) noexcept = delete;
        udp_socket &operator=(udp_socket &&other) noexcept = delete;

        int     initialize(const SocketConfig &config);
        ssize_t send_to(const void *data, size_t size, int flags,
                        const struct sockaddr *dest_addr, socklen_t addr_len);
        ssize_t receive_from(void *buffer, size_t size, int flags,
                             struct sockaddr *src_addr, socklen_t *addr_len);
        bool    close_socket();
        int     get_fd() const;
        const struct sockaddr_storage &get_address() const;
};

#endif
