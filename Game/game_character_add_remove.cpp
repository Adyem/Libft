#include "game_character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int coins) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_coins);
    this->_coins += coins;
    this->emit_game_metric("character.coins_change", "coins",
        static_cast<long long>(this->_coins) - previous_total,
        this->_coins, "coins");
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_coins);
    this->_coins -= coins;
    this->emit_game_metric("character.coins_change", "coins",
        static_cast<long long>(this->_coins) - previous_total,
        this->_coins, "coins");
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_valor);
    this->_valor += valor;
    this->emit_game_metric("character.valor_change", "valor",
        static_cast<long long>(this->_valor) - previous_total,
        this->_valor, "valor");
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_valor);
    this->_valor -= valor;
    this->emit_game_metric("character.valor_change", "valor",
        static_cast<long long>(this->_valor) - previous_total,
        this->_valor, "valor");
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_experience);
    this->_experience += experience;
    this->emit_game_metric("character.experience_change", "experience",
        static_cast<long long>(this->_experience) - previous_total,
        this->_experience, "xp");
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    long long previous_total;

    previous_total = static_cast<long long>(this->_experience);
    this->_experience -= experience;
    this->emit_game_metric("character.experience_change", "experience",
        static_cast<long long>(this->_experience) - previous_total,
        this->_experience, "xp");
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_character::add_skill(const ft_skill &skill) noexcept
{
    this->_skills.insert(skill.get_id(), skill);
    if (this->handle_component_error(this->_skills.get_error()) == true)
        return (this->_error);
    this->set_error(ER_SUCCESS);
    return (ER_SUCCESS);
}

void ft_character::remove_skill(int id) noexcept
{
    this->_skills.remove(id);
    if (this->handle_component_error(this->_skills.get_error()) == true)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

