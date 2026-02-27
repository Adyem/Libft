#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/map.hpp"
#include <cstdint>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

class ft_socket;

class ft_websocket_server
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name, const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        struct s_connection_state
        {
            bool _permessage_deflate_enabled;
        };

        ft_socket *_server_socket;
        mutable pt_recursive_mutex *_mutex;
        ft_map<int, s_connection_state> _connection_states;

        void store_connection_state_locked(int client_fd, bool permessage_deflate_enabled);
        void remove_connection_state_locked(int client_fd);
        bool connection_supports_permessage_deflate_locked(int client_fd) const;
        int perform_handshake_locked(int client_fd);
        int receive_frame_locked(int client_fd, ft_string &message);
        int send_pong_locked(int client_fd, const unsigned char *payload, std::size_t length);

    public:
        ft_websocket_server();
        ~ft_websocket_server();
        ft_websocket_server(const ft_websocket_server &other) = delete;
        ft_websocket_server &operator=(const ft_websocket_server &other) = delete;
        ft_websocket_server(ft_websocket_server &&other) noexcept = delete;
        ft_websocket_server &operator=(ft_websocket_server &&other) noexcept = delete;

        int initialize();
        int destroy();
        int start(const char *ip, uint16_t port, int address_family = AF_INET, bool non_blocking = false);
        int run_once(int &client_fd, ft_string &message);
        int send_text(int client_fd, const ft_string &message);
        int get_port(unsigned short &port_value) const;
};

#endif
