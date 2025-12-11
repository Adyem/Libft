#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include "socket_class.hpp"
#include "../CPP_class/class_string.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <cstdint>

class ft_http_server
{
    private:
        ft_socket _server_socket;
        mutable int _error_code;
        bool _non_blocking;
        mutable pt_mutex _mutex;

        void set_error(int error_code) const;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;
        int run_once_locked(ft_unique_lock<pt_mutex> &guard);

    public:
        ft_http_server();
        ~ft_http_server();

        int start(const char *ip, uint16_t port, int address_family = AF_INET, bool non_blocking = false);
        int run_once();
        int get_error() const;
        const char *get_error_str() const;
};

#endif
