#include "game_world.hpp"
#include "game_character.hpp"
#include "game_inventory.hpp"
#include "game_world_registry.hpp"
#include "game_world_replay.hpp"
#include "game_economy_table.hpp"
#include "game_crafting.hpp"
#include "game_dialogue_table.hpp"
#include "ft_world_region.hpp"
#include "game_quest.hpp"
#include "ft_vendor_profile.hpp"
#include "game_upgrade.hpp"
#include "../JSon/json.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Template/function.hpp"
#include <utility>
#include <new>
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

thread_local int ft_world::_last_error = FT_ERR_SUCCESS;

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
    int scheduler_error = this->_event_scheduler->get_error();
    if (scheduler_error != FT_ERR_SUCCESS)
    {
        this->set_error(scheduler_error);
        return (true);
    }
    return (false);
}

ft_world::ft_world() noexcept
    : _event_scheduler(new (std::nothrow) ft_event_scheduler()),
    _world_registry(new (std::nothrow) ft_world_registry()),
    _replay_session(new (std::nothrow) ft_world_replay_session()),
    _economy_table(new (std::nothrow) ft_economy_table()),
    _crafting(new (std::nothrow) ft_crafting()),
    _dialogue_table(new (std::nothrow) ft_dialogue_table()),
    _world_region(new (std::nothrow) ft_world_region()),
    _quest(new (std::nothrow) ft_quest()),
    _vendor_profile(new (std::nothrow) ft_vendor_profile()),
    _upgrade(new (std::nothrow) ft_upgrade())
{
    if (!this->_event_scheduler)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_world_registry)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_replay_session)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_economy_table)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_crafting)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_dialogue_table)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_world_region)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_quest)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_vendor_profile)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (!this->_upgrade)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (this->propagate_scheduler_state_error() == true)
        return ;
    if (this->propagate_registry_state_error() == true)
        return ;
    if (this->propagate_replay_state_error() == true)
        return ;
    if (this->propagate_economy_state_error() == true)
        return ;
    if (this->propagate_crafting_state_error() == true)
        return ;
    if (this->propagate_dialogue_state_error() == true)
        return ;
    if (this->propagate_region_state_error() == true)
        return ;
    if (this->propagate_quest_state_error() == true)
        return ;
    if (this->propagate_vendor_profile_state_error() == true)
        return ;
    if (this->propagate_upgrade_state_error() == true)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_world::~ft_world() noexcept
{
    return ;
}

void ft_world::schedule_event(const ft_sharedptr<ft_event> &event) noexcept
{
    if (!event)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return ;
    }
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->_event_scheduler->schedule_event(event);
    if (this->propagate_scheduler_state_error() == true)
        return ;
    this->set_error(FT_ERR_SUCCESS);
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
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_sharedptr<ft_event_scheduler> &ft_world::get_event_scheduler() noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_event_scheduler);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_event_scheduler);
}

const ft_sharedptr<ft_event_scheduler> &ft_world::get_event_scheduler() const noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_event_scheduler);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_event_scheduler);
}

ft_sharedptr<ft_world_registry> &ft_world::get_world_registry() noexcept
{
    if (this->propagate_registry_state_error() == true)
        return (this->_world_registry);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_registry);
}

const ft_sharedptr<ft_world_registry> &ft_world::get_world_registry() const noexcept
{
    if (this->propagate_registry_state_error() == true)
        return (this->_world_registry);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_registry);
}

ft_sharedptr<ft_world_replay_session> &ft_world::get_replay_session() noexcept
{
    if (this->propagate_replay_state_error() == true)
        return (this->_replay_session);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_replay_session);
}

const ft_sharedptr<ft_world_replay_session> &ft_world::get_replay_session() const noexcept
{
    if (this->propagate_replay_state_error() == true)
        return (this->_replay_session);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_replay_session);
}

ft_sharedptr<ft_economy_table> &ft_world::get_economy_table() noexcept
{
    if (this->propagate_economy_state_error() == true)
        return (this->_economy_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_economy_table);
}

const ft_sharedptr<ft_economy_table> &ft_world::get_economy_table() const noexcept
{
    if (this->propagate_economy_state_error() == true)
        return (this->_economy_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_economy_table);
}

