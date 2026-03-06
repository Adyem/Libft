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
#include "../System_utils/system_utils.hpp"
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
static void destroy_world_lifecycle_components(
    ft_sharedptr<ft_event_scheduler> &event_scheduler,
    ft_sharedptr<ft_world_registry> &world_registry,
    ft_sharedptr<ft_world_replay_session> &replay_session,
    ft_sharedptr<ft_economy_table> &economy_table,
    ft_sharedptr<ft_crafting> &crafting,
    ft_sharedptr<ft_dialogue_table> &dialogue_table,
    ft_sharedptr<ft_world_region> &world_region,
    ft_sharedptr<ft_quest> &quest,
    ft_sharedptr<ft_vendor_profile> &vendor_profile,
    ft_sharedptr<ft_upgrade> &upgrade) noexcept;

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

static void destroy_world_lifecycle_components(
    ft_sharedptr<ft_event_scheduler> &event_scheduler,
    ft_sharedptr<ft_world_registry> &world_registry,
    ft_sharedptr<ft_world_replay_session> &replay_session,
    ft_sharedptr<ft_economy_table> &economy_table,
    ft_sharedptr<ft_crafting> &crafting,
    ft_sharedptr<ft_dialogue_table> &dialogue_table,
    ft_sharedptr<ft_world_region> &world_region,
    ft_sharedptr<ft_quest> &quest,
    ft_sharedptr<ft_vendor_profile> &vendor_profile,
    ft_sharedptr<ft_upgrade> &upgrade) noexcept
{
    if (replay_session.get() != ft_nullptr)
        (void)replay_session->destroy();
    if (upgrade.get() != ft_nullptr)
        (void)upgrade->destroy();
    if (vendor_profile.get() != ft_nullptr)
        (void)vendor_profile->destroy();
    if (quest.get() != ft_nullptr)
        (void)quest->destroy();
    if (world_region.get() != ft_nullptr)
        (void)world_region->destroy();
    if (dialogue_table.get() != ft_nullptr)
        (void)dialogue_table->destroy();
    if (crafting.get() != ft_nullptr)
        (void)crafting->destroy();
    if (economy_table.get() != ft_nullptr)
        (void)economy_table->destroy();
    if (world_registry.get() != ft_nullptr)
        (void)world_registry->destroy();
    if (event_scheduler.get() != ft_nullptr)
        (void)event_scheduler->destroy();
    event_scheduler.reset();
    world_registry.reset();
    replay_session.reset();
    economy_table.reset();
    crafting.reset();
    dialogue_table.reset();
    world_region.reset();
    quest.reset();
    vendor_profile.reset();
    upgrade.reset();
    return ;
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
    : _event_scheduler(), _world_registry(), _replay_session(),
    _economy_table(), _crafting(), _dialogue_table(), _world_region(), _quest(),
    _vendor_profile(), _upgrade(),
    _initialized_state(ft_world::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_world::~ft_world() noexcept
{
    if (this->_initialized_state == ft_world::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_world::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    static const char prefix[] = "ft_world lifecycle error: ";
    static const char separator[] = ": ";
    static const char suffix[] = "\n";

    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    (void)su_write(2, prefix, sizeof(prefix) - 1);
    (void)su_write(2, method_name, ft_strlen_size_t(method_name));
    (void)su_write(2, separator, sizeof(separator) - 1);
    (void)su_write(2, reason, ft_strlen_size_t(reason));
    (void)su_write(2, suffix, sizeof(suffix) - 1);
    su_abort();
    return ;
}

void ft_world::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_world::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_world::initialize() noexcept
{
    int initialize_error;

    if (this->_initialized_state == ft_world::_state_initialized)
    {
        this->abort_lifecycle_error("ft_world::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_event_scheduler = ft_sharedptr<ft_event_scheduler>(
        new (std::nothrow) ft_event_scheduler());
    this->_world_registry = ft_sharedptr<ft_world_registry>(
        new (std::nothrow) ft_world_registry());
    this->_replay_session = ft_sharedptr<ft_world_replay_session>(
        new (std::nothrow) ft_world_replay_session());
    this->_economy_table = ft_sharedptr<ft_economy_table>(
        new (std::nothrow) ft_economy_table());
    this->_crafting = ft_sharedptr<ft_crafting>(new (std::nothrow) ft_crafting());
    this->_dialogue_table = ft_sharedptr<ft_dialogue_table>(
        new (std::nothrow) ft_dialogue_table());
    this->_world_region = ft_sharedptr<ft_world_region>(
        new (std::nothrow) ft_world_region());
    this->_quest = ft_sharedptr<ft_quest>(new (std::nothrow) ft_quest());
    this->_vendor_profile = ft_sharedptr<ft_vendor_profile>(
        new (std::nothrow) ft_vendor_profile());
    this->_upgrade = ft_sharedptr<ft_upgrade>(new (std::nothrow) ft_upgrade());
    if (!this->_event_scheduler || !this->_world_registry || !this->_replay_session
        || !this->_economy_table || !this->_crafting || !this->_dialogue_table
        || !this->_world_region || !this->_quest || !this->_vendor_profile
        || !this->_upgrade)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = this->_event_scheduler->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_replay_session->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_economy_table->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_crafting->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_dialogue_table->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_world_region->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_quest->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_vendor_profile->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_upgrade->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_world_registry->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        destroy_world_lifecycle_components(this->_event_scheduler,
            this->_world_registry, this->_replay_session, this->_economy_table,
            this->_crafting, this->_dialogue_table, this->_world_region,
            this->_quest, this->_vendor_profile, this->_upgrade);
        this->_initialized_state = ft_world::_state_destroyed;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_initialized_state = ft_world::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world::destroy() noexcept
{
    if (this->_initialized_state != ft_world::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_world_lifecycle_components(this->_event_scheduler,
        this->_world_registry, this->_replay_session, this->_economy_table,
        this->_crafting, this->_dialogue_table, this->_world_region,
        this->_quest, this->_vendor_profile, this->_upgrade);
    this->_initialized_state = ft_world::_state_destroyed;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_world::schedule_event(const ft_sharedptr<ft_event> &event) noexcept
{
    this->abort_if_not_initialized("ft_world::schedule_event");
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
    this->abort_if_not_initialized("ft_world::update_events");
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

    this->abort_if_not_initialized("ft_world::get_event_scheduler");
    previous_error = this->get_error();
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_event_scheduler);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_event_scheduler);
}

const ft_sharedptr<ft_event_scheduler> &ft_world::get_event_scheduler() const noexcept
{
    int previous_error;

    this->abort_if_not_initialized("ft_world::get_event_scheduler const");
    previous_error = this->get_error();
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_event_scheduler);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_event_scheduler);
}

ft_sharedptr<ft_world_registry> &ft_world::get_world_registry() noexcept
{
    this->abort_if_not_initialized("ft_world::get_world_registry");
    if (this->propagate_registry_state_error() == true)
        return (this->_world_registry);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_registry);
}

const ft_sharedptr<ft_world_registry> &ft_world::get_world_registry() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_world_registry const");
    if (this->propagate_registry_state_error() == true)
        return (this->_world_registry);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_registry);
}

