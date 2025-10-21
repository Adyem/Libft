#ifndef NETWORKING_SOCKET_CLASS_HPP
#define NETWORKING_SOCKET_CLASS_HPP

#include "networking.hpp"
#include "../Template/vector.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <sys/types.h>
# include <unistd.h>
#endif

class ft_socket
{
    private:
        int     setup_server(const SocketConfig &config);
        int     setup_client(const SocketConfig &config);
        int     create_socket(const SocketConfig &config);
        int     set_reuse_address(const SocketConfig &config);
        int     set_non_blocking(const SocketConfig &config);
        int     set_timeouts(const SocketConfig &config);
        int     configure_address(const SocketConfig &config);
        int     bind_socket(const SocketConfig &config);
        int     listen_socket(const SocketConfig &config);
        int        accept_connection();
        void     set_error(int error_code) const noexcept;
        void     reset_to_empty_state();

        struct sockaddr_storage _address;
        ft_vector<ft_socket>     _connected;
        int                         _socket_fd;
        mutable int             _error_code;

        ft_socket(int fd, const sockaddr_storage &addr);
        ft_socket(const ft_socket &other) = delete;
        ft_socket &operator=(const ft_socket &other) = delete;

    public:
        ft_socket(const SocketConfig &config);
        ft_socket();
        ~ft_socket();

        ft_socket(ft_socket &&other) noexcept;
        ft_socket &operator=(ft_socket &&other) noexcept;

        int            initialize(const SocketConfig &config);
        ssize_t     send_data(const void *data, size_t size, int flags = 0);
        ssize_t         send_all(const void *data, size_t size, int flags = 0);
        ssize_t        receive_data(void *buffer, size_t size, int flags = 0);
        bool        close_socket();
        int         get_error() const;
        const char    *get_error_str() const;
        ssize_t     broadcast_data(const void *data, size_t size, int flags);
        ssize_t     broadcast_data(const void *data, size_t size, int flags, int exception);
        ssize_t     send_data(const void *data, size_t size, int flags, int fd);
        bool        disconnect_client(int fd);
        void        disconnect_all_clients();
        size_t      get_client_count() const;
        bool        is_client_connected(int fd) const;
        int            get_fd() const;
        const struct sockaddr_storage &get_address() const;
        int            join_multicast_group(const SocketConfig &config);
};

#endif
