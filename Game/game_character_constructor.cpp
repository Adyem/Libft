#include "game_character.hpp"
#include "../Errno/errno.hpp"

ft_character::ft_character() noexcept
    : _hit_points(0), _physical_armor(0), _magic_armor(0),
      _current_physical_armor(0), _current_magic_armor(0),
      _physical_damage_multiplier(1.0), _magic_damage_multiplier(1.0),
      _damage_rule(FT_DAMAGE_RULE_FLAT),
      _might(0), _agility(0), _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _valor(0), _experience(0), _x(0), _y(0), _z(0),
      _fire_res{0, 0}, _frost_res{0, 0}, _lightning_res{0, 0},
      _air_res{0, 0}, _earth_res{0, 0}, _chaos_res{0, 0},
      _physical_res{0, 0}, _skills(), _buffs(), _debuffs(), _upgrades(), _quests(), _achievements(), _reputation(), _inventory(), _equipment(),
      _error(ER_SUCCESS)
{
    if (this->_buffs.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_buffs.get_error());
        return ;
    }
    if (this->_skills.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_skills.get_error());
        return ;
    }
    if (this->_debuffs.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_debuffs.get_error());
        return ;
    }
    if (this->_upgrades.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_upgrades.get_error());
        return ;
    }
    if (this->_quests.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_quests.get_error());
        return ;
    }
    if (this->_achievements.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_achievements.get_error());
        return ;
    }
    if (this->_reputation.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_reputation.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
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
    if (this != &other)
    {
        int other_error = other._error;
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
        if (this->_buffs.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_buffs.get_error());
            return (*this);
        }
        if (this->_skills.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_skills.get_error());
            return (*this);
        }
        if (this->_debuffs.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_debuffs.get_error());
            return (*this);
        }
        if (this->_upgrades.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_upgrades.get_error());
            return (*this);
        }
        if (this->_quests.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_quests.get_error());
            return (*this);
        }
        if (this->_achievements.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_achievements.get_error());
            return (*this);
        }
        if (this->_reputation.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reputation.get_error());
            return (*this);
        }
        this->set_error(other_error);
    }
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
    if (this != &other)
    {
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
        if (this->_buffs.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_buffs.get_error());
            return (*this);
        }
        if (this->_skills.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_skills.get_error());
            return (*this);
        }
        if (this->_debuffs.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_debuffs.get_error());
            return (*this);
        }
        if (this->_upgrades.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_upgrades.get_error());
            return (*this);
        }
        if (this->_quests.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_quests.get_error());
            return (*this);
        }
        if (this->_achievements.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_achievements.get_error());
            return (*this);
        }
        if (this->_reputation.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_reputation.get_error());
            return (*this);
        }
        this->set_error(other._error);
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
        other._fire_res.dr_percent = 0;
        other._fire_res.dr_flat = 0;
        other._frost_res.dr_percent = 0;
        other._frost_res.dr_flat = 0;
        other._lightning_res.dr_percent = 0;
        other._lightning_res.dr_flat = 0;
        other._air_res.dr_percent = 0;
        other._air_res.dr_flat = 0;
        other._earth_res.dr_percent = 0;
        other._earth_res.dr_flat = 0;
        other._chaos_res.dr_percent = 0;
        other._chaos_res.dr_flat = 0;
        other._physical_res.dr_percent = 0;
        other._physical_res.dr_flat = 0;
        other._skills.clear();
        other._buffs.clear();
        other._debuffs.clear();
        other._upgrades.clear();
        other._quests.clear();
        other._achievements.clear();
        other._reputation = ft_reputation();
        other._inventory = ft_inventory();
        other._equipment = ft_equipment();
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

