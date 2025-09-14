#include "character.hpp"
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
        this->set_error(this->_buffs.get_error());
    else if (this->_skills.get_error() != ER_SUCCESS)
        this->set_error(this->_skills.get_error());
    else if (this->_debuffs.get_error() != ER_SUCCESS)
        this->set_error(this->_debuffs.get_error());
    else if (this->_upgrades.get_error() != ER_SUCCESS)
        this->set_error(this->_upgrades.get_error());
    else if (this->_quests.get_error() != ER_SUCCESS)
        this->set_error(this->_quests.get_error());
    else if (this->_achievements.get_error() != ER_SUCCESS)
        this->set_error(this->_achievements.get_error());
    else if (this->_reputation.get_error() != ER_SUCCESS)
        this->set_error(this->_reputation.get_error());
    return ;
}

ft_character::~ft_character() noexcept
{
    return ;
}

