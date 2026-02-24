#ifndef GAME_SERVER_HPP
# define GAME_SERVER_HPP

#include "game_world.hpp"
#include "game_event.hpp"
#include "../Networking/websocket_server.hpp"
#include "../JSon/json.hpp"
#include "../Template/map.hpp"
#include "../Template/shared_ptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"

class ft_game_server
{
    private:
        ft_websocket_server *_server;
        ft_sharedptr<ft_world> _world;
        ft_map<int, int>   _clients;
        ft_string          _auth_token;
        void              (*_on_join)(int);
        void              (*_on_leave)(int);
        mutable pt_recursive_mutex    *_mutex;
        int handle_message_locked(int client_handle,
                const ft_string &message) noexcept;
        int serialize_world_locked(ft_string &out) const noexcept;
        void join_client_locked(int client_id, int client_handle) noexcept;
        void leave_client_locked(int client_id) noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        void unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_game_server() noexcept;
        ~ft_game_server();
        ft_game_server(const ft_game_server &other) noexcept = delete;
        ft_game_server &operator=(const ft_game_server &other) noexcept = delete;
        ft_game_server(ft_game_server &&other) noexcept = delete;
        ft_game_server &operator=(ft_game_server &&other) noexcept = delete;

        int initialize(const ft_sharedptr<ft_world> &world,
            const char *auth_token = ft_nullptr) noexcept;

        void set_join_callback(void (*callback)(int)) noexcept;
        void set_leave_callback(void (*callback)(int)) noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int start(const char *ip, uint16_t port) noexcept;
        void run_once() noexcept;
};

#endif
