#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
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
        mutable pt_mutex _mutex;

        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

        void set_error(int error_code) const;
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
