#include "game_character.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

int ft_character::get_hit_points() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int hit_points;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    hit_points = this->_hit_points;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_hit_points = hp;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

bool ft_character::is_alive() const noexcept
{
    bool lock_acquired;
    int lock_error;
    bool alive;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (false);
    }
    alive = this->_hit_points > 0;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (alive);
}

int ft_character::get_physical_armor() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int armor_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    armor_value = this->_physical_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (armor_value);
}

void ft_character::set_physical_armor(int armor) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->set_physical_armor_internal(armor);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_magic_armor() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int armor_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    armor_value = this->_magic_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (armor_value);
}

void ft_character::set_magic_armor(int armor) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->set_magic_armor_internal(armor);
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_current_physical_armor() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int armor_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    armor_value = this->_current_physical_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (armor_value);
}

void ft_character::set_current_physical_armor(int armor) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_physical_armor = armor;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_current_magic_armor() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int armor_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    armor_value = this->_current_magic_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (armor_value);
}

void ft_character::set_current_magic_armor(int armor) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_magic_armor = armor;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

void ft_character::set_damage_rule(uint8_t rule) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_damage_rule = rule;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

uint8_t ft_character::get_damage_rule() const noexcept
{
    bool lock_acquired;
    int lock_error;
    uint8_t rule_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (FT_DAMAGE_RULE_FLAT);
    }
    rule_value = this->_damage_rule;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (rule_value);
}

int ft_character::get_might() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_might;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_might(int might) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_might = might;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_agility() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_agility;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_agility(int agility) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_agility = agility;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_endurance() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_endurance;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_endurance(int endurance) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_endurance = endurance;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_reason() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_reason;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_reason(int reason) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_reason = reason;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_insigh() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_insigh;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_insigh(int insigh) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_insigh = insigh;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_presence() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int attribute;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    attribute = this->_presence;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (attribute);
}

void ft_character::set_presence(int presence) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_presence = presence;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_coins() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int coins_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    coins_value = this->_coins;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (coins_value);
}

void ft_character::set_coins(int coins) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_coins = coins;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_valor() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int valor_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    valor_value = this->_valor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (valor_value);
}

void ft_character::set_valor(int valor) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_valor = valor;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_experience() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int experience_value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    experience_value = this->_experience;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (experience_value);
}

void ft_character::set_experience(int experience) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_experience = experience;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_x() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int coordinate;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    coordinate = this->_x;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (coordinate);
}

void ft_character::set_x(int x) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_x = x;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_y() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int coordinate;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    coordinate = this->_y;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (coordinate);
}

void ft_character::set_y(int y) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_y = y;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

int ft_character::get_z() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int coordinate;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    coordinate = this->_z;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (coordinate);
}

void ft_character::set_z(int z) noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_z = z;
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_fire_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_fire_res);
    }
    if (this->_fire_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_fire_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_fire_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_fire_res);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_fire_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_frost_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_frost_res);
    }
    if (this->_frost_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_frost_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_frost_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_frost_res);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_frost_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_lightning_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_lightning_res);
    }
    if (this->_lightning_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_lightning_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_lightning_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_lightning_res);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_lightning_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_air_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_air_res);
    }
    if (this->_air_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_air_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_air_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_air_res);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_air_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_earth_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_earth_res);
    }
    if (this->_earth_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_earth_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_earth_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_earth_res);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_earth_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_chaos_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_chaos_res);
    }
    if (this->_chaos_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_chaos_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_chaos_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_chaos_res);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_chaos_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

const ft_resistance &ft_character::get_physical_res() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_physical_res);
    }
    if (this->_physical_res.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(this->_physical_res.get_error());
        this->unlock_internal(lock_acquired);
        return (this->_physical_res);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_physical_res);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    error_code = this->_physical_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

ft_map<int, ft_skill> &ft_character::get_skills() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_skills);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_skills);
}

const ft_map<int, ft_skill> &ft_character::get_skills() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_skills);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_skills);
}

