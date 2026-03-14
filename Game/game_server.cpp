#include "../PThread/pthread_internal.hpp"
#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Networking/socket_class.hpp"
#include "../Template/pair.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local uint32_t game_server::_last_error = FT_ERR_SUCCESS;

uint32_t game_server::set_error(uint32_t error_code) noexcept
{
    game_server::_last_error = error_code;
    return (error_code);
}

int32_t game_server::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_server::get_error");
    return (static_cast<int32_t>(game_server::_last_error));
}

const char *game_server::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "game_server::get_error_str");
    return (ft_strerror(game_server::_last_error));
}

game_server::game_server() noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(),
      _on_join(ft_nullptr), _on_leave(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_server::game_server(const game_server &other) noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(),
      _on_join(ft_nullptr), _on_leave(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_server::game_server(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other._world, other._auth_token.c_str());
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_clients = other._clients;
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->set_error(other.get_error());
    return ;
}

game_server::game_server(game_server &&other) noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(),
      _on_join(ft_nullptr), _on_leave(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_server::game_server(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t game_server::initialize() noexcept
{
    return (this->initialize(ft_sharedptr<game_world>(), ft_nullptr));
}

int32_t game_server::initialize(const ft_sharedptr<game_world> &world,
    const char *auth_token) noexcept
{
    ft_websocket_server *server_instance;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_server::initialize", "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_world = world;
    if (this->_auth_token.initialize() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_auth_token.get_error());
        return (this->get_error());
    }
    int32_t clients_error = this->_clients.initialize();
    if (clients_error != FT_ERR_SUCCESS)
    {
        (void)this->_auth_token.destroy();
        this->set_error(clients_error);
        return (this->get_error());
    }
    this->_auth_token.clear();
    this->_clients.clear();
    this->_on_join = ft_nullptr;
    this->_on_leave = ft_nullptr;
    if (this->_server != ft_nullptr)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    server_instance = new (std::nothrow) ft_websocket_server();
    if (server_instance == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    this->_server = server_instance;
    if (auth_token != ft_nullptr)
        this->_auth_token = auth_token;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_server::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    if (this->_server != ft_nullptr)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    this->_world = ft_sharedptr<game_world>();
    this->_clients.clear();
    this->_auth_token.clear();
    this->_on_join = ft_nullptr;
    this->_on_leave = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_server::move(game_server &other) noexcept
{
    int32_t destroy_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_server::move", "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
    {
        this->set_error(destroy_error);
        return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    this->_server = other._server;
    this->_world = other._world;
    this->_clients = other._clients;
    this->_auth_token = other._auth_token;
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._server = ft_nullptr;
    other._world = ft_sharedptr<game_world>();
    other._clients.clear();
    other._auth_token.clear();
    other._on_join = ft_nullptr;
    other._on_leave = ft_nullptr;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(other.get_error());
    return (FT_ERR_SUCCESS);
}

game_server::~game_server()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t game_server::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_server::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);

    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_server::start(const char *ip, uint16_t port) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t start_result;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::start");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (this->_server == ft_nullptr)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    start_result = this->_server->start(ip, port, AF_INET, FT_FALSE);
    (void)this->unlock_internal(lock_acquired);
    if (start_result != 0)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void game_server::set_join_callback(void (*callback)(int32_t)) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::set_join_callback");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_on_join = callback;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_server::set_leave_callback(void (*callback)(int32_t)) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::set_leave_callback");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_on_leave = callback;
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_server::handle_message_locked(int32_t client_handle,
    const ft_string &message) noexcept
{
    json_group *groups;
    json_group *join_group;
    json_group *leave_group;
    json_group *event_group;
    json_item *id_item;
    json_item *token_item;
    json_item *duration_item;
    int32_t client_id;
    ft_sharedptr<game_event> event;
    ft_sharedptr<game_event_scheduler> scheduler;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::handle_message_locked");
    groups = json_read_from_string(message.c_str());
    if (groups == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    join_group = json_find_group(groups, "join");
    if (join_group != ft_nullptr)
    {
        id_item = json_find_item(join_group, "id");
        token_item = json_find_item(join_group, "token");
        if (id_item == ft_nullptr)
        {
            json_free_groups(groups);
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            return (FT_ERR_GAME_GENERAL_ERROR);
        }
        if (this->_auth_token.size() > 0)
        {
            if (token_item == ft_nullptr || this->_auth_token != token_item->value)
            {
                json_free_groups(groups);
                nw_close(client_handle);
                this->set_error(FT_ERR_GAME_GENERAL_ERROR);
                return (FT_ERR_GAME_GENERAL_ERROR);
            }
        }
        client_id = ft_atoi(id_item->value);
        this->join_client_locked(client_id, client_handle);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    leave_group = json_find_group(groups, "leave");
    if (leave_group != ft_nullptr)
    {
        id_item = json_find_item(leave_group, "id");
        if (id_item == ft_nullptr)
        {
            json_free_groups(groups);
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            return (FT_ERR_GAME_GENERAL_ERROR);
        }
        client_id = ft_atoi(id_item->value);
        this->leave_client_locked(client_id);
        nw_close(client_handle);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    event_group = json_find_group(groups, "event");
    if (event_group == ft_nullptr)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    id_item = json_find_item(event_group, "id");
    duration_item = json_find_item(event_group, "duration");
    if (id_item == ft_nullptr || duration_item == ft_nullptr)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    event = ft_sharedptr<game_event>(new game_event());
    if (!event)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    event->set_id(ft_atoi(id_item->value));
    event->set_duration(ft_atoi(duration_item->value));
    if (!this->_world)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    this->_world->schedule_event(event);
    json_free_groups(groups);
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_server::serialize_world_locked(ft_string &out) const noexcept
{
    ft_sharedptr<game_event_scheduler> scheduler;
    json_document document;
    json_group *group;
    char *content;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::serialize_world_locked");
    if (!this->_world)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    group = serialize_event_scheduler(scheduler);
    if (group == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    document.append_group(group);
    content = document.write_to_string();
    if (content == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (FT_ERR_GAME_GENERAL_ERROR);
    }
    out = content;
    cma_free(content);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void game_server::run_once() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t client_handle;
    ft_string message;
    ft_string update;
    Pair<int32_t, int32_t> *client_ptr;
    Pair<int32_t, int32_t> *client_end;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::run_once");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_server == ft_nullptr)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_server->run_once(client_handle, message) != 0)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->handle_message_locked(client_handle, message) != FT_ERR_SUCCESS)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->serialize_world_locked(update) != FT_ERR_SUCCESS)
    {
        (void)this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    client_ptr = this->_clients.end() - this->_clients.size();
    client_end = this->_clients.end();
    while (client_ptr != client_end)
    {
        this->_server->send_text(client_ptr->value, update);
        client_ptr++;
    }
    (void)this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int32_t game_server::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_server::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int32_t destroy_error;

    errno_abort_if_uninitialised(this->_initialised_state, "game_server::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_server::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

void game_server::join_client_locked(int32_t client_id, int32_t client_handle) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_server::join_client_locked");
    this->_clients.insert(client_id, client_handle);
    if (this->_on_join != ft_nullptr)
        this->_on_join(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void game_server::leave_client_locked(int32_t client_id) noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "game_server::leave_client_locked");
    this->_clients.remove(client_id);
    if (this->_on_leave != ft_nullptr)
        this->_on_leave(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}
