#include "game_world.hpp"
#include "game_character.hpp"
#include "game_inventory.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/function.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../Storage/kv_store.hpp"

json_group *serialize_character(const ft_character &character);
int deserialize_character(ft_character &character, json_group *group);
json_group *serialize_event_scheduler(const ft_sharedptr<ft_event_scheduler> &scheduler);
int deserialize_event_scheduler(ft_sharedptr<ft_event_scheduler> &scheduler, json_group *group);
json_group *serialize_inventory(const ft_inventory &inventory);
int deserialize_inventory(ft_inventory &inventory, json_group *group);
json_group *serialize_equipment(const ft_character &character);
int deserialize_equipment(ft_character &character, json_group *group);

static void default_event_callback(ft_world &world, ft_event &event) noexcept;
static ft_function<void(ft_world&, ft_event&)> get_callback_by_id(int type_id) noexcept;

static void default_event_callback(ft_world &world, ft_event &event) noexcept
{
    (void)world;
    (void)event;
    return ;
}

static ft_function<void(ft_world&, ft_event&)> get_callback_by_id(int type_id) noexcept
{
    if (type_id == 1)
        return (ft_function<void(ft_world&, ft_event&)>(default_event_callback));
    return (ft_function<void(ft_world&, ft_event&)>());
}

bool ft_world::propagate_scheduler_state_error() const noexcept
{
    if (!this->_event_scheduler)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    int pointer_error = this->_event_scheduler.get_error();
    if (pointer_error != ER_SUCCESS)
    {
        this->set_error(pointer_error);
        return (true);
    }
    int scheduler_error = this->_event_scheduler->get_error();
    if (scheduler_error != ER_SUCCESS)
    {
        this->set_error(scheduler_error);
        return (true);
    }
    return (false);
}

ft_world::ft_world() noexcept
    : _event_scheduler(new ft_event_scheduler()), _error(ER_SUCCESS)
{
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_world::ft_world(const ft_world &other) noexcept
    : _event_scheduler(other._event_scheduler), _error(other._error)
{
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(other._error);
    return ;
}

ft_world &ft_world::operator=(const ft_world &other) noexcept
{
    if (this != &other)
    {
        this->_event_scheduler = other._event_scheduler;
        if (this->propagate_scheduler_state_error() == true)
            return (*this);
        this->set_error(other._error);
    }
    return (*this);
}

ft_world::ft_world(ft_world &&other) noexcept
    : _event_scheduler(ft_move(other._event_scheduler)), _error(other._error)
{
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    return ;
}

ft_world &ft_world::operator=(ft_world &&other) noexcept
{
    if (this != &other)
    {
        this->_event_scheduler = ft_move(other._event_scheduler);
        if (this->propagate_scheduler_state_error() == true)
            return (*this);
        this->set_error(other._error);
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

void ft_world::schedule_event(const ft_sharedptr<ft_event> &event) noexcept
{
    if (!event)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (event.get_error() != ER_SUCCESS)
    {
        this->set_error(event.get_error());
        return ;
    }
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->_event_scheduler->schedule_event(event);
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_world::update_events(ft_sharedptr<ft_world> &self, int ticks, const char *log_file_path, ft_string *log_buffer) noexcept
{
    if (!self)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->_event_scheduler->update_events(self, ticks, log_file_path, log_buffer);
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

ft_sharedptr<ft_event_scheduler> &ft_world::get_event_scheduler() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_event_scheduler);
}

const ft_sharedptr<ft_event_scheduler> &ft_world::get_event_scheduler() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_event_scheduler);
}

int ft_world::save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept
{
    json_group *groups;
    int error_code;

    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    error_code = ER_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->_error);
    if (json_write_to_file(file_path, groups) != 0)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->_error);
    }
    json_free_groups(groups);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world::load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept
{
    json_group *groups;
    int restore_result;

    groups = json_read_from_file(file_path);
    if (!groups)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->_error);
    }
    restore_result = this->restore_from_groups(groups, character, inventory);
    json_free_groups(groups);
    return (restore_result);
}

int ft_world::save_to_store(kv_store &store, const char *slot_key, const ft_character &character, const ft_inventory &inventory) const noexcept
{
    json_group *groups;
    char *serialized_state;
    int error_code;
    int store_error;

    if (slot_key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->_error);
    }
    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    error_code = ER_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->_error);
    serialized_state = json_write_to_string(groups);
    json_free_groups(groups);
    if (!serialized_state)
    {
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_NO_MEMORY;
        this->set_error(error_code);
        return (this->_error);
    }
    if (store.kv_set(slot_key, serialized_state) != 0)
    {
        store_error = store.get_error();
        cma_free(serialized_state);
        if (store_error == ER_SUCCESS)
            store_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(store_error);
        return (this->_error);
    }
    store_error = store.get_error();
    cma_free(serialized_state);
    if (store_error != ER_SUCCESS)
    {
        this->set_error(store_error);
        return (this->_error);
    }
    if (store.kv_flush() != 0)
    {
        store_error = store.get_error();
        if (store_error == ER_SUCCESS)
            store_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(store_error);
        return (this->_error);
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world::load_from_store(kv_store &store, const char *slot_key, ft_character &character, ft_inventory &inventory) noexcept
{
    const char *serialized_state;
    int store_error;
    json_group *groups;
    int restore_result;

    if (slot_key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->_error);
    }
    serialized_state = store.kv_get(slot_key);
    store_error = store.get_error();
    if (serialized_state == ft_nullptr)
    {
        if (store_error == ER_SUCCESS)
            store_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(store_error);
        return (this->_error);
    }
    groups = json_read_from_string(serialized_state);
    if (!groups)
    {
        int parse_error;

        parse_error = ft_errno;
        if (parse_error == ER_SUCCESS)
            parse_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(parse_error);
        return (this->_error);
    }
    restore_result = this->restore_from_groups(groups, character, inventory);
    json_free_groups(groups);
    return (restore_result);
}