ft_skill *ft_character::get_skill(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    Pair<int, ft_skill> *found;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    if (found == this->_skills.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (&found->value);
}

const ft_skill *ft_character::get_skill(int id) const noexcept
{
    bool lock_acquired;
    int lock_error;
    const Pair<int, ft_skill> *found;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    if (found == this->_skills.end())
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (&found->value);
}

ft_map<int, ft_buff> &ft_character::get_buffs() noexcept
{
    bool lock_acquired;
    int lock_error;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_buffs);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_buffs);
}

const ft_map<int, ft_buff> &ft_character::get_buffs() const noexcept
{
    bool lock_acquired;
    int lock_error;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_buffs);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_buffs);
}

ft_map<int, ft_debuff> &ft_character::get_debuffs() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_debuffs);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_debuffs);
}

const ft_map<int, ft_debuff> &ft_character::get_debuffs() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_debuffs);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_debuffs);
}

ft_map<int, ft_upgrade> &ft_character::get_upgrades() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_upgrades);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_upgrades);
}

const ft_map<int, ft_upgrade> &ft_character::get_upgrades() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_upgrades);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_upgrades);
}

ft_map<int, ft_quest> &ft_character::get_quests() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_quests);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_character::get_quests() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_quests);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_quests);
}

ft_map<int, ft_achievement> &ft_character::get_achievements() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_achievements);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_character::get_achievements() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_achievements);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_achievements);
}

ft_reputation &ft_character::get_reputation() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_reputation);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_reputation);
}

const ft_reputation &ft_character::get_reputation() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_reputation);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_reputation);
}

ft_experience_table &ft_character::get_experience_table() noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_experience_table);
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_experience_table);
}

const ft_experience_table &ft_character::get_experience_table() const noexcept
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (this->_experience_table);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_experience_table);
}

int ft_character::get_level() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int level_value;
    int experience_error;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (0);
    }
    level_value = this->_experience_table.get_level(this->_experience);
    experience_error = this->_experience_table.get_error();
    if (experience_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(experience_error);
        this->unlock_internal(lock_acquired);
        return (0);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (level_value);
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) noexcept
{
    bool lock_acquired;
    int lock_error;
    int equipment_error;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (ft_sharedptr<ft_item>());
    }
    this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != FT_ERR_SUCCESS)
    {
        this->handle_component_error(equipment_error);
        this->unlock_internal(lock_acquired);
        return (ft_sharedptr<ft_item>());
    }
    this->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_equipment.get_item(slot));
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) const noexcept
{
    bool lock_acquired;
    int lock_error;
    int equipment_error;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (ft_sharedptr<ft_item>());
    }
    this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->handle_component_error(equipment_error);
        this->unlock_internal(lock_acquired);
        return (ft_sharedptr<ft_item>());
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (this->_equipment.get_item(slot));
}

int ft_character::get_error() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (lock_error);
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    this->unlock_internal(lock_acquired);
    return (error_code);
}

const char *ft_character::get_error_str() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int error_code;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(lock_error);
        return (ft_strerror(lock_error));
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    this->unlock_internal(lock_acquired);
    return (ft_strerror(error_code));
}

void ft_character::set_physical_armor_internal(int armor) noexcept
{
    int index;

    this->_physical_armor = armor;
    this->_current_physical_armor = armor;
    this->_physical_damage_multiplier = 1.0;
    index = 0;
    while (index < armor)
    {
        this->_physical_damage_multiplier = this->_physical_damage_multiplier * FT_ARMOR_POINT_REDUCTION;
        index++;
    }
    return ;
}

void ft_character::set_magic_armor_internal(int armor) noexcept
{
    int index;

    this->_magic_armor = armor;
    this->_current_magic_armor = armor;
    this->_magic_damage_multiplier = 1.0;
    index = 0;
    while (index < armor)
    {
        this->_magic_damage_multiplier = this->_magic_damage_multiplier * FT_ARMOR_POINT_REDUCTION;
        index++;
    }
    return ;
}

void ft_character::restore_physical_armor_internal() noexcept
{
    this->_current_physical_armor = this->_physical_armor;
    return ;
}

void ft_character::restore_magic_armor_internal() noexcept
{
    this->_current_magic_armor = this->_magic_armor;
    return ;
}

void ft_character::set_error(int err) const noexcept
{
    this->_error = err;
    return ;
}

int ft_character::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

void ft_character::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return ;
    (void)this->_mutex.unlock();
    return ;
}
