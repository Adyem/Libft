#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "socket_handle.hpp"
#include <cstdint>

class ft_websocket_client
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name, const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        ft_socket_handle _socket;
        mutable pt_mutex _mutex;

        int close_locked();
        int perform_handshake_locked(const char *host, const char *path);
        int send_pong_locked(const unsigned char *payload, std::size_t length);
        int send_text_locked(const ft_string &message);
        int receive_text_locked(ft_string &message);
    public:
        ft_websocket_client();
        ~ft_websocket_client();
        ft_websocket_client(const ft_websocket_client &other) = delete;
        ft_websocket_client &operator=(const ft_websocket_client &other) = delete;
        ft_websocket_client(ft_websocket_client &&other) noexcept = delete;
        ft_websocket_client &operator=(ft_websocket_client &&other) noexcept = delete;

        int initialize();
        int destroy();
        int connect(const char *host, uint16_t port, const char *path);
        int send_text(const ft_string &message);
        int receive_text(ft_string &message);
        void close();
};

#endif
