#ifndef WEBSOCKET_CLIENT_HPP
#define WEBSOCKET_CLIENT_HPP

#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "socket_handle.hpp"
#include <cstdint>

class ft_websocket_client
{
    private:
        ft_socket_handle _socket;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;

        int close_locked(ft_unique_lock<pt_mutex> &guard);
        int perform_handshake_locked(const char *host, const char *path, ft_unique_lock<pt_mutex> &guard);
        int send_pong_locked(const unsigned char *payload, std::size_t length, ft_unique_lock<pt_mutex> &guard);
        int send_text_locked(const ft_string &message, ft_unique_lock<pt_mutex> &guard);
        int receive_text_locked(ft_string &message, ft_unique_lock<pt_mutex> &guard);
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
