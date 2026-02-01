#ifndef NETWORKING_SOCKET_CLASS_HPP
#define NETWORKING_SOCKET_CLASS_HPP

#include "networking.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

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
        void     reset_to_empty_state();
        void     reset_to_empty_state_locked();
        static int lock_pair(const ft_socket &first, const ft_socket &second,
                ft_unique_lock<pt_recursive_mutex> &first_guard,
                ft_unique_lock<pt_recursive_mutex> &second_guard);
        static void sleep_backoff();
        ssize_t send_data_locked(const void *data, size_t size, int flags);
        ssize_t send_all_locked(const void *data, size_t size, int flags);
        ssize_t receive_data_locked(void *buffer, size_t size, int flags);
        bool close_socket_locked();

        void finalize_mutex_guard(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
        void report_operation_result(int error_code,
                unsigned long long operation_id = 0) const noexcept;
        void record_operation_error(int error_code,
                unsigned long long operation_id = 0) const noexcept;

        struct sockaddr_storage _address;
        ft_vector<ft_socket>     _connected;
        int                         _socket_fd;
        mutable int             _error_code;
        mutable pt_recursive_mutex _mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};

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
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
        ft_operation_error_stack *operation_error_stack_handle() const noexcept;
#endif
};

#endif
