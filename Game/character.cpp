#include "character.hpp"

ft_character::ft_character() noexcept
    : _hit_points(0), _armor(0), _might(0), _agility(0),
      _endurance(0), _reason(0), _insigh(0), _presence(0),
      _coins(0), _x(0), _y(0), _z(0),
      _fire_res{0, 0}, _frost_res{0, 0}, _lightning_res{0, 0},
      _air_res{0, 0}, _earth_res{0, 0}, _chaos_res{0, 0},
      _physical_res{0, 0}
{
    return ;
}

int ft_character::get_hit_points() const noexcept
{
    return (_hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    _hit_points = hp;
    return ;
}

int ft_character::get_armor() const noexcept
{
    return (_armor);
}

void ft_character::set_armor(int armor) noexcept
{
    _armor = armor;
    return ;
}

int ft_character::get_might() const noexcept
{
    return (_might);
}

void ft_character::set_might(int might) noexcept
{
    _might = might;
    return ;
}

int ft_character::get_agility() const noexcept
{
    return (_agility);
}

void ft_character::set_agility(int agility) noexcept
{
    _agility = agility;
    return ;
}

int ft_character::get_endurance() const noexcept
{
    return (_endurance);
}

void ft_character::set_endurance(int endurance) noexcept
{
    _endurance = endurance;
    return ;
}

int ft_character::get_reason() const noexcept
{
    return (_reason);
}

void ft_character::set_reason(int reason) noexcept
{
    _reason = reason;
    return ;
}

int ft_character::get_insigh() const noexcept
{
    return (_insigh);
}

void ft_character::set_insigh(int insigh) noexcept
{
    _insigh = insigh;
    return ;
}

int ft_character::get_presence() const noexcept
{
    return (_presence);
}

void ft_character::set_presence(int presence) noexcept
{
    _presence = presence;
    return ;
}

int ft_character::get_coins() const noexcept
{
    return (_coins);
}

void ft_character::set_coins(int coins) noexcept
{
    _coins = coins;
    return ;
}

<<<<<<< HEAD
=======
int ft_character::get_valor() const noexcept
{
    return (_valor);
}

void ft_character::set_valor(int valor) noexcept
{
    _valor = valor;
    return ;
}

int ft_character::get_x() const noexcept
{
    return (_x);
}

void ft_character::set_x(int x) noexcept
{
    _x = x;
    return ;
}

int ft_character::get_y() const noexcept
{
    return (_y);
}

void ft_character::set_y(int y) noexcept
{
    _y = y;
    return ;
}

int ft_character::get_z() const noexcept
{
    return (_z);
}

void ft_character::set_z(int z) noexcept
{
    _z = z;
    return ;
}

>>>>>>> origin/main
ft_resistance ft_character::get_fire_res() const noexcept
{
    return (_fire_res);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    _fire_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_frost_res() const noexcept
{
    return (_frost_res);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    _frost_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_lightning_res() const noexcept
{
    return (_lightning_res);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    _lightning_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_air_res() const noexcept
{
    return (_air_res);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    _air_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_earth_res() const noexcept
{
    return (_earth_res);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    _earth_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_chaos_res() const noexcept
{
    return (_chaos_res);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    _chaos_res = {percent, flat};
    return ;
}

ft_resistance ft_character::get_physical_res() const noexcept
{
    return (_physical_res);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    _physical_res = {percent, flat};
    return ;
}
