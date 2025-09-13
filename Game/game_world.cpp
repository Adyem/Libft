#include "world.hpp"
#include "character.hpp"
#include "inventory.hpp"
#include "../JSon/json.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string_class.hpp"

json_group *serialize_character(const ft_character &character);
int deserialize_character(ft_character &character, json_group *group);
json_group *serialize_world(const ft_world &world);
int deserialize_world(ft_world &world, json_group *group);
json_group *serialize_inventory(const ft_inventory &inventory);
int deserialize_inventory(ft_inventory &inventory, json_group *group);
json_group *serialize_equipment(const ft_character &character);
int deserialize_equipment(ft_character &character, json_group *group);

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

int ft_world::save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept
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
    json_group *inventory_group = serialize_inventory(inventory);
    if (!inventory_group)
    {
        json_free_groups(groups);
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_append_group(&groups, inventory_group);
    json_group *equipment_group = serialize_equipment(character);
    if (!equipment_group)
    {
        json_free_groups(groups);
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_append_group(&groups, equipment_group);
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

int ft_world::load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept
{
    json_group *groups = json_read_from_file(file_path);
    if (!groups)
    {
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    json_group *world_group = json_find_group(groups, "world");
    json_group *character_group = json_find_group(groups, "character");
    json_group *inventory_group = json_find_group(groups, "inventory");
    json_group *equipment_group = json_find_group(groups, "equipment");
    if (!world_group || !character_group || !inventory_group || !equipment_group)
    {
        json_free_groups(groups);
        this->set_error(GAME_GENERAL_ERROR);
        return (this->_error);
    }
    this->_events.clear();
    inventory.get_items().clear();
    if (deserialize_world(*this, world_group) != ER_SUCCESS ||
        deserialize_character(character, character_group) != ER_SUCCESS ||
        deserialize_inventory(inventory, inventory_group) != ER_SUCCESS ||
        deserialize_equipment(character, equipment_group) != ER_SUCCESS)
    {
        json_free_groups(groups);
        this->set_error(ft_errno);
        return (this->_error);
    }
    json_free_groups(groups);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
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
