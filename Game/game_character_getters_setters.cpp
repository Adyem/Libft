#include "game_character.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

int ft_character::get_hit_points() const noexcept
{
    int hit_points;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    hit_points = this->_hit_points;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_hit_points = hp;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

bool ft_character::is_alive() const noexcept
{
    bool alive;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (false);
    }
    alive = this->_hit_points > 0;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (alive);
}

int ft_character::get_physical_armor() const noexcept
{
    int armor_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    armor_value = this->_physical_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (armor_value);
}

void ft_character::set_physical_armor(int armor) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_physical_armor_internal(armor);
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_magic_armor() const noexcept
{
    int armor_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    armor_value = this->_magic_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (armor_value);
}

void ft_character::set_magic_armor(int armor) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_magic_armor_internal(armor);
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_current_physical_armor() const noexcept
{
    int armor_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    armor_value = this->_current_physical_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (armor_value);
}

void ft_character::set_current_physical_armor(int armor) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_current_physical_armor = armor;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_current_magic_armor() const noexcept
{
    int armor_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    armor_value = this->_current_magic_armor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (armor_value);
}

void ft_character::set_current_magic_armor(int armor) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_current_magic_armor = armor;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

void ft_character::set_damage_rule(uint8_t rule) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_damage_rule = rule;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

uint8_t ft_character::get_damage_rule() const noexcept
{
    uint8_t rule_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (FT_DAMAGE_RULE_FLAT);
    }
    rule_value = this->_damage_rule;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (rule_value);
}

int ft_character::get_might() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_might;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_might(int might) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_might = might;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_agility() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_agility;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_agility(int agility) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_agility = agility;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_endurance() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_endurance;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_endurance(int endurance) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_endurance = endurance;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_reason() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_reason;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_reason(int reason) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_reason = reason;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_insigh() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_insigh;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_insigh(int insigh) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_insigh = insigh;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_presence() const noexcept
{
    int attribute;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    attribute = this->_presence;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (attribute);
}

void ft_character::set_presence(int presence) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_presence = presence;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_coins() const noexcept
{
    int coins_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    coins_value = this->_coins;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (coins_value);
}

void ft_character::set_coins(int coins) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_coins = coins;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_valor() const noexcept
{
    int valor_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    valor_value = this->_valor;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (valor_value);
}

void ft_character::set_valor(int valor) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_valor = valor;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_experience() const noexcept
{
    int experience_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    experience_value = this->_experience;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (experience_value);
}

void ft_character::set_experience(int experience) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_experience = experience;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_x() const noexcept
{
    int coordinate;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    coordinate = this->_x;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (coordinate);
}

void ft_character::set_x(int x) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_x = x;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_y() const noexcept
{
    int coordinate;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    coordinate = this->_y;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (coordinate);
}

void ft_character::set_y(int y) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_y = y;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_character::get_z() const noexcept
{
    int coordinate;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    coordinate = this->_z;
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (coordinate);
}

void ft_character::set_z(int z) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->_z = z;
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_fire_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_fire_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_fire_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_frost_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_frost_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_frost_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_lightning_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_lightning_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_lightning_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_air_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_air_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_air_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_earth_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_earth_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_earth_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_chaos_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_chaos_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_chaos_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_resistance ft_character::get_physical_res() const noexcept
{
    ft_resistance resistance_copy;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_resistance());
    }
    resistance_copy = this->_physical_res;
    if (resistance_copy.get_error() != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        if (guard.owns_lock())
            guard.unlock();
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (resistance_copy);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    error_code = this->_physical_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        if (guard.owns_lock())
            guard.unlock();
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_map<int, ft_skill> &ft_character::get_skills() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_skills);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_skills);
}

const ft_map<int, ft_skill> &ft_character::get_skills() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_skills);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_skills);
}

ft_skill *ft_character::get_skill(int id) noexcept
{
    Pair<int, ft_skill> *found;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    if (found == this->_skills.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (&found->value);
}

const ft_skill *ft_character::get_skill(int id) const noexcept
{
    const Pair<int, ft_skill> *found;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    if (found == this->_skills.end())
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_NOT_FOUND);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_nullptr);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (&found->value);
}

ft_map<int, ft_buff> &ft_character::get_buffs() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_buffs);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_buffs);
}

const ft_map<int, ft_buff> &ft_character::get_buffs() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_buffs);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_buffs);
}

ft_map<int, ft_debuff> &ft_character::get_debuffs() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_debuffs);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_debuffs);
}

const ft_map<int, ft_debuff> &ft_character::get_debuffs() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_debuffs);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_debuffs);
}

ft_map<int, ft_upgrade> &ft_character::get_upgrades() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_upgrades);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_upgrades);
}

const ft_map<int, ft_upgrade> &ft_character::get_upgrades() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_upgrades);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_upgrades);
}

ft_map<int, ft_quest> &ft_character::get_quests() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_quests);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_character::get_quests() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_quests);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_quests);
}

ft_map<int, ft_achievement> &ft_character::get_achievements() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_achievements);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_character::get_achievements() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_achievements);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_achievements);
}

ft_reputation &ft_character::get_reputation() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_reputation);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_reputation);
}

const ft_reputation &ft_character::get_reputation() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_reputation);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_reputation);
}

ft_experience_table &ft_character::get_experience_table() noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_experience_table);
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_experience_table);
}

const ft_experience_table &ft_character::get_experience_table() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (this->_experience_table);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_experience_table);
}

int ft_character::get_level() const noexcept
{
    int level_value;
    int experience_error;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    level_value = this->_experience_table.get_level(this->_experience);
    experience_error = this->_experience_table.get_error();
    if (experience_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(experience_error);
        if (guard.owns_lock())
            guard.unlock();
        return (0);
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (level_value);
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) noexcept
{
    int equipment_error;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_sharedptr<ft_item>());
    }
    this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != FT_ERR_SUCCESS)
    {
        this->handle_component_error(equipment_error);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_sharedptr<ft_item>());
    }
    this->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_equipment.get_item(slot));
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) const noexcept
{
    int equipment_error;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_sharedptr<ft_item>());
    }
    this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != FT_ERR_SUCCESS)
    {
        const_cast<ft_character *>(this)->handle_component_error(equipment_error);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_sharedptr<ft_item>());
    }
    const_cast<ft_character *>(this)->set_error(FT_ERR_SUCCESS);
    if (guard.owns_lock())
        guard.unlock();
    return (this->_equipment.get_item(slot));
}

int ft_character::get_error() const noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (FT_ERR_INVALID_STATE);
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    if (guard.owns_lock())
        guard.unlock();
    return (error_code);
}

const char *ft_character::get_error_str() const noexcept
{
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_INVALID_STATE);
        if (guard.owns_lock())
            guard.unlock();
        return (ft_strerror(FT_ERR_INVALID_STATE));
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    if (guard.owns_lock())
        guard.unlock();
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
