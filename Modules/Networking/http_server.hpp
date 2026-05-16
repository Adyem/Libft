#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "socket_class.hpp"
#include "../CPP_class/class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class ft_http_server
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        uint8_t _initialised_state;
        ft_socket _server_socket;
        ft_bool _non_blocking;
        mutable pt_recursive_mutex *_mutex;

        int32_t run_once_locked();

    public:
        ft_http_server() noexcept;
        ft_http_server(const ft_http_server &other) noexcept = delete;
        ft_http_server(ft_http_server &&other) noexcept = delete;
        ~ft_http_server() noexcept;
        ft_http_server &operator=(const ft_http_server &other) = delete;
        ft_http_server &operator=(ft_http_server &&other) noexcept = delete;
        int32_t move(ft_http_server &other) noexcept;

        int32_t initialize() noexcept;
        int32_t initialize(const ft_http_server &other) noexcept;
        int32_t initialize(ft_http_server &&other) noexcept;
        int32_t destroy() noexcept;
        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;
        ft_bool is_thread_safe() const noexcept;
        int32_t start(const char *ip_address, uint16_t port, int32_t address_family = AF_INET, ft_bool non_blocking = FT_FALSE);
        int32_t run_once() noexcept;
};

#endif
