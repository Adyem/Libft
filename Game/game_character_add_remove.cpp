#include "game_character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int coins) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins += coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_coins);
    this->_coins -= coins;
    delta_value = static_cast<long long>(this->_coins) - previous_total;
    this->emit_game_metric("character.coins_change", "coins",
        delta_value, this->_coins, "coins");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor += valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_valor);
    this->_valor -= valor;
    delta_value = static_cast<long long>(this->_valor) - previous_total;
    this->emit_game_metric("character.valor_change", "valor",
        delta_value, this->_valor, "valor");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience += experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    long long previous_total;
    long long delta_value;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    previous_total = static_cast<long long>(this->_experience);
    this->_experience -= experience;
    delta_value = static_cast<long long>(this->_experience) - previous_total;
    this->emit_game_metric("character.experience_change", "experience",
        delta_value, this->_experience, "xp");
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

int ft_character::add_skill(const ft_skill &skill) noexcept
{
    int component_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_error);
    }
    this->_skills.insert(skill.get_id(), skill);
    component_error = this->_skills.last_operation_error();
    if (this->handle_component_error(component_error) == true)
    {
        return (this->_error);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_character::remove_skill(int id) noexcept
{
    int component_error;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_skills.remove(id);
    component_error = this->_skills.last_operation_error();
    if (this->handle_component_error(component_error) == true)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}
