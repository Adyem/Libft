#include "game_character.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"

bool ft_character::handle_component_error(int error) noexcept
{
    if (error == FT_ERR_SUCCESS)
        return (false);
    this->set_error(error);
    return (true);
}

bool ft_character::check_internal_errors() noexcept
{
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
      _error(FT_ERR_SUCCESS), _mutex()
{
    if (this->check_internal_errors() == true)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_character::~ft_character() noexcept
{
    return ;
}
