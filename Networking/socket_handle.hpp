#ifndef NETWORKING_SOCKET_HANDLE_HPP
#define NETWORKING_SOCKET_HANDLE_HPP

#include "../Errno/errno.hpp"
#include <cstdint>

int ft_socket_runtime_acquire();
void ft_socket_runtime_release();

class ft_socket_handle
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        int _socket_fd;
        void abort_lifecycle_error(const char *method_name, const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;

    public:
        ft_socket_handle();
        explicit ft_socket_handle(int socket_fd);
        ~ft_socket_handle();

        ft_socket_handle(const ft_socket_handle &other) = delete;
        ft_socket_handle &operator=(const ft_socket_handle &other) = delete;

        ft_socket_handle(ft_socket_handle &&other) noexcept = delete;
        ft_socket_handle &operator=(ft_socket_handle &&other) noexcept = delete;

        int initialize();
        int destroy();

        bool reset(int socket_fd);
        bool close();
        bool is_valid() const;
        int get() const;
};

#endif
