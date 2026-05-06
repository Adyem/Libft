#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "socket_handle.hpp"
#include <cstdint>

class ft_websocket_client
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        uint8_t _initialised_state;
        ft_socket_handle _socket;
        mutable pt_recursive_mutex *_mutex;

        int32_t close_locked();
        int32_t perform_handshake_locked(const char *host, const char *path);
        int32_t send_pong_locked(const unsigned char *payload, ft_size_t length);
        int32_t send_text_locked(const ft_string &message);
        int32_t receive_text_locked(ft_string &message);
    public:
        ft_websocket_client() noexcept;
        ft_websocket_client(const ft_websocket_client &other) noexcept;
        ft_websocket_client(ft_websocket_client &&other) noexcept;
        ~ft_websocket_client() noexcept;
        ft_websocket_client &operator=(const ft_websocket_client &other) = delete;
        ft_websocket_client &operator=(ft_websocket_client &&other) noexcept = delete;
        int32_t move(ft_websocket_client &other) noexcept;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_websocket_client &other) noexcept;
        int32_t initialize(ft_websocket_client &&other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t connect(const char *host, uint16_t port, const char *path);
        int32_t send_text(const ft_string &message);
        int32_t receive_text(ft_string &message);
        void close();
};

#endif