ft_sharedptr<ft_crafting> &ft_world::get_crafting() noexcept
{
    if (this->propagate_crafting_state_error() == true)
        return (this->_crafting);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_crafting);
}

const ft_sharedptr<ft_crafting> &ft_world::get_crafting() const noexcept
{
    if (this->propagate_crafting_state_error() == true)
        return (this->_crafting);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_crafting);
}

ft_sharedptr<ft_dialogue_table> &ft_world::get_dialogue_table() noexcept
{
    if (this->propagate_dialogue_state_error() == true)
        return (this->_dialogue_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_dialogue_table);
}

const ft_sharedptr<ft_dialogue_table> &ft_world::get_dialogue_table() const noexcept
{
    if (this->propagate_dialogue_state_error() == true)
        return (this->_dialogue_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_dialogue_table);
}

ft_sharedptr<ft_world_region> &ft_world::get_world_region() noexcept
{
    if (this->propagate_region_state_error() == true)
        return (this->_world_region);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_region);
}

const ft_sharedptr<ft_world_region> &ft_world::get_world_region() const noexcept
{
    if (this->propagate_region_state_error() == true)
        return (this->_world_region);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_region);
}

ft_sharedptr<ft_quest> &ft_world::get_quest() noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (this->propagate_quest_state_error() == true)
        return (this->_quest);
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_quest);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_quest);
}

const ft_sharedptr<ft_quest> &ft_world::get_quest() const noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (this->propagate_quest_state_error() == true)
        return (this->_quest);
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_quest);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_quest);
}

ft_sharedptr<ft_vendor_profile> &ft_world::get_vendor_profile() noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (this->propagate_vendor_profile_state_error() == true)
        return (this->_vendor_profile);
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_vendor_profile);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profile);
}

const ft_sharedptr<ft_vendor_profile> &ft_world::get_vendor_profile() const noexcept
{
    int previous_error;

    previous_error = this->get_error();
    if (this->propagate_vendor_profile_state_error() == true)
        return (this->_vendor_profile);
    if (previous_error != FT_ERR_SUCCESS)
        return (this->_vendor_profile);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profile);
}

ft_sharedptr<ft_upgrade> &ft_world::get_upgrade() noexcept
{
    if (this->propagate_upgrade_state_error() == true)
        return (this->_upgrade);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_upgrade);
}

const ft_sharedptr<ft_upgrade> &ft_world::get_upgrade() const noexcept
{
    if (this->propagate_upgrade_state_error() == true)
        return (this->_upgrade);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_upgrade);
}

