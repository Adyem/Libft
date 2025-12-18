#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <cstdint>
#include <map>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <netinet/in.h>
#endif

class ft_socket;

class ft_websocket_server
{
    private:
        struct s_connection_state
        {
            bool _permessage_deflate_enabled;
        };

        ft_socket *_server_socket;
        mutable int _error_code;
        mutable pt_mutex _mutex;
        std::map<int, s_connection_state> _connection_states;

        static void finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept;

        void set_error(int error_code) const;
        void store_connection_state_locked(int client_fd, bool permessage_deflate_enabled);
        void remove_connection_state_locked(int client_fd);
        bool connection_supports_permessage_deflate_locked(int client_fd) const;
        int perform_handshake_locked(int client_fd, ft_unique_lock<pt_mutex> &guard);
        int receive_frame_locked(int client_fd, ft_string &message, ft_unique_lock<pt_mutex> &guard);
        int send_pong_locked(int client_fd, const unsigned char *payload, std::size_t length, ft_unique_lock<pt_mutex> &guard);

    public:
        ft_websocket_server();
        ~ft_websocket_server();

        int start(const char *ip, uint16_t port, int address_family = AF_INET, bool non_blocking = false);
        int run_once(int &client_fd, ft_string &message);
        int send_text(int client_fd, const ft_string &message);
        int get_error() const;
        const char *get_error_str() const;
        int get_port(unsigned short &port_value) const;
};

#endif
