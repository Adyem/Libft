#ifndef GAME_SERVER_HPP
# define GAME_SERVER_HPP

#include "game_world.hpp"
#include "game_event.hpp"
#include "../Networking/websocket_server.hpp"
#include "../JSon/json.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

class ft_game_server
{
    private:
        ft_websocket_server _server;
        ft_world           *_world;
        mutable int         _error_code;

        void set_error(int error) const noexcept;
        int handle_message(const ft_string &message) noexcept;
        int serialize_world(ft_string &out) const noexcept;

    public:
        ft_game_server(ft_world &world) noexcept;
        ~ft_game_server();
        ft_game_server(const ft_game_server &other) noexcept;
        ft_game_server &operator=(const ft_game_server &other) noexcept;
        ft_game_server(ft_game_server &&other) noexcept;
        ft_game_server &operator=(ft_game_server &&other) noexcept;

        int start(const char *ip, uint16_t port) noexcept;
        void run_once() noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
