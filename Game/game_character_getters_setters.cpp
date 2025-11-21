#include "game_character.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static void game_character_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

int ft_character::get_hit_points() const noexcept
{
    int entry_errno;
    int hit_points;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    hit_points = this->_hit_points;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (hit_points);
}

void ft_character::set_hit_points(int hp) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_hit_points = hp;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

bool ft_character::is_alive() const noexcept
{
    int entry_errno;
    bool alive;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (false);
    }
    alive = this->_hit_points > 0;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (alive);
}

int ft_character::get_physical_armor() const noexcept
{
    int entry_errno;
    int armor_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    armor_value = this->_physical_armor;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (armor_value);
}

void ft_character::set_physical_armor(int armor) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_physical_armor_internal(armor);
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_magic_armor() const noexcept
{
    int entry_errno;
    int armor_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    armor_value = this->_magic_armor;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (armor_value);
}

void ft_character::set_magic_armor(int armor) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_magic_armor_internal(armor);
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_current_physical_armor() const noexcept
{
    int entry_errno;
    int armor_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    armor_value = this->_current_physical_armor;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (armor_value);
}

void ft_character::set_current_physical_armor(int armor) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_physical_armor = armor;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_current_magic_armor() const noexcept
{
    int entry_errno;
    int armor_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    armor_value = this->_current_magic_armor;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (armor_value);
}

void ft_character::set_current_magic_armor(int armor) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_current_magic_armor = armor;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::set_damage_rule(uint8_t rule) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_damage_rule = rule;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

uint8_t ft_character::get_damage_rule() const noexcept
{
    int entry_errno;
    uint8_t rule_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (FT_DAMAGE_RULE_FLAT);
    }
    rule_value = this->_damage_rule;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (rule_value);
}

int ft_character::get_might() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_might;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_might(int might) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_might = might;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_agility() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_agility;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_agility(int agility) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_agility = agility;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_endurance() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_endurance;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_endurance(int endurance) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_endurance = endurance;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_reason() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_reason;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_reason(int reason) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_reason = reason;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_insigh() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_insigh;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_insigh(int insigh) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_insigh = insigh;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_presence() const noexcept
{
    int entry_errno;
    int attribute;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    attribute = this->_presence;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (attribute);
}

void ft_character::set_presence(int presence) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_presence = presence;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_coins() const noexcept
{
    int entry_errno;
    int coins_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    coins_value = this->_coins;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (coins_value);
}

void ft_character::set_coins(int coins) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_coins = coins;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_valor() const noexcept
{
    int entry_errno;
    int valor_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    valor_value = this->_valor;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (valor_value);
}

void ft_character::set_valor(int valor) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_valor = valor;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_experience() const noexcept
{
    int entry_errno;
    int experience_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    experience_value = this->_experience;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (experience_value);
}

void ft_character::set_experience(int experience) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_experience = experience;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_x() const noexcept
{
    int entry_errno;
    int coordinate;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    coordinate = this->_x;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (coordinate);
}

void ft_character::set_x(int x) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_x = x;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_y() const noexcept
{
    int entry_errno;
    int coordinate;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    coordinate = this->_y;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (coordinate);
}

void ft_character::set_y(int y) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_y = y;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::get_z() const noexcept
{
    int entry_errno;
    int coordinate;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    coordinate = this->_z;
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (coordinate);
}

void ft_character::set_z(int z) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_z = z;
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_fire_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_fire_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_fire_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_fire_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_frost_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_frost_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_frost_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_frost_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_lightning_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_lightning_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_lightning_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_lightning_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_air_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_air_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_air_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_air_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_earth_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_earth_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_earth_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_earth_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_chaos_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_chaos_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_chaos_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_chaos_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_resistance ft_character::get_physical_res() const noexcept
{
    int entry_errno;
    ft_resistance resistance_copy;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_resistance());
    }
    resistance_copy = this->_physical_res;
    if (resistance_copy.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(resistance_copy.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (resistance_copy);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (resistance_copy);
}

void ft_character::set_physical_res(int percent, int flat) noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    error_code = this->_physical_res.set_values(percent, flat);
    if (this->handle_component_error(error_code) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, ft_skill> &ft_character::get_skills() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_skills);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_skills);
}