int ft_world::save_to_buffer(ft_string &out_buffer, const ft_character &character, const ft_inventory &inventory) const noexcept
{
    json_group *groups;
    char *serialized_state;
    int error_code;

    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    error_code = ER_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->_error);
    serialized_state = json_write_to_string(groups);
    json_free_groups(groups);
    if (!serialized_state)
    {
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (this->_error);
    }
    out_buffer = serialized_state;
    if (out_buffer.get_error() != ER_SUCCESS)
    {
        int assign_error;

        assign_error = out_buffer.get_error();
        cma_free(serialized_state);
        this->set_error(assign_error);
        return (this->_error);
    }
    cma_free(serialized_state);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

int ft_world::load_from_buffer(const char *buffer, ft_character &character, ft_inventory &inventory) noexcept
{
    json_group *groups;
    int restore_result;
    int parse_error;

    if (buffer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->_error);
    }
    groups = json_read_from_string(buffer);
    if (!groups)
    {
        parse_error = ft_errno;
        if (parse_error == ER_SUCCESS)
            parse_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(parse_error);
        return (this->_error);
    }
    restore_result = this->restore_from_groups(groups, character, inventory);
    json_free_groups(groups);
    return (restore_result);
}

int ft_world::plan_route(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &path) const noexcept
{
    ft_pathfinding finder;
    if (finder.astar_grid(grid, start_x, start_y, start_z,
            goal_x, goal_y, goal_z, path) != ER_SUCCESS)
    {
        this->set_error(finder.get_error());
        return (this->_error);
    }
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
json_group *ft_world::build_snapshot_groups(const ft_character &character,
    const ft_inventory &inventory, int &error_code) const noexcept
{
    json_group *groups;
    json_group *event_group;
    json_group *character_group;
    json_group *inventory_group;
    json_group *equipment_group;

    groups = ft_nullptr;
    error_code = ER_SUCCESS;
    event_group = serialize_event_scheduler(this->_event_scheduler);
    if (!event_group)
    {
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, event_group);
    character_group = serialize_character(character);
    if (!character_group)
    {
        json_free_groups(groups);
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, character_group);
    inventory_group = serialize_inventory(inventory);
    if (!inventory_group)
    {
        json_free_groups(groups);
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, inventory_group);
    equipment_group = serialize_equipment(character);
    if (!equipment_group)
    {
        json_free_groups(groups);
        error_code = ft_errno;
        if (error_code == ER_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, equipment_group);
    this->set_error(ER_SUCCESS);
    error_code = ER_SUCCESS;
    return (groups);
}

int ft_world::restore_from_groups(json_group *groups, ft_character &character,
    ft_inventory &inventory) noexcept
{
    json_group *event_group;
    json_group *character_group;
    json_group *inventory_group;
    json_group *equipment_group;

    if (!groups)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->_error);
    }
    event_group = json_find_group(groups, "world");
    character_group = json_find_group(groups, "character");
    inventory_group = json_find_group(groups, "inventory");
    equipment_group = json_find_group(groups, "equipment");
    if (!event_group || !character_group || !inventory_group || !equipment_group)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->_error);
    }
    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    this->_event_scheduler->clear();
    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    inventory.get_items().clear();
    if (deserialize_event_scheduler(this->_event_scheduler, event_group) != ER_SUCCESS ||
        deserialize_character(character, character_group) != ER_SUCCESS ||
        deserialize_inventory(inventory, inventory_group) != ER_SUCCESS ||
        deserialize_equipment(character, equipment_group) != ER_SUCCESS)
    {
        this->set_error(ft_errno);
        return (this->_error);
    }
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;
    this->_event_scheduler->dump_events(scheduled_events);
    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    this->_event_scheduler->clear();
    if (this->propagate_scheduler_state_error() == true)
        return (this->_error);
    size_t event_index;
    size_t event_count;

    event_index = 0;
    event_count = scheduled_events.size();
    while (event_index < event_count)
    {
        ft_sharedptr<ft_event> &scheduled_event = scheduled_events[event_index];

        scheduled_event->set_callback(get_callback_by_id(scheduled_event->get_id()));
        this->_event_scheduler->schedule_event(scheduled_event);
        if (this->propagate_scheduler_state_error() == true)
            return (this->_error);
        event_index++;
    }
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

