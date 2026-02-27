#include "../PThread/pthread_internal.hpp"
#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Networking/socket_class.hpp"
#include "../Template/pair.hpp"
#include <new>

thread_local int ft_game_server::_last_error = FT_ERR_SUCCESS;

void ft_game_server::set_error(int error_code) const noexcept
{
    ft_game_server::_last_error = error_code;
    return ;
}

int ft_game_server::get_error() const noexcept
{
    return (ft_game_server::_last_error);
}

const char *ft_game_server::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_game_server::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_game_server lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_game_server::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_game_server::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

ft_game_server::ft_game_server() noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(),
      _on_join(ft_nullptr), _on_leave(ft_nullptr), _mutex(ft_nullptr),
      _initialized_state(ft_game_server::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_server::initialize(const ft_sharedptr<ft_world> &world,
    const char *auth_token) noexcept
{
    ft_websocket_server *server_instance;

    if (this->_initialized_state == ft_game_server::_state_initialized)
    {
        this->abort_lifecycle_error("ft_game_server::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_world = world;
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
    this->_initialized_state = ft_game_server::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_server::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_game_server::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    if (this->_server != ft_nullptr)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    this->_world = ft_sharedptr<ft_world>();
    this->_clients.clear();
    this->_auth_token.clear();
    this->_on_join = ft_nullptr;
    this->_on_leave = ft_nullptr;
    this->_initialized_state = ft_game_server::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

ft_game_server::~ft_game_server()
{
    if (this->_initialized_state == ft_game_server::_state_initialized)
        (void)this->destroy();
    return ;
}

int ft_game_server::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_game_server::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_server::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int unlock_error;

    unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (unlock_error);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_game_server::start(const char *ip, uint16_t port) noexcept
{
    bool lock_acquired;
    int lock_error;
    int start_result;
    int unlock_error;

    this->abort_if_not_initialized("ft_game_server::start");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (1);
    }
    if (this->_server == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (1);
        }
        this->set_error(FT_ERR_INVALID_STATE);
        return (1);
    }
    start_result = this->_server->start(ip, port, AF_INET, false);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (1);
    }
    if (start_result != 0)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

void ft_game_server::set_join_callback(void (*callback)(int)) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_game_server::set_join_callback");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_on_join = callback;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_server::set_leave_callback(void (*callback)(int)) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_game_server::set_leave_callback");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_on_leave = callback;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_server::handle_message_locked(int client_handle,
    const ft_string &message) noexcept
{
    json_group *groups;
    json_group *join_group;
    json_group *leave_group;
    json_group *event_group;
    json_item *id_item;
    json_item *token_item;
    json_item *duration_item;
    int client_id;
    ft_sharedptr<ft_event> event;
    ft_sharedptr<ft_event_scheduler> scheduler;

    this->abort_if_not_initialized("ft_game_server::handle_message_locked");
    groups = json_read_from_string(message.c_str());
    if (groups == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
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
            return (1);
        }
        if (this->_auth_token.size() > 0)
        {
            if (token_item == ft_nullptr || this->_auth_token != token_item->value)
            {
                json_free_groups(groups);
                nw_close(client_handle);
                this->set_error(FT_ERR_GAME_GENERAL_ERROR);
                return (1);
            }
        }
        client_id = ft_atoi(id_item->value);
        this->join_client_locked(client_id, client_handle);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (0);
    }
    leave_group = json_find_group(groups, "leave");
    if (leave_group != ft_nullptr)
    {
        id_item = json_find_item(leave_group, "id");
        if (id_item == ft_nullptr)
        {
            json_free_groups(groups);
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            return (1);
        }
        client_id = ft_atoi(id_item->value);
        this->leave_client_locked(client_id);
        nw_close(client_handle);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (0);
    }
    event_group = json_find_group(groups, "event");
    if (event_group == ft_nullptr)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    id_item = json_find_item(event_group, "id");
    duration_item = json_find_item(event_group, "duration");
    if (id_item == ft_nullptr || duration_item == ft_nullptr)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    event = ft_sharedptr<ft_event>(new ft_event());
    if (!event)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    event->set_id(ft_atoi(id_item->value));
    event->set_duration(ft_atoi(duration_item->value));
    if (!this->_world)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    this->_world->schedule_event(event);
    json_free_groups(groups);
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

int ft_game_server::serialize_world_locked(ft_string &out) const noexcept
{
    ft_sharedptr<ft_event_scheduler> scheduler;
    json_document document;
    json_group *group;
    char *content;

    this->abort_if_not_initialized("ft_game_server::serialize_world_locked");
    if (!this->_world)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    group = serialize_event_scheduler(scheduler);
    if (group == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    document.append_group(group);
    content = document.write_to_string();
    if (content == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    out = content;
    cma_free(content);
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

void ft_game_server::run_once() noexcept
{
    bool lock_acquired;
    int lock_error;
    int client_handle;
    ft_string message;
    ft_string update;
    Pair<int, int> *client_ptr;
    Pair<int, int> *client_end;
    int unlock_error;

    this->abort_if_not_initialized("ft_game_server::run_once");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    if (this->_server == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_server->run_once(client_handle, message) != 0)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->handle_message_locked(client_handle, message) != 0)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->serialize_world_locked(update) != 0)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return ;
        }
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
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_server::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_game_server::enable_thread_safety");
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

int ft_game_server::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int destroy_error;

    this->abort_if_not_initialized("ft_game_server::disable_thread_safety");
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

bool ft_game_server::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_game_server::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

void ft_game_server::join_client_locked(int client_id, int client_handle) noexcept
{
    this->abort_if_not_initialized("ft_game_server::join_client_locked");
    this->_clients.insert(client_id, client_handle);
    if (this->_on_join != ft_nullptr)
        this->_on_join(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_server::leave_client_locked(int client_id) noexcept
{
    this->abort_if_not_initialized("ft_game_server::leave_client_locked");
    this->_clients.remove(client_id);
    if (this->_on_leave != ft_nullptr)
        this->_on_leave(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}
