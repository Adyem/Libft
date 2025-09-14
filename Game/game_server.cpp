#include "game_server.hpp"
#include "../JSon/document.hpp"
#include "../Printf/printf.hpp"

ft_game_server::ft_game_server(ft_world &world) noexcept
    : _server(), _world(&world), _error_code(ER_SUCCESS)
{
    return ;
}

ft_game_server::~ft_game_server()
{
    return ;
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

int ft_game_server::handle_message(const ft_string &message) noexcept
{
    json_group *groups = json_read_from_string(message.c_str());
    if (!groups)
    {
        this->set_error(ft_errno);
        return (1);
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
    ft_string message;
    if (this->_server.run_once(message) != 0)
    {
        this->set_error(this->_server.get_error());
        return ;
    }
    if (this->handle_message(message) != 0)
        return ;
    ft_string update;
    if (this->serialize_world(update) != 0)
        return ;
    pf_printf_fd(1, "%s\n", update.c_str());
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
