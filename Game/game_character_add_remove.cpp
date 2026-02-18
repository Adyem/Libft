#include "game_character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int coins) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins += coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins -= coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor += valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor -= valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience += experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience -= experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_character::add_skill(const ft_skill &skill) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (this->_error);
    }
    this->_skills.insert(skill.get_id(), skill);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_character::remove_skill(int id) noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_skills.remove(id);
    this->set_error(FT_ERR_SUCCESS);
    return ;
}