int ft_world::save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept
{
    json_group *groups;
    int error_code;

    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
    error_code = FT_ERR_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->get_error());
    if (json_write_to_file(file_path, groups) != 0)
    {
        json_free_groups(groups);
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }
    json_free_groups(groups);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world::load_from_file(const char *file_path, ft_character &character, ft_inventory &inventory) noexcept
{
    json_group *groups;
    int restore_result;

    groups = json_read_from_file(file_path);
    if (!groups)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
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
    if (slot_key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->get_error());
    }
    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
    error_code = FT_ERR_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->get_error());
    serialized_state = json_write_to_string(groups);
    json_free_groups(groups);
    if (!serialized_state)
    {
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_NO_MEMORY;
        this->set_error(error_code);
        return (this->get_error());
    }
    if (store.kv_set(slot_key, serialized_state) != 0)
    {        cma_free(serialized_state);        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }    cma_free(serialized_state);
    if (store.kv_flush() != 0)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world::load_from_store(kv_store &store, const char *slot_key, ft_character &character, ft_inventory &inventory) noexcept
{
    const char *serialized_state;
    json_group *groups;
    int restore_result;

    if (slot_key == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->get_error());
    }
    serialized_state = store.kv_get(slot_key);
    if (serialized_state == ft_nullptr)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }
    groups = json_read_from_string(serialized_state);
    if (!groups)
    {
        int parse_error;

        parse_error = FT_ERR_GAME_GENERAL_ERROR;
        if (parse_error == FT_ERR_SUCCESS)
            parse_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(parse_error);
        return (this->get_error());
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
        return (this->get_error());
    error_code = FT_ERR_SUCCESS;
    groups = this->build_snapshot_groups(character, inventory, error_code);
    if (!groups)
        return (this->get_error());
    serialized_state = json_write_to_string(groups);
    json_free_groups(groups);
    if (!serialized_state)
    {
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (this->get_error());
    }
    out_buffer = serialized_state;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        int assign_error;

        assign_error = ft_string::last_operation_error();
        cma_free(serialized_state);
        this->set_error(assign_error);
        return (this->get_error());
    }
    cma_free(serialized_state);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world::load_from_buffer(const char *buffer, ft_character &character, ft_inventory &inventory) noexcept
{
    json_group *groups;
    int restore_result;
    int parse_error;

    if (buffer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (this->get_error());
    }
    groups = json_read_from_string(buffer);
    if (!groups)
    {
        parse_error = FT_ERR_GAME_GENERAL_ERROR;
        if (parse_error == FT_ERR_SUCCESS)
            parse_error = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(parse_error);
        return (this->get_error());
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
            goal_x, goal_y, goal_z, path) != FT_ERR_SUCCESS)
    {
        this->set_error(finder.get_error());
        return (this->get_error());
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world::get_error() const noexcept
{
    return (ft_world::_last_error);
}

const char *ft_world::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

void ft_world::set_error(int err) const noexcept
{
    ft_world::_last_error = err;
    return ;
}

bool ft_world::propagate_registry_state_error() const noexcept
{
    if (!this->_world_registry)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_replay_state_error() const noexcept
{
    if (!this->_replay_session)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_economy_state_error() const noexcept
{
    if (!this->_economy_table)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_crafting_state_error() const noexcept
{
    if (!this->_crafting)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    int crafting_error;

    crafting_error = this->_crafting->get_error();
    if (crafting_error != FT_ERR_SUCCESS)
    {
        this->set_error(crafting_error);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_dialogue_state_error() const noexcept
{
    if (!this->_dialogue_table)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_region_state_error() const noexcept
{
    if (!this->_world_region)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_quest_state_error() const noexcept
{
    if (!this->_quest)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_vendor_profile_state_error() const noexcept
{
    if (!this->_vendor_profile)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
}

bool ft_world::propagate_upgrade_state_error() const noexcept
{
    if (!this->_upgrade)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (true);
    }
    return (false);
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
    error_code = FT_ERR_SUCCESS;
    event_group = serialize_event_scheduler(this->_event_scheduler);
    if (!event_group)
    {
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, event_group);
    character_group = serialize_character(character);
    if (!character_group)
    {
        json_free_groups(groups);
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, character_group);
    inventory_group = serialize_inventory(inventory);
    if (!inventory_group)
    {
        json_free_groups(groups);
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, inventory_group);
    equipment_group = serialize_equipment(character);
    if (!equipment_group)
    {
        json_free_groups(groups);
        error_code = FT_ERR_GAME_GENERAL_ERROR;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_GAME_GENERAL_ERROR;
        this->set_error(error_code);
        return (ft_nullptr);
    }
    json_append_group(&groups, equipment_group);
    this->set_error(FT_ERR_SUCCESS);
    error_code = FT_ERR_SUCCESS;
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
        return (this->get_error());
    }
    event_group = json_find_group(groups, "world");
    character_group = json_find_group(groups, "character");
    inventory_group = json_find_group(groups, "inventory");
    equipment_group = json_find_group(groups, "equipment");
    if (!event_group || !character_group || !inventory_group || !equipment_group)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }
    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
    this->_event_scheduler->clear();
    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
    inventory.get_items().clear();
    if (deserialize_event_scheduler(this->_event_scheduler, event_group) != FT_ERR_SUCCESS ||
        deserialize_character(character, character_group) != FT_ERR_SUCCESS ||
        deserialize_inventory(inventory, inventory_group) != FT_ERR_SUCCESS ||
        deserialize_equipment(character, equipment_group) != FT_ERR_SUCCESS)
    {
        this->set_error(FT_ERR_GAME_GENERAL_ERROR);
        return (this->get_error());
    }
    ft_vector<ft_sharedptr<ft_event> > scheduled_events;
    this->_event_scheduler->dump_events(scheduled_events);
    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
    this->_event_scheduler->clear();
    if (this->propagate_scheduler_state_error() == true)
        return (this->get_error());
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
            return (this->get_error());
        event_index++;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}
