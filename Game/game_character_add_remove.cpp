#include "game_character.hpp"
#include "../Errno/errno.hpp"

static void game_character_restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno)
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

void ft_character::add_coins(int coins) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins += coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins -= coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor += valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor -= valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience += experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    int entry_errno;
    long long previous_total;
    long long delta_value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience -= experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

int ft_character::add_skill(const ft_skill &skill) noexcept
{
    int entry_errno;
    int component_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    this->_skills.insert(skill.get_id(), skill);
    component_error = this->_skills.get_error();
    if (this->handle_component_error(component_error) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return (this->_error);
    }
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return (FT_ERR_SUCCESSS);
}

void ft_character::remove_skill(int id) noexcept
{
    int entry_errno;
    int component_error;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->_skills.remove(id);
    component_error = this->_skills.get_error();
    if (this->handle_component_error(component_error) == true)
    {
        game_character_restore_errno(guard, entry_errno);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    game_character_restore_errno(guard, entry_errno);
    return ;
}

