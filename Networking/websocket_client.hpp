#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include "../CPP_class/class_string_class.hpp"
#include <cstdint>

class ft_websocket_client
{
    private:
        int _socket_fd;
        mutable int _error_code;

        int perform_handshake(const char *host, const char *path);
        int send_pong(const unsigned char *payload, std::size_t length);
        void set_error(int error_code) const;

    public:
        ft_websocket_client();
        ~ft_websocket_client();

        int connect(const char *host, uint16_t port, const char *path);
        int send_text(const ft_string &message);
        int receive_text(ft_string &message);
        void close();
        int get_error() const;
        const char *get_error_str() const;
};

#endif