ft_sharedptr<ft_world_replay_session> &ft_world::get_replay_session() noexcept
{
    this->abort_if_not_initialized("ft_world::get_replay_session");
    if (this->propagate_replay_state_error() == true)
        return (this->_replay_session);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_replay_session);
}

const ft_sharedptr<ft_world_replay_session> &ft_world::get_replay_session() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_replay_session const");
    if (this->propagate_replay_state_error() == true)
        return (this->_replay_session);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_replay_session);
}

ft_sharedptr<ft_economy_table> &ft_world::get_economy_table() noexcept
{
    this->abort_if_not_initialized("ft_world::get_economy_table");
    if (this->propagate_economy_state_error() == true)
        return (this->_economy_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_economy_table);
}

const ft_sharedptr<ft_economy_table> &ft_world::get_economy_table() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_economy_table const");
    if (this->propagate_economy_state_error() == true)
        return (this->_economy_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_economy_table);
}

ft_sharedptr<ft_crafting> &ft_world::get_crafting() noexcept
{
    this->abort_if_not_initialized("ft_world::get_crafting");
    if (this->propagate_crafting_state_error() == true)
        return (this->_crafting);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_crafting);
}

const ft_sharedptr<ft_crafting> &ft_world::get_crafting() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_crafting const");
    if (this->propagate_crafting_state_error() == true)
        return (this->_crafting);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_crafting);
}

