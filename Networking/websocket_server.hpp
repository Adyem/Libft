#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "../CPP_class/class_string_class.hpp"
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
        ft_socket *_server_socket;
        mutable int _error_code;

        void set_error(int error_code) const;
        int perform_handshake(int client_fd);
        int receive_frame(int client_fd, ft_string &message);
        int send_pong(int client_fd, const unsigned char *payload, std::size_t length);

    public:
        ft_websocket_server();
        ~ft_websocket_server();

        int start(const char *ip, uint16_t port, int address_family = AF_INET, bool non_blocking = false);
        int run_once(int &client_fd, ft_string &message);
        int send_text(int client_fd, const ft_string &message);
        int get_error() const;
        const char *get_error_str() const;
};

#endif
