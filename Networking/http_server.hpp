#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "socket_class.hpp"
#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include <cstdint>

class ft_http_server
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name, const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        ft_socket _server_socket;
        bool _non_blocking;
        mutable pt_mutex _mutex;

        int run_once_locked();

    public:
        ft_http_server();
        ~ft_http_server();
        ft_http_server(const ft_http_server &other) = delete;
        ft_http_server &operator=(const ft_http_server &other) = delete;
        ft_http_server(ft_http_server &&other) noexcept = delete;
        ft_http_server &operator=(ft_http_server &&other) noexcept = delete;

        int initialize();
        int destroy();
        int start(const char *ip, uint16_t port, int address_family = AF_INET, bool non_blocking = false);
        int run_once();
};

#endif
