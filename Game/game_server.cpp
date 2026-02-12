#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Networking/socket_class.hpp"
#include "../Template/pair.hpp"
#include "../Template/shared_ptr.hpp"
#include "../PThread/pthread.hpp"
#include <utility>
#include <new>
#include "../Template/move.hpp"

static void game_server_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

int ft_game_server::lock_pair(const ft_game_server &first,
    const ft_game_server &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_game_server *ordered_first;
    const ft_game_server *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESS;
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_game_server *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ERR_SUCCESS;
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_server_sleep_backoff();
    }
}

ft_game_server::ft_game_server(const ft_sharedptr<ft_world> &world, const char *auth_token) noexcept
    : _server(ft_nullptr), _world(world), _clients(), _auth_token(), _on_join(ft_nullptr), _on_leave(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_websocket_server *server_instance;

    server_instance = new (std::nothrow) ft_websocket_server();
    if (!server_instance)
    {
        this->_server = ft_nullptr;
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_server = server_instance;
    if (auth_token)
        this->_auth_token = auth_token;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_server::~ft_game_server()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (this->_server)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_server::ft_game_server(const ft_game_server &other) noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(), _on_join(ft_nullptr), _on_leave(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_websocket_server *server_instance;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    server_instance = new (std::nothrow) ft_websocket_server();
    if (!server_instance)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_server = server_instance;
    this->_world = other._world;
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world.get_error());
        return ;
    }
    this->_clients = other._clients;
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return ;
    }
    this->_auth_token = other._auth_token;
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return ;
}

ft_game_server &ft_game_server::operator=(const ft_game_server &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_server::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    if (!this->_server)
    {
        ft_websocket_server *server_instance;

        server_instance = new (std::nothrow) ft_websocket_server();
        if (!server_instance)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return (*this);
        }
        this->_server = server_instance;
    }
    this->_world = other._world;
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world.get_error());
        return (*this);
    }
    this->_clients = other._clients;
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return (*this);
    }
    this->_auth_token = other._auth_token;
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return (*this);
}

ft_game_server::ft_game_server(ft_game_server &&other) noexcept
    : _server(ft_nullptr), _world(), _clients(), _auth_token(), _on_join(ft_nullptr), _on_leave(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_server = other._server;
    other._server = ft_nullptr;
    this->_world = ft_move(other._world);
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world.get_error());
        return ;
    }
    this->_clients = ft_move(other._clients);
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return ;
    }
    this->_auth_token = ft_move(other._auth_token);
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->_error_code = other._error_code;
    other._on_join = ft_nullptr;
    other._on_leave = ft_nullptr;
    other._auth_token.clear();
    other._clients.clear();
    if (other._clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._clients.get_error());
        return ;
    }
    other._world = ft_sharedptr<ft_world>();
    if (other._world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._world.get_error());
        return ;
    }
    this->set_error(this->_error_code);
    other._error_code = FT_ERR_SUCCESS;
    other.set_error(FT_ERR_SUCCESS);
    return ;
}

