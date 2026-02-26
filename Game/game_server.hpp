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
#include "../PThread/recursive_mutex.hpp"
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
        static thread_local int        _last_error;
        uint8_t                       _initialized_state;
        static const uint8_t          _state_uninitialized = 0;
        static const uint8_t          _state_destroyed = 1;
        static const uint8_t          _state_initialized = 2;
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int handle_message_locked(int client_handle,
                const ft_string &message) noexcept;
        int serialize_world_locked(ft_string &out) const noexcept;
        void join_client_locked(int client_id, int client_handle) noexcept;
        void leave_client_locked(int client_id) noexcept;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;
        void set_error(int error_code) const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

    public:
        ft_game_server() noexcept;
        virtual ~ft_game_server();
        ft_game_server(const ft_game_server &other) noexcept = delete;
        ft_game_server &operator=(const ft_game_server &other) noexcept = delete;
        ft_game_server(ft_game_server &&other) noexcept = delete;
        ft_game_server &operator=(ft_game_server &&other) noexcept = delete;

        int initialize(const ft_sharedptr<ft_world> &world,
            const char *auth_token = ft_nullptr) noexcept;
        int destroy() noexcept;

        void set_join_callback(void (*callback)(int)) noexcept;
        void set_leave_callback(void (*callback)(int)) noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        int start(const char *ip, uint16_t port) noexcept;
        void run_once() noexcept;
};

#endif
