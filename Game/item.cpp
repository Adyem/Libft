#include "item.hpp"

ft_item::ft_item() noexcept
    : _hit_points(0), _armor(0), _might(0), _agility(0),
      _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0),
      _fire_res{0, 0}, _frost_res{0, 0}, _lightning_res{0, 0},
      _air_res{0, 0}, _earth_res{0, 0}, _chaos_res{0, 0},
      _physical_res{0, 0},
      _ability_1{0, 0, 0, 0, 0, 0}, _ability_2{0, 0, 0, 0, 0, 0}
{
    return ;
}

int ft_item::get_hit_points() const noexcept
{
    return (_hit_points);
}

void ft_item::set_hit_points(int hp) noexcept
{
    _hit_points = hp;
    return ;
}

int ft_item::get_armor() const noexcept
{
    return (_armor);
}

void ft_item::set_armor(int armor) noexcept
{
    _armor = armor;
    return ;
}

int ft_item::get_might() const noexcept
{
    return (_might);
}

void ft_item::set_might(int might) noexcept
{
    _might = might;
    return ;
}

int ft_item::get_agility() const noexcept
{
    return (_agility);
}

void ft_item::set_agility(int agility) noexcept
{
    _agility = agility;
    return ;
}

int ft_item::get_endurance() const noexcept
{
    return (_endurance);
}

void ft_item::set_endurance(int endurance) noexcept
{
    _endurance = endurance;
    return ;
}

int ft_item::get_reason() const noexcept
{
    return (_reason);
}

void ft_item::set_reason(int reason) noexcept
{
    _reason = reason;
    return ;
}

int ft_item::get_insigh() const noexcept
{
    return (_insigh);
}

void ft_item::set_insigh(int insigh) noexcept
{
    _insigh = insigh;
    return ;
}

int ft_item::get_presence() const noexcept
{
    return (_presence);
}

void ft_item::set_presence(int presence) noexcept
{
    _presence = presence;
    return ;
}

int ft_item::get_coins() const noexcept
{
    return (_coins);
}

void ft_item::set_coins(int coins) noexcept
{
    _coins = coins;
    return ;
}

ft_resistance ft_item::get_fire_res() const noexcept
{
    return (_fire_res);
}

void ft_item::set_fire_res(int percent, int flat) noexcept
{
    _fire_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_frost_res() const noexcept
{
    return (_frost_res);
}

void ft_item::set_frost_res(int percent, int flat) noexcept
{
    _frost_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_lightning_res() const noexcept
{
    return (_lightning_res);
}

void ft_item::set_lightning_res(int percent, int flat) noexcept
{
    _lightning_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_air_res() const noexcept
{
    return (_air_res);
}

void ft_item::set_air_res(int percent, int flat) noexcept
{
    _air_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_earth_res() const noexcept
{
    return (_earth_res);
}

void ft_item::set_earth_res(int percent, int flat) noexcept
{
    _earth_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_chaos_res() const noexcept
{
    return (_chaos_res);
}

void ft_item::set_chaos_res(int percent, int flat) noexcept
{
    _chaos_res = {percent, flat};
    return ;
}

ft_resistance ft_item::get_physical_res() const noexcept
{
    return (_physical_res);
}

void ft_item::set_physical_res(int percent, int flat) noexcept
{
    _physical_res = {percent, flat};
    return ;
}

ft_item_ability ft_item::get_ability_1() const noexcept
{
    return (_ability_1);
}

void ft_item::set_ability_1(const ft_item_ability &ability) noexcept
{
    _ability_1 = ability;
    return ;
}

ft_item_ability ft_item::get_ability_2() const noexcept
{
    return (_ability_2);
}

void ft_item::set_ability_2(const ft_item_ability &ability) noexcept
{
    _ability_2 = ability;
    return ;
}

uint8_t ft_item::get_ability_1_stat_id() const noexcept
{
    return (_ability_1.stat_id);
}

void ft_item::set_ability_1_stat_id(uint8_t id) noexcept
{
    _ability_1.stat_id = id;
    return ;
}

int ft_item::get_ability_1_main_modifier() const noexcept
{
    return (_ability_1.main_modifier);
}

void ft_item::set_ability_1_main_modifier(int mod) noexcept
{
    _ability_1.main_modifier = mod;
    return ;
}

int ft_item::get_ability_1_bonus_modifier() const noexcept
{
    return (_ability_1.bonus_modifier);
}

void ft_item::set_ability_1_bonus_modifier(int mod) noexcept
{
    _ability_1.bonus_modifier = mod;
    return ;
}

int ft_item::get_ability_1_duration() const noexcept
{
    return (_ability_1.duration);
}

void ft_item::set_ability_1_duration(int duration) noexcept
{
    _ability_1.duration = duration;
    return ;
}

int ft_item::get_ability_1_duration_modifier() const noexcept
{
    return (_ability_1.duration_modifier);
}

void ft_item::set_ability_1_duration_modifier(int mod) noexcept
{
    _ability_1.duration_modifier = mod;
    return ;
}

int ft_item::get_ability_1_stat_modifier() const noexcept
{
    return (_ability_1.stat_modifier);
}

void ft_item::set_ability_1_stat_modifier(int mod) noexcept
{
    _ability_1.stat_modifier = mod;
    return ;
}

uint8_t ft_item::get_ability_2_stat_id() const noexcept
{
    return (_ability_2.stat_id);
}

void ft_item::set_ability_2_stat_id(uint8_t id) noexcept
{
    _ability_2.stat_id = id;
    return ;
}

int ft_item::get_ability_2_main_modifier() const noexcept
{
    return (_ability_2.main_modifier);
}

void ft_item::set_ability_2_main_modifier(int mod) noexcept
{
    _ability_2.main_modifier = mod;
    return ;
}

int ft_item::get_ability_2_bonus_modifier() const noexcept
{
    return (_ability_2.bonus_modifier);
}

void ft_item::set_ability_2_bonus_modifier(int mod) noexcept
{
    _ability_2.bonus_modifier = mod;
    return ;
}

int ft_item::get_ability_2_duration() const noexcept
{
    return (_ability_2.duration);
}

void ft_item::set_ability_2_duration(int duration) noexcept
{
    _ability_2.duration = duration;
    return ;
}

int ft_item::get_ability_2_duration_modifier() const noexcept
{
    return (_ability_2.duration_modifier);
}

void ft_item::set_ability_2_duration_modifier(int mod) noexcept
{
    _ability_2.duration_modifier = mod;
    return ;
}

int ft_item::get_ability_2_stat_modifier() const noexcept
{
    return (_ability_2.stat_modifier);
}

void ft_item::set_ability_2_stat_modifier(int mod) noexcept
{
    _ability_2.stat_modifier = mod;
    return ;
}
