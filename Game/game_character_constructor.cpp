#include "game_character.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
#include <new>

thread_local uint32_t game_character::_last_error = FT_ERR_SUCCESS;

ft_bool game_character::handle_component_error(int32_t error) noexcept
{
    if (error == FT_ERR_SUCCESS)
        return (FT_FALSE);
    this->set_error(error);
    return (FT_TRUE);
}

ft_bool game_character::check_internal_errors() noexcept
{
    if (this->handle_component_error(this->_equipment.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_fire_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_frost_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_lightning_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_air_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_earth_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_chaos_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    if (this->handle_component_error(this->_physical_res.get_error()) == FT_TRUE)
        return (FT_TRUE);
    return (FT_FALSE);
}

game_character::game_character() noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res(), _frost_res(), _lightning_res(),
      _air_res(), _earth_res(), _chaos_res(),
      _physical_res(), _skills(), _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_character::game_character(const game_character &other) noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res(), _frost_res(), _lightning_res(),
      _air_res(), _earth_res(), _chaos_res(),
      _physical_res(), _skills(), _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;
    int32_t level_count;
    int32_t level_index;
    int32_t transfer_error;
    ft_sharedptr<game_item> equipped_item;

    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_character::game_character(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return ;
    }
    this->_hit_points = other._hit_points;
    this->_physical_armor = other._physical_armor;
    this->_magic_armor = other._magic_armor;
    this->_current_physical_armor = other._current_physical_armor;
    this->_current_magic_armor = other._current_magic_armor;
    this->_damage_rule = other._damage_rule;
    this->_might = other._might;
    this->_agility = other._agility;
    this->_endurance = other._endurance;
    this->_reason = other._reason;
    this->_insigh = other._insigh;
    this->_presence = other._presence;
    this->_coins = other._coins;
    this->_valor = other._valor;
    this->_experience = other._experience;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_physical_damage_multiplier = other._physical_damage_multiplier;
    this->_magic_damage_multiplier = other._magic_damage_multiplier;
    transfer_error = this->_fire_res.set_values(other._fire_res.get_percent(), other._fire_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_frost_res.set_values(other._frost_res.get_percent(), other._frost_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_lightning_res.set_values(other._lightning_res.get_percent(), other._lightning_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_air_res.set_values(other._air_res.get_percent(), other._air_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_earth_res.set_values(other._earth_res.get_percent(), other._earth_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_chaos_res.set_values(other._chaos_res.get_percent(), other._chaos_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_physical_res.set_values(other._physical_res.get_percent(), other._physical_res.get_flat());
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    this->_skills = other._skills;
    this->_buffs = other._buffs;
    this->_debuffs = other._debuffs;
    this->_upgrades = other._upgrades;
    this->_quests = other._quests;
    this->_achievements = other._achievements;
    transfer_error = this->_reputation.destroy();
    if (transfer_error == FT_ERR_SUCCESS)
        transfer_error = this->_reputation.initialize(other._reputation);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    transfer_error = this->_inventory.destroy();
    if (transfer_error == FT_ERR_SUCCESS)
        transfer_error = this->_inventory.initialize(other._inventory);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    level_count = other._experience_table.get_count();
    transfer_error = this->_experience_table.resize(level_count);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    level_index = 0;
    while (level_index < level_count)
    {
        this->_experience_table.set_value(level_index, other._experience_table.get_value(level_index));
        transfer_error = this->_experience_table.get_error();
        if (transfer_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->set_error(transfer_error);
            return ;
        }
        level_index += 1;
    }
    equipped_item = other._equipment.get_item(EQUIP_HEAD);
    transfer_error = other._equipment.get_error();
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    if (equipped_item)
    {
        transfer_error = this->_equipment.equip(EQUIP_HEAD, equipped_item);
        if (transfer_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->set_error(transfer_error);
            return ;
        }
    }
    equipped_item = other._equipment.get_item(EQUIP_CHEST);
    transfer_error = other._equipment.get_error();
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    if (equipped_item)
    {
        transfer_error = this->_equipment.equip(EQUIP_CHEST, equipped_item);
        if (transfer_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->set_error(transfer_error);
            return ;
        }
    }
    equipped_item = other._equipment.get_item(EQUIP_WEAPON);
    transfer_error = other._equipment.get_error();
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return ;
    }
    if (equipped_item)
    {
        transfer_error = this->_equipment.equip(EQUIP_WEAPON, equipped_item);
        if (transfer_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->set_error(transfer_error);
            return ;
        }
    }
    this->set_error(other.get_error());
    return ;
}

game_character::game_character(game_character &&other) noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res(), _frost_res(), _lightning_res(),
      _air_res(), _earth_res(), _chaos_res(),
      _physical_res(), _skills(), _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    this->set_error(FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_character::game_character(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

int32_t game_character::initialize() noexcept
{
    int32_t initialize_error;
    ft_bool fire_initialised;
    ft_bool frost_initialised;
    ft_bool lightning_initialised;
    ft_bool air_initialised;
    ft_bool earth_initialised;
    ft_bool chaos_initialised;
    ft_bool physical_initialised;
    ft_bool reputation_initialised;
    ft_bool inventory_initialised;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_character::initialize",
            "called while object is already initialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    fire_initialised = FT_FALSE;
    frost_initialised = FT_FALSE;
    lightning_initialised = FT_FALSE;
    air_initialised = FT_FALSE;
    earth_initialised = FT_FALSE;
    chaos_initialised = FT_FALSE;
    physical_initialised = FT_FALSE;
    reputation_initialised = FT_FALSE;
    inventory_initialised = FT_FALSE;
    initialize_error = this->_fire_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    fire_initialised = FT_TRUE;
    initialize_error = this->_frost_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    frost_initialised = FT_TRUE;
    initialize_error = this->_lightning_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    lightning_initialised = FT_TRUE;
    initialize_error = this->_air_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    air_initialised = FT_TRUE;
    initialize_error = this->_earth_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    earth_initialised = FT_TRUE;
    initialize_error = this->_chaos_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    chaos_initialised = FT_TRUE;
    initialize_error = this->_physical_res.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    physical_initialised = FT_TRUE;
    initialize_error = this->_reputation.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    reputation_initialised = FT_TRUE;
    initialize_error = this->_inventory.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    inventory_initialised = FT_TRUE;
    initialize_error = this->_experience_table.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_skills.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_experience_table.destroy();
        if (inventory_initialised == FT_TRUE)
            (void)this->_inventory.destroy();
        if (reputation_initialised == FT_TRUE)
            (void)this->_reputation.destroy();
        if (physical_initialised == FT_TRUE)
            (void)this->_physical_res.destroy();
        if (chaos_initialised == FT_TRUE)
            (void)this->_chaos_res.destroy();
        if (earth_initialised == FT_TRUE)
            (void)this->_earth_res.destroy();
        if (air_initialised == FT_TRUE)
            (void)this->_air_res.destroy();
        if (lightning_initialised == FT_TRUE)
            (void)this->_lightning_res.destroy();
        if (frost_initialised == FT_TRUE)
            (void)this->_frost_res.destroy();
        if (fire_initialised == FT_TRUE)
            (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_buffs.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_skills.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_debuffs.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_skills.destroy();
        (void)this->_buffs.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_upgrades.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_skills.destroy();
        (void)this->_buffs.destroy();
        (void)this->_debuffs.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_quests.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_skills.destroy();
        (void)this->_buffs.destroy();
        (void)this->_debuffs.destroy();
        (void)this->_upgrades.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_achievements.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_skills.destroy();
        (void)this->_buffs.destroy();
        (void)this->_debuffs.destroy();
        (void)this->_upgrades.destroy();
        (void)this->_quests.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    initialize_error = this->_equipment.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->_achievements.destroy();
        (void)this->_quests.destroy();
        (void)this->_upgrades.destroy();
        (void)this->_debuffs.destroy();
        (void)this->_buffs.destroy();
        (void)this->_skills.destroy();
        (void)this->_experience_table.destroy();
        (void)this->_inventory.destroy();
        (void)this->_reputation.destroy();
        (void)this->_physical_res.destroy();
        (void)this->_chaos_res.destroy();
        (void)this->_earth_res.destroy();
        (void)this->_air_res.destroy();
        (void)this->_lightning_res.destroy();
        (void)this->_frost_res.destroy();
        (void)this->_fire_res.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

game_character::~game_character() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t game_character::destroy() noexcept
{
    int32_t first_error;
    int32_t current_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    first_error = FT_ERR_SUCCESS;
    current_error = this->disable_thread_safety();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_achievements.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_quests.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_upgrades.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_debuffs.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_buffs.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_skills.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_experience_table.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_equipment.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_inventory.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_reputation.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_physical_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_chaos_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_earth_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_air_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_lightning_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_frost_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    current_error = this->_fire_res.destroy();
    if (first_error == FT_ERR_SUCCESS && current_error != FT_ERR_SUCCESS)
        first_error = current_error;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(first_error);
    return (first_error);
}

int32_t game_character::move(game_character &other) noexcept
{
    int32_t destroy_error;
    int32_t initialize_error;
    int32_t transfer_error;
    int32_t source_destroy_error;
    int32_t source_error;
    int32_t level_count;
    int32_t level_index;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_character::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_hit_points = other._hit_points;
    this->_physical_armor = other._physical_armor;
    this->_magic_armor = other._magic_armor;
    this->_current_physical_armor = other._current_physical_armor;
    this->_current_magic_armor = other._current_magic_armor;
    this->_damage_rule = other._damage_rule;
    this->_might = other._might;
    this->_agility = other._agility;
    this->_endurance = other._endurance;
    this->_reason = other._reason;
    this->_insigh = other._insigh;
    this->_presence = other._presence;
    this->_coins = other._coins;
    this->_valor = other._valor;
    this->_experience = other._experience;
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_physical_damage_multiplier = other._physical_damage_multiplier;
    this->_magic_damage_multiplier = other._magic_damage_multiplier;
    transfer_error = this->_fire_res.move(other._fire_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_frost_res.move(other._frost_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_lightning_res.move(other._lightning_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_air_res.move(other._air_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_earth_res.move(other._earth_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_chaos_res.move(other._chaos_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_physical_res.move(other._physical_res);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_skills.move(other._skills));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_buffs.move(other._buffs));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_debuffs.move(other._debuffs));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_upgrades.move(other._upgrades));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_quests.move(other._quests));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = static_cast<int32_t>(this->_achievements.move(other._achievements));
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_reputation.move(other._reputation);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    transfer_error = this->_inventory.move(other._inventory);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    level_count = other._experience_table.get_count();
    transfer_error = this->_experience_table.resize(level_count);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    level_index = 0;
    while (level_index < level_count)
    {
        this->_experience_table.set_value(level_index, other._experience_table.get_value(level_index));
        transfer_error = this->_experience_table.get_error();
        if (transfer_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            this->set_error(transfer_error);
            return (transfer_error);
        }
        level_index += 1;
    }
    transfer_error = this->_equipment.move(other._equipment);
    if (transfer_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->set_error(transfer_error);
        return (transfer_error);
    }
    source_error = other.get_error();
    source_destroy_error = other.destroy();
    if (source_destroy_error != FT_ERR_SUCCESS)
    {
        this->set_error(source_destroy_error);
        return (source_destroy_error);
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t game_character::enable_thread_safety() noexcept
{
    int32_t initialize_error;
    pt_recursive_mutex *new_mutex;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = new_mutex->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (initialize_error);
    }
    this->_mutex = new_mutex;
    return (FT_ERR_SUCCESS);
}

int32_t game_character::disable_thread_safety() noexcept
{
    int32_t destroy_error;
    pt_recursive_mutex *old_mutex;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    return (destroy_error);
}

ft_bool game_character::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}
