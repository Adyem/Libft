#include "game_character.hpp"
#include "../Errno/errno.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

static void game_character_sleep_backoff()
{
    pt_thread_sleep(1);
    return ;
}

static void game_character_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_character::lock_pair(const ft_character &first, const ft_character &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_character *ordered_first;
    const ft_character *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_character *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESSS)
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
            ft_errno = FT_ERR_SUCCESSS;
            return (FT_ERR_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_character_sleep_backoff();
    }
}

bool ft_character::handle_component_error(int error) noexcept
{
    if (error == FT_ERR_SUCCESSS)
        return (false);
    this->set_error(error);
    return (true);
}

bool ft_character::check_internal_errors() noexcept
{
    if (this->handle_component_error(this->_buffs.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_skills.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_debuffs.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_upgrades.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_quests.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_achievements.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_reputation.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_inventory.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_equipment.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_fire_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_frost_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_lightning_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_air_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_earth_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_chaos_res.get_error()) == true)
        return (true);
    if (this->handle_component_error(this->_physical_res.get_error()) == true)
        return (true);
    return (false);
}

ft_character::ft_character() noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res(), _frost_res(), _lightning_res(),
      _air_res(), _earth_res(), _chaos_res(),
      _physical_res(), _skills(), _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _error(FT_ERR_SUCCESSS), _mutex()
{
    if (this->check_internal_errors() == true)
        return ;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_character::~ft_character() noexcept
{
    return ;
}

ft_character::ft_character(const ft_character &other) noexcept
    : ft_character()
{
    *this = other;
    return ;
}

ft_character &ft_character::operator=(const ft_character &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_character::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_hit_points = other._hit_points;
    this->_physical_armor = other._physical_armor;
    this->_magic_armor = other._magic_armor;
    this->_current_physical_armor = other._current_physical_armor;
    this->_current_magic_armor = other._current_magic_armor;
    this->_physical_damage_multiplier = other._physical_damage_multiplier;
    this->_magic_damage_multiplier = other._magic_damage_multiplier;
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
    this->_experience_table = other._experience_table;
    this->_fire_res = other._fire_res;
    this->_frost_res = other._frost_res;
    this->_lightning_res = other._lightning_res;
    this->_air_res = other._air_res;
    this->_earth_res = other._earth_res;
    this->_chaos_res = other._chaos_res;
    this->_physical_res = other._physical_res;
    this->_skills = other._skills;
    this->_buffs = other._buffs;
    this->_debuffs = other._debuffs;
    this->_upgrades = other._upgrades;
    this->_quests = other._quests;
    this->_achievements = other._achievements;
    this->_reputation = other._reputation;
    this->_inventory = other._inventory;
    this->_equipment = other._equipment;
    this->_error = other._error;
    if (this->check_internal_errors() == true)
    {
        game_character_restore_errno(this_guard, entry_errno);
        game_character_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->set_error(other._error);
    game_character_restore_errno(this_guard, entry_errno);
    game_character_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_character::ft_character(ft_character &&other) noexcept
    : ft_character()
{
    *this = ft_move(other);
    return ;
}

ft_character &ft_character::operator=(ft_character &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_character::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_hit_points = other._hit_points;
    this->_physical_armor = other._physical_armor;
    this->_magic_armor = other._magic_armor;
    this->_current_physical_armor = other._current_physical_armor;
    this->_current_magic_armor = other._current_magic_armor;
    this->_physical_damage_multiplier = other._physical_damage_multiplier;
    this->_magic_damage_multiplier = other._magic_damage_multiplier;
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
    this->_experience_table = ft_move(other._experience_table);
    this->_fire_res = other._fire_res;
    this->_frost_res = other._frost_res;
    this->_lightning_res = other._lightning_res;
    this->_air_res = other._air_res;
    this->_earth_res = other._earth_res;
    this->_chaos_res = other._chaos_res;
    this->_physical_res = other._physical_res;
    this->_skills = ft_move(other._skills);
    this->_buffs = ft_move(other._buffs);
    this->_debuffs = ft_move(other._debuffs);
    this->_upgrades = ft_move(other._upgrades);
    this->_quests = ft_move(other._quests);
    this->_achievements = ft_move(other._achievements);
    this->_reputation = ft_move(other._reputation);
    this->_inventory = ft_move(other._inventory);
    this->_equipment = ft_move(other._equipment);
    this->_error = other._error;
    if (this->check_internal_errors() == true)
    {
        game_character_restore_errno(this_guard, entry_errno);
        game_character_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    other._hit_points = 0;
    other._physical_armor = 0;
    other._magic_armor = 0;
    other._current_physical_armor = 0;
    other._current_magic_armor = 0;
    other._physical_damage_multiplier = 1.0;
    other._magic_damage_multiplier = 1.0;
    other._damage_rule = FT_DAMAGE_RULE_FLAT;
    other._might = 0;
    other._agility = 0;
    other._endurance = 0;
    other._reason = 0;
    other._insigh = 0;
    other._presence = 0;
    other._coins = 0;
    other._valor = 0;
    other._experience = 0;
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other._experience_table = ft_experience_table();
    other._fire_res.reset();
    other._frost_res.reset();
    other._lightning_res.reset();
    other._air_res.reset();
    other._earth_res.reset();
    other._chaos_res.reset();
    other._physical_res.reset();
    other._skills.clear();
    other._buffs.clear();
    other._debuffs.clear();
    other._upgrades.clear();
    other._quests.clear();
    other._achievements.clear();
    other._reputation = ft_reputation();
    other._inventory = ft_inventory();
    other._equipment = ft_equipment();
    this->set_error(this->_error);
    other.set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(this_guard, entry_errno);
    game_character_restore_errno(other_guard, entry_errno);
    return (*this);
}