ft_game_server &ft_game_server::operator=(ft_game_server &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_game_server::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    if (this->_server)
    {
        delete this->_server;
        this->_server = ft_nullptr;
    }
    this->_server = other._server;
    other._server = ft_nullptr;
    this->_world = ft_move(other._world);
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world.get_error());
        return (*this);
    }
    this->_clients = ft_move(other._clients);
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return (*this);
    }
    this->_auth_token = ft_move(other._auth_token);
    this->_on_join = other._on_join;
    this->_on_leave = other._on_leave;
    this->_error_code = other._error_code;
    other._on_join = ft_nullptr;
    other._on_leave = ft_nullptr;
    other._auth_token.clear();
    other._clients.clear();
    if (other._clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._clients.get_error());
        return (*this);
    }
    other._world = ft_sharedptr<ft_world>();
    if (other._world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(other._world.get_error());
        return (*this);
    }
    this->set_error(this->_error_code);
    other._error_code = FT_ERR_SUCCESS;
    other.set_error(FT_ERR_SUCCESS);
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
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (1);
    }
    if (!this->_server)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (1);
    }
    if (this->_server->start(ip, port, AF_INET, false) != 0)
    {
        this->set_error(this->_server->get_error());
        return (1);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

void ft_game_server::set_join_callback(void (*callback)(int)) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_on_join = callback;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_server::set_leave_callback(void (*callback)(int)) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_on_leave = callback;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_server::handle_message_locked(int client_handle, const ft_string &message, ft_unique_lock<pt_mutex> &guard) noexcept
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
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            return (1);
        }
        if (this->_auth_token.size() > 0)
        {
            if (!token_item || this->_auth_token != token_item->value)
            {
                json_free_groups(groups);
                nw_close(client_handle);
                this->set_error(FT_ERR_GAME_GENERAL_ERROR);
                return (1);
            }
        }
        this->join_client_locked(ft_atoi(id_item->value, ft_nullptr), client_handle, guard);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (0);
    }
    json_group *leave_group = json_find_group(groups, "leave");
    if (leave_group)
    {
        json_item *id_item = json_find_item(leave_group, "id");
        if (!id_item)
        {
            json_free_groups(groups);
            this->set_error(FT_ERR_GAME_GENERAL_ERROR);
            return (1);
        }
        this->leave_client_locked(ft_atoi(id_item->value, ft_nullptr), guard);
        nw_close(client_handle);
        json_free_groups(groups);
        this->set_error(FT_ERR_SUCCESS);
        return (0);
    }
    json_group *event_group = json_find_group(groups, "event");
    if (!event_group)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    json_item *id_item = json_find_item(event_group, "id");
    json_item *duration_item = json_find_item(event_group, "duration");
    if (!id_item || !duration_item)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (1);
    }
    ft_sharedptr<ft_event> event(new ft_event());
    if (event.get_error() != FT_ERR_SUCCESS)
    {
        json_free_groups(groups);
        this->set_error(event.get_error());
        return (1);
    }
    event->set_id(ft_atoi(id_item->value, ft_nullptr));
    event->set_duration(ft_atoi(duration_item->value, ft_nullptr));
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        json_free_groups(groups);
        this->set_error(this->_world.get_error());
        return (1);
    }
    else if (this->_world && this->_world->get_error() != FT_ERR_SUCCESS)
    {
        json_free_groups(groups);
        this->set_error(this->_world->get_error());
        return (1);
    }
    this->_world->schedule_event(event);
    json_free_groups(groups);
    ft_sharedptr<ft_event_scheduler> scheduler = this->_world->get_event_scheduler();
    if (scheduler.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(scheduler.get_error());
        return (1);
    }
    else if (scheduler && scheduler->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(scheduler->get_error());
        return (1);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

int ft_game_server::serialize_world_locked(ft_string &out, ft_unique_lock<pt_mutex> &guard) const noexcept
{
    (void)guard;
    if (this->_world.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world.get_error());
        return (1);
    }
    else if (this->_world && this->_world->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_world->get_error());
        return (1);
    }
    ft_sharedptr<ft_event_scheduler> scheduler = this->_world->get_event_scheduler();
    if (scheduler.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(scheduler.get_error());
        return (1);
    }
    else if (scheduler && scheduler->get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(scheduler->get_error());
        return (1);
    }
    json_document document;
    json_group *group = serialize_event_scheduler(*scheduler);
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
    this->set_error(FT_ERR_SUCCESS);
    return (0);
}

void ft_game_server::run_once() noexcept
{
    int client_handle;
    ft_string message;
    ft_string update;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (!this->_server)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_server->run_once(client_handle, message) != 0)
    {
        this->set_error(this->_server->get_error());
        return ;
    }
    if (this->handle_message_locked(client_handle, message, guard) != 0)
    {
        return ;
    }
    if (this->serialize_world_locked(update, guard) != 0)
    {
        return ;
    }
    Pair<int, int> *client_ptr = this->_clients.end() - this->_clients.size();
    Pair<int, int> *client_end = this->_clients.end();
    while (client_ptr != client_end)
    {
        this->_server->send_text(client_ptr->value, update);
        client_ptr++;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_server::join_client_locked(int client_id, int client_handle, ft_unique_lock<pt_mutex> &guard) noexcept
{
    (void)guard;
    this->_clients.insert(client_id, client_handle);
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return ;
    }
    if (this->_on_join)
        this->_on_join(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_game_server::leave_client_locked(int client_id, ft_unique_lock<pt_mutex> &guard) noexcept
{
    (void)guard;
    this->_clients.remove(client_id);
    if (this->_clients.get_error() != FT_ERR_SUCCESS)
    {
        this->set_error(this->_clients.get_error());
        return ;
    }
    if (this->_on_leave)
        this->_on_leave(client_id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_game_server::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_server *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_game_server *>(this)->set_error(error_code);
    return (error_code);
}

const char *ft_game_server::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_game_server *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_game_server *>(this)->set_error(error_code);
    return (ft_strerror(error_code));
}