const ft_map<int, ft_skill> &ft_character::get_skills() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_skills);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_skills);
}

ft_skill *ft_character::get_skill(int id) noexcept
{
    int entry_errno;
    Pair<int, ft_skill> *found;
    int component_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    component_error = this->_skills.get_error();
    if (component_error != ER_SUCCESS)
    {
        this->set_error(component_error);
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (found == this->_skills.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (&found->value);
}

const ft_skill *ft_character::get_skill(int id) const noexcept
{
    int entry_errno;
    const Pair<int, ft_skill> *found;
    int component_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    found = this->_skills.find(id);
    component_error = this->_skills.get_error();
    if (component_error != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(component_error);
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (found == this->_skills.end())
    {
        const_cast<ft_character *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_character_restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (&found->value);
}

ft_map<int, ft_buff> &ft_character::get_buffs() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_buffs);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_buffs);
}

const ft_map<int, ft_buff> &ft_character::get_buffs() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_buffs);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_buffs);
}

ft_map<int, ft_debuff> &ft_character::get_debuffs() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_debuffs);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_debuffs);
}

const ft_map<int, ft_debuff> &ft_character::get_debuffs() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_debuffs);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_debuffs);
}

ft_map<int, ft_upgrade> &ft_character::get_upgrades() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_upgrades);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_upgrades);
}

const ft_map<int, ft_upgrade> &ft_character::get_upgrades() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_upgrades);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_upgrades);
}

ft_map<int, ft_quest> &ft_character::get_quests() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_quests);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_quests);
}

const ft_map<int, ft_quest> &ft_character::get_quests() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_quests);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_quests);
}

ft_map<int, ft_achievement> &ft_character::get_achievements() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_achievements);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_achievements);
}

const ft_map<int, ft_achievement> &ft_character::get_achievements() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_achievements);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_achievements);
}

ft_reputation &ft_character::get_reputation() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_reputation);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_reputation);
}

const ft_reputation &ft_character::get_reputation() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_reputation);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_reputation);
}

ft_experience_table &ft_character::get_experience_table() noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_experience_table);
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_experience_table);
}

const ft_experience_table &ft_character::get_experience_table() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_experience_table);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (this->_experience_table);
}

int ft_character::get_level() const noexcept
{
    int entry_errno;
    int level_value;
    int experience_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    level_value = this->_experience_table.get_level(this->_experience);
    experience_error = this->_experience_table.get_error();
    if (experience_error != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(experience_error);
        game_character_restore_errno(guard, entry_errno);
        return (0);
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (level_value);
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) noexcept
{
    int entry_errno;
    ft_sharedptr<ft_item> item;
    int equipment_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    item = this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != ER_SUCCESS)
    {
        this->handle_component_error(equipment_error);
        game_character_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    this->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (item);
}

ft_sharedptr<ft_item> ft_character::get_equipped_item(int slot) const noexcept
{
    int entry_errno;
    ft_sharedptr<ft_item> item;
    int equipment_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    item = this->_equipment.get_item(slot);
    equipment_error = this->_equipment.get_error();
    if (equipment_error != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->handle_component_error(equipment_error);
        game_character_restore_errno(guard, entry_errno);
        return (ft_sharedptr<ft_item>());
    }
    const_cast<ft_character *>(this)->set_error(ER_SUCCESS);
    game_character_restore_errno(guard, entry_errno);
    return (item);
}

int ft_character::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    game_character_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_character::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_character *>(this)->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error;
    const_cast<ft_character *>(this)->set_error(error_code);
    game_character_restore_errno(guard, entry_errno);
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
    ft_errno = err;
    this->_error = err;
    return ;
}

