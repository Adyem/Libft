#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Networking/socket_class.hpp"
#include "../Template/pair.hpp"

ft_game_server::ft_game_server(ft_world &world, const char *auth_token) noexcept
    : _server(), _world(&world), _clients(), _auth_token(), _on_join(ft_nullptr), _on_leave(ft_nullptr), _error_code(ER_SUCCESS)
{
    if (auth_token)
        this->_auth_token = auth_token;
    return ;
}

ft_game_server::~ft_game_server()
{
    return ;
}

ft_game_server::ft_game_server(const ft_game_server &other) noexcept
    : _server(other._server), _world(other._world), _clients(other._clients), _auth_token(other._auth_token), _on_join(other._on_join), _on_leave(other._on_leave), _error_code(other._error_code)
{
    return ;
}

ft_game_server &ft_game_server::operator=(const ft_game_server &other) noexcept
{
    if (this != &other)
    {
        this->_server = other._server;
        this->_world = other._world;
        this->_clients = other._clients;
        this->_auth_token = other._auth_token;
        this->_on_join = other._on_join;
        this->_on_leave = other._on_leave;
        this->_error_code = other._error_code;
    }
    return (*this);
}

ft_game_server::ft_game_server(ft_game_server &&other) noexcept
    : _server(ft_move(other._server)), _world(other._world), _clients(ft_move(other._clients)), _auth_token(ft_move(other._auth_token)), _on_join(other._on_join), _on_leave(other._on_leave), _error_code(other._error_code)
{
    other._world = ft_nullptr;
    other._on_join = ft_nullptr;
    other._on_leave = ft_nullptr;
    other._error_code = ER_SUCCESS;
    return ;
}

ft_game_server &ft_game_server::operator=(ft_game_server &&other) noexcept
{
    if (this != &other)
    {
        this->_server = ft_move(other._server);
        this->_world = other._world;
        this->_clients = ft_move(other._clients);
        this->_auth_token = ft_move(other._auth_token);
        this->_on_join = other._on_join;
        this->_on_leave = other._on_leave;
        this->_error_code = other._error_code;
        other._world = ft_nullptr;
        other._on_join = ft_nullptr;
        other._on_leave = ft_nullptr;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

void ft_game_server::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

int ft_game_server::start(const char *ip, uint16_t port) noexcept
{
    if (this->_server.start(ip, port, AF_INET, false) != 0)
    {
        this->set_error(this->_server.get_error());
        return (1);
    }
    this->_error_code = ER_SUCCESS;
    return (0);
}

void ft_game_server::set_join_callback(void (*callback)(int)) noexcept
{
    this->_on_join = callback;
    return ;
}

void ft_game_server::set_leave_callback(void (*callback)(int)) noexcept
{
    this->_on_leave = callback;
    return ;
}

int ft_game_server::handle_message(int client_handle, const ft_string &message) noexcept
{
    json_group *groups = json_read_from_string(message.c_str());
    if (!groups)
    {
        this->set_error(ft_errno);
        return (1);
    }
    json_group *join_group = json_find_group(groups, "join");
    if (join_group)
    {
        json_item *id_item = json_find_item(join_group, "id");
        json_item *token_item = json_find_item(join_group, "token");
        if (!id_item)
        {
            json_free_groups(groups);
            this->set_error(GAME_GENERAL_ERROR);
            return (1);
        }
        if (this->_auth_token.size() > 0)
        {
            if (!token_item || this->_auth_token != token_item->value)
            {
                json_free_groups(groups);
                FT_CLOSE_SOCKET(client_handle);
                this->set_error(GAME_GENERAL_ERROR);
                return (1);
            }
        }
        this->join_client(ft_atoi(id_item->value), client_handle);
        json_free_groups(groups);
        this->_error_code = ER_SUCCESS;
        return (0);
    }
    json_group *leave_group = json_find_group(groups, "leave");
    if (leave_group)
    {
        json_item *id_item = json_find_item(leave_group, "id");
        if (!id_item)
        {
            json_free_groups(groups);
            this->set_error(GAME_GENERAL_ERROR);
            return (1);
        }
        this->leave_client(ft_atoi(id_item->value));
        FT_CLOSE_SOCKET(client_handle);
        json_free_groups(groups);
        this->_error_code = ER_SUCCESS;
        return (0);
    }
    json_group *event_group = json_find_group(groups, "event");
    if (!event_group)
    {
        json_free_groups(groups);
        this->set_error(GAME_GENERAL_ERROR);
        return (1);
    }
    json_item *id_item = json_find_item(event_group, "id");
    json_item *duration_item = json_find_item(event_group, "duration");
    if (!id_item || !duration_item)
    {
        json_free_groups(groups);
        this->set_error(GAME_GENERAL_ERROR);
        return (1);
    }
    ft_event event;
    event.set_id(ft_atoi(id_item->value));
    event.set_duration(ft_atoi(duration_item->value));
    this->_world->schedule_event(event);
    json_free_groups(groups);
    if (this->_world->get_event_scheduler().get_error() != ER_SUCCESS)
    {
        this->set_error(this->_world->get_event_scheduler().get_error());
        return (1);
    }
    this->_error_code = ER_SUCCESS;
    return (0);
}

int ft_game_server::serialize_world(ft_string &out) const noexcept
{
    json_document document;
    json_group *group = serialize_event_scheduler(this->_world->get_event_scheduler());
    if (!group)
    {
        this->set_error(ft_errno);
        return (1);
    }
    document.append_group(group);
    char *content = document.write_to_string();
    if (!content)
    {
        this->set_error(ft_errno);
        return (1);
    }
    out = content;
    cma_free(content);
    this->_error_code = ER_SUCCESS;
    return (0);
}

void ft_game_server::run_once() noexcept
{
    int client_handle;
    ft_string message;
    if (this->_server.run_once(client_handle, message) != 0)
    {
        this->set_error(this->_server.get_error());
        return ;
    }
    if (this->handle_message(client_handle, message) != 0)
        return ;
    ft_string update;
    if (this->serialize_world(update) != 0)
        return ;
    Pair<int, int> *client_ptr = this->_clients.end() - this->_clients.size();
    Pair<int, int> *client_end = this->_clients.end();
    while (client_ptr != client_end)
    {
        this->_server.send_text(client_ptr->value, update);
        client_ptr++;
    }
    return ;
}

void ft_game_server::join_client(int client_id, int client_handle) noexcept
{
    this->_clients.insert(client_id, client_handle);
    if (this->_on_join)
        this->_on_join(client_id);
    return ;
}

void ft_game_server::leave_client(int client_id) noexcept
{
    this->_clients.remove(client_id);
    if (this->_on_leave)
        this->_on_leave(client_id);
    return ;
}

int ft_game_server::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_game_server::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
