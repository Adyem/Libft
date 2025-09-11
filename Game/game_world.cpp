#include "world.hpp"
#include "character.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"

json_group *serialize_character(const ft_character &character);
int deserialize_character(ft_character &character, json_group *group);

static json_group *serialize_world(const ft_world &world)
{
    json_group *group = json_create_json_group("world");
    if (!group)
        return (ft_nullptr);
    json_item *count_item = json_create_item("event_count", static_cast<int>(world.get_events().size()));
    if (!count_item)
    {
        json_free_groups(group);
        return (ft_nullptr);
    }
    json_add_item_to_group(group, count_item);
    size_t index = 0;
    size_t events_size = world.get_events().size();
    const Pair<int, ft_event> *start = world.get_events().end() - events_size;
    while (index < events_size)
    {
        char *index_string = cma_itoa(static_cast<int>(index));
        if (!index_string)
        {
            json_free_groups(group);
            return (ft_nullptr);
        }
        ft_string key_id = "event_";
        key_id += index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += index_string;
        key_duration += "_duration";
        cma_free(index_string);
        json_item *item_id = json_create_item(key_id.c_str(), start[index].value.get_id());
        if (!item_id)
        {
            json_free_groups(group);
            return (ft_nullptr);
        }
        json_item *item_duration = json_create_item(key_duration.c_str(), start[index].value.get_duration());
        if (!item_duration)
        {
            json_free_groups(group);
            return (ft_nullptr);
        }
        json_add_item_to_group(group, item_id);
        json_add_item_to_group(group, item_duration);
        index++;
    }
    return (group);
}

static int deserialize_world(ft_world &world, json_group *group)
{
    json_item *count_item = json_find_item(group, "event_count");
    if (!count_item)
    {
        ft_errno = GAME_GENERAL_ERROR;
        return (GAME_GENERAL_ERROR);
    }
    int event_count = ft_atoi(count_item->value);
    int index = 0;
    while (index < event_count)
    {
        char *index_string = cma_itoa(index);
        if (!index_string)
            return (JSON_MALLOC_FAIL);
        ft_string key_id = "event_";
        key_id += index_string;
        key_id += "_id";
        ft_string key_duration = "event_";
        key_duration += index_string;
        key_duration += "_duration";
        cma_free(index_string);
        json_item *id_item = json_find_item(group, key_id.c_str());
        json_item *duration_item = json_find_item(group, key_duration.c_str());
        if (!id_item || !duration_item)
        {
            ft_errno = GAME_GENERAL_ERROR;
            return (GAME_GENERAL_ERROR);
        }
        ft_event event;
        event.set_id(ft_atoi(id_item->value));
        event.set_duration(ft_atoi(duration_item->value));
        world.get_events().insert(event.get_id(), event);
        index++;
    }
    return (ER_SUCCESS);
}

ft_world::ft_world() noexcept
    : _events(), _error(ER_SUCCESS)
{
    if (this->_events.get_error() != ER_SUCCESS)
        this->set_error(this->_events.get_error());
    return ;
}

ft_map<int, ft_event> &ft_world::get_events() noexcept
{
    return (this->_events);
}

const ft_map<int, ft_event> &ft_world::get_events() const noexcept
{
    return (this->_events);
}

int ft_world::save_game(const char *file_path, const ft_character &character) const noexcept
{
    json_group *groups = ft_nullptr;
    json_group *world_group = serialize_world(*this);
    if (!world_group)
    {
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_append_group(&groups, world_group);
    json_group *character_group = serialize_character(character);
    if (!character_group)
    {
        json_free_groups(groups);
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_append_group(&groups, character_group);
    if (json_write_to_file(file_path, groups) != 0)
    {
        json_free_groups(groups);
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    json_free_groups(groups);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world::load_game(const char *file_path, ft_character &character) noexcept
{
    json_group *groups = json_read_from_file(file_path);
    if (!groups)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    json_group *world_group = json_find_group(groups, "world");
    json_group *character_group = json_find_group(groups, "character");
    if (!world_group || !character_group)
    {
        json_free_groups(groups);
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    this->_events.clear();
    if (deserialize_world(*this, world_group) != ER_SUCCESS ||
        deserialize_character(character, character_group) != ER_SUCCESS)
    {
        json_free_groups(groups);
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_free_groups(groups);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_world::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_world::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
