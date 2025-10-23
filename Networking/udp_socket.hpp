#ifndef NETWORKING_UDP_SOCKET_HPP
#define NETWORKING_UDP_SOCKET_HPP

#include "networking.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

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
        static void restore_errno(ft_unique_lock<pt_mutex> &guard,
                int entry_errno) noexcept;
        int     create_socket(const SocketConfig &config);
        int     set_non_blocking(const SocketConfig &config);
        int     set_timeouts(const SocketConfig &config);
        int     configure_address(const SocketConfig &config);
        int     bind_socket(const SocketConfig &config);
        int     connect_socket(const SocketConfig &config);
        void    set_error(int error_code) const noexcept;

        struct sockaddr_storage _address;
        int     _socket_fd;
        mutable int _error_code;
        mutable pt_mutex _mutex;

    public:
        udp_socket();
        ~udp_socket();

        int     initialize(const SocketConfig &config);
        ssize_t send_to(const void *data, size_t size, int flags,
                        const struct sockaddr *dest_addr, socklen_t addr_len);
        ssize_t receive_from(void *buffer, size_t size, int flags,
                             struct sockaddr *src_addr, socklen_t *addr_len);
        bool    close_socket();
        int     get_error() const;
        const char  *get_error_str() const;
        int     get_fd() const;
        const struct sockaddr_storage &get_address() const;
};

#endif
