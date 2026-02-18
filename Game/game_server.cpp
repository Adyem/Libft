#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Networking/socket_class.hpp"
#include "../Template/pair.hpp"
#include <new>

ft_game_server::ft_game_server(const ft_sharedptr<ft_world> &world,
    const char *auth_token) noexcept
    : _server(ft_nullptr), _world(world), _clients(), _auth_token(),
      _on_join(ft_nullptr), _on_leave(ft_nullptr), _mutex()
{
    ft_websocket_server *server_instance;

    server_instance = new (std::nothrow) ft_websocket_server();
    if (server_instance == ft_nullptr)
        return ;
    this->_server = server_instance;
    if (auth_token != ft_nullptr)
        this->_auth_token = auth_token;
    return ;
}

ft_game_server::~ft_game_server()
{
    if (this->_server != ft_nullptr)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    return ;
}

int ft_game_server::start(const char *ip, uint16_t port) noexcept
{
    int lock_error;
    int start_result;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (1);
    if (this->_server == ft_nullptr)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return (1);
    }
    start_result = this->_server->start(ip, port, AF_INET, false);
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    if (start_result != 0)
        return (1);
    return (0);
}

void ft_game_server::set_join_callback(void (*callback)(int)) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_on_join = callback;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return ;
}

void ft_game_server::set_leave_callback(void (*callback)(int)) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_on_leave = callback;
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
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

    groups = json_read_from_string(message.c_str());
    if (groups == ft_nullptr)
        return (1);
    join_group = json_find_group(groups, "join");
    if (join_group != ft_nullptr)
    {
        id_item = json_find_item(join_group, "id");
        token_item = json_find_item(join_group, "token");
        if (id_item == ft_nullptr)
        {
            json_free_groups(groups);
            return (1);
        }
        if (this->_auth_token.size() > 0)
        {
            if (token_item == ft_nullptr || this->_auth_token != token_item->value)
            {
                json_free_groups(groups);
                nw_close(client_handle);
                return (1);
            }
        }
        client_id = ft_atoi(id_item->value);
        this->join_client_locked(client_id, client_handle);
        json_free_groups(groups);
        return (0);
    }
    leave_group = json_find_group(groups, "leave");
    if (leave_group != ft_nullptr)
    {
        id_item = json_find_item(leave_group, "id");
        if (id_item == ft_nullptr)
        {
            json_free_groups(groups);
            return (1);
        }
        client_id = ft_atoi(id_item->value);
        this->leave_client_locked(client_id);
        nw_close(client_handle);
        json_free_groups(groups);
        return (0);
    }
    event_group = json_find_group(groups, "event");
    if (event_group == ft_nullptr)
    {
        json_free_groups(groups);
        return (1);
    }
    id_item = json_find_item(event_group, "id");
    duration_item = json_find_item(event_group, "duration");
    if (id_item == ft_nullptr || duration_item == ft_nullptr)
    {
        json_free_groups(groups);
        return (1);
    }
    event = ft_sharedptr<ft_event>(new ft_event());
    if (!event)
    {
        json_free_groups(groups);
        return (1);
    }
    event->set_id(ft_atoi(id_item->value));
    event->set_duration(ft_atoi(duration_item->value));
    if (!this->_world)
    {
        json_free_groups(groups);
        return (1);
    }
    this->_world->schedule_event(event);
    json_free_groups(groups);
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
        return (1);
    return (0);
}

int ft_game_server::serialize_world_locked(ft_string &out) const noexcept
{
    ft_sharedptr<ft_event_scheduler> scheduler;
    json_document document;
    json_group *group;
    char *content;

    if (!this->_world)
        return (1);
    scheduler = this->_world->get_event_scheduler();
    if (!scheduler)
        return (1);
    group = serialize_event_scheduler(scheduler);
    if (group == ft_nullptr)
        return (1);
    document.append_group(group);
    content = document.write_to_string();
    if (content == ft_nullptr)
        return (1);
    out = content;
    cma_free(content);
    return (0);
}

void ft_game_server::run_once() noexcept
{
    int lock_error;
    int unlock_error;
    int client_handle;
    ft_string message;
    ft_string update;
    Pair<int, int> *client_ptr;
    Pair<int, int> *client_end;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_server == ft_nullptr)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return ;
    }
    if (this->_server->run_once(client_handle, message) != 0)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return ;
    }
    if (this->handle_message_locked(client_handle, message) != 0)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return ;
    }
    if (this->serialize_world_locked(update) != 0)
    {
        unlock_error = this->_mutex.unlock();
        (void)unlock_error;
        return ;
    }
    client_ptr = this->_clients.end() - this->_clients.size();
    client_end = this->_clients.end();
    while (client_ptr != client_end)
    {
        this->_server->send_text(client_ptr->value, update);
        client_ptr++;
    }
    unlock_error = this->_mutex.unlock();
    (void)unlock_error;
    return ;
}

void ft_game_server::join_client_locked(int client_id, int client_handle) noexcept
{
    this->_clients.insert(client_id, client_handle);
    if (this->_on_join != ft_nullptr)
        this->_on_join(client_id);
    return ;
}

void ft_game_server::leave_client_locked(int client_id) noexcept
{
    this->_clients.remove(client_id);
    if (this->_on_leave != ft_nullptr)
        this->_on_leave(client_id);
    return ;
}
