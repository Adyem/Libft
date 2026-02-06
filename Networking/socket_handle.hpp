#ifndef NETWORKING_SOCKET_HANDLE_HPP
#define NETWORKING_SOCKET_HANDLE_HPP

#include "../Errno/errno.hpp"

int ft_socket_runtime_acquire();
void ft_socket_runtime_release();

class ft_socket_handle
{
    private:
        int _socket_fd;

    public:
        ft_socket_handle();
        explicit ft_socket_handle(int socket_fd);
        ~ft_socket_handle();

        ft_socket_handle(const ft_socket_handle &other) = delete;
        ft_socket_handle &operator=(const ft_socket_handle &other) = delete;

        ft_socket_handle(ft_socket_handle &&other) noexcept;
        ft_socket_handle &operator=(ft_socket_handle &&other) noexcept;

        bool reset(int socket_fd);
        bool close();
        bool is_valid() const;
        int get() const;
};

#endif
