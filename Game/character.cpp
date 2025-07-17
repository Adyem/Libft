#include "character.hpp"

ft_character::ft_character() noexcept
    : _hit_points(0), _armor(0), _might(0), _agility(0),
      _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _x(0), _y(0), _z(0),
      _fire_res{0, 0}, _frost_res{0, 0}, _lightning_res{0, 0},
      _air_res{0, 0}, _earth_res{0, 0}, _chaos_res{0, 0},
      _physical_res{0, 0}, _buffs(), _debuffs(), _quests(), _reputation(),
      _error(ER_SUCCESS)
{
    if (this->_buffs.get_error() != ER_SUCCESS)
        this->set_error(this->_buffs.get_error());
    else if (this->_debuffs.get_error() != ER_SUCCESS)
        this->set_error(this->_debuffs.get_error());
    else if (this->_quests.get_error() != ER_SUCCESS)
        this->set_error(this->_quests.get_error());
    else if (this->_reputation.get_error() != ER_SUCCESS)
        this->set_error(this->_reputation.get_error());
    return ;
}

int ft_character::get_hit_points() const noexcept
{
    return (this->_hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    this->_hit_points = hp;
    return ;
}

int ft_character::get_armor() const noexcept
{
    return (this->_armor);
}

void ft_character::set_armor(int armor) noexcept
{
    this->_armor = armor;
    return ;
}

int ft_character::get_might() const noexcept
{
    return (this->_might);
}

void ft_character::set_might(int might) noexcept
{
    this->_might = might;
    return ;
}

int ft_character::get_agility() const noexcept
{
    return (this->_agility);
}

void ft_character::set_agility(int agility) noexcept
{
    this->_agility = agility;
    return ;
}

int ft_character::get_endurance() const noexcept
{
    return (this->_endurance);
}

void ft_character::set_endurance(int endurance) noexcept
{
    this->_endurance = endurance;
    return ;
}

int ft_character::get_reason() const noexcept
{
    return (this->_reason);
}

void ft_character::set_reason(int reason) noexcept
{
    this->_reason = reason;
    return ;
}

int ft_character::get_insigh() const noexcept
{
    return (this->_insigh);
}

void ft_character::set_insigh(int insigh) noexcept
{
    this->_insigh = insigh;
    return ;
}

int ft_character::get_presence() const noexcept
{
    return (this->_presence);
}

void ft_character::set_presence(int presence) noexcept
{
    this->_presence = presence;
    return ;
}

int ft_character::get_coins() const noexcept
{
    return (this->_coins);
}

void ft_character::set_coins(int coins) noexcept
{
    this->_coins = coins;
    return ;
}

int ft_character::get_x() const noexcept
{
    return (this->_x);
}

void ft_character::set_x(int x) noexcept
{
    this->_x = x;
    return ;
}

int ft_character::get_y() const noexcept
{
    return (this->_y);
}

void ft_character::set_y(int y) noexcept
{
    this->_y = y;
    return ;
}

int ft_character::get_z() const noexcept
{
    return (this->_z);
}

void ft_character::set_z(int z) noexcept
{
    this->_z = z;
    return ;
}

ft_resistance ft_character::get_fire_res() const noexcept
{
    return (this->_fire_res);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    this->_fire_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_frost_res() const noexcept
{
    return (this->_frost_res);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    this->_frost_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_lightning_res() const noexcept
{
    return (this->_lightning_res);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    this->_lightning_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_air_res() const noexcept
{
    return (this->_air_res);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    this->_air_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_earth_res() const noexcept
{
    return (this->_earth_res);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    this->_earth_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_chaos_res() const noexcept
{
    return (this->_chaos_res);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    this->_chaos_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_physical_res() const noexcept
{
    return (this->_physical_res);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    this->_physical_res = {percent, flat};
    return ;
}

ft_map<int, ft_buff> &ft_character::get_buffs() noexcept
{
    return (this->_buffs);
}

const ft_map<int, ft_buff> &ft_character::get_buffs() const noexcept
{
    return (this->_buffs);
}

ft_map<int, ft_debuff> &ft_character::get_debuffs() noexcept
{
    return (this->_debuffs);
}

const ft_map<int, ft_debuff> &ft_character::get_debuffs() const noexcept
{
    return (this->_debuffs);
}

ft_map<int, ft_quest> &ft_character::get_quests() noexcept
{
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_character::get_quests() const noexcept
{
    return (this->_quests);
}

ft_reputation &ft_character::get_reputation() noexcept
{
    return (this->_reputation);
}

const ft_reputation &ft_character::get_reputation() const noexcept
{
    return (this->_reputation);
}

int ft_character::get_error() const noexcept
{
    return (this->_error);
}

void ft_character::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