ft_sharedptr<ft_dialogue_table> &ft_world::get_dialogue_table() noexcept
{
    this->abort_if_not_initialized("ft_world::get_dialogue_table");
    if (this->propagate_dialogue_state_error() == true)
        return (this->_dialogue_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_dialogue_table);
}

const ft_sharedptr<ft_dialogue_table> &ft_world::get_dialogue_table() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_dialogue_table const");
    if (this->propagate_dialogue_state_error() == true)
        return (this->_dialogue_table);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_dialogue_table);
}

ft_sharedptr<ft_world_region> &ft_world::get_world_region() noexcept
{
    this->abort_if_not_initialized("ft_world::get_world_region");
    if (this->propagate_region_state_error() == true)
        return (this->_world_region);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_region);
}

const ft_sharedptr<ft_world_region> &ft_world::get_world_region() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_world_region const");
    if (this->propagate_region_state_error() == true)
        return (this->_world_region);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_region);
}

ft_sharedptr<ft_quest> &ft_world::get_quest() noexcept
{
    int previous_error;

    this->abort_if_not_initialized("ft_world::get_quest");
    previous_error = this->get_error();
    if (this->propagate_quest_state_error() == true)
        return (this->_quest);
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_quest);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_quest);
}

const ft_sharedptr<ft_quest> &ft_world::get_quest() const noexcept
{
    int previous_error;

    this->abort_if_not_initialized("ft_world::get_quest const");
    previous_error = this->get_error();
    if (this->propagate_quest_state_error() == true)
        return (this->_quest);
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_quest);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_quest);
}

ft_sharedptr<ft_vendor_profile> &ft_world::get_vendor_profile() noexcept
{
    int previous_error;

    this->abort_if_not_initialized("ft_world::get_vendor_profile");
    previous_error = this->get_error();
    if (this->propagate_vendor_profile_state_error() == true)
        return (this->_vendor_profile);
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_vendor_profile);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profile);
}

const ft_sharedptr<ft_vendor_profile> &ft_world::get_vendor_profile() const noexcept
{
    int previous_error;

    this->abort_if_not_initialized("ft_world::get_vendor_profile const");
    previous_error = this->get_error();
    if (this->propagate_vendor_profile_state_error() == true)
        return (this->_vendor_profile);
    if (previous_error != FT_ERR_SUCCESS)
    {
        this->set_error(previous_error);
        return (this->_vendor_profile);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_vendor_profile);
}

ft_sharedptr<ft_upgrade> &ft_world::get_upgrade() noexcept
{
    this->abort_if_not_initialized("ft_world::get_upgrade");
    if (this->propagate_upgrade_state_error() == true)
        return (this->_upgrade);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_upgrade);
}

const ft_sharedptr<ft_upgrade> &ft_world::get_upgrade() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_upgrade const");
    if (this->propagate_upgrade_state_error() == true)
        return (this->_upgrade);
    this->set_error(FT_ERR_SUCCESS);
    return (this->_upgrade);
}

int ft_world::save_to_file(const char *file_path, const ft_character &character, const ft_inventory &inventory) const noexcept
{
    json_group *groups;
    int error_code;

    this->abort_if_not_initialized("ft_world::save_to_file");
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

    this->abort_if_not_initialized("ft_world::load_from_file");
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
    this->abort_if_not_initialized("ft_world::save_to_store");
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

    this->abort_if_not_initialized("ft_world::load_from_store");
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

    this->abort_if_not_initialized("ft_world::save_to_buffer");
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
    if (ft_string::get_error() != FT_ERR_SUCCESS)
    {
        int assign_error;

        assign_error = ft_string::get_error();
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

    this->abort_if_not_initialized("ft_world::load_from_buffer");
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
    this->abort_if_not_initialized("ft_world::plan_route");
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
    this->abort_if_not_initialized("ft_world::get_error");
    return (ft_world::_last_error);
}

const char *ft_world::get_error_str() const noexcept
{
    this->abort_if_not_initialized("ft_world::get_error_str");
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
