#include "game_character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int coins) noexcept
{
    this->_coins += coins;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    this->_coins -= coins;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    this->_valor += valor;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    this->_valor -= valor;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    this->_experience += experience;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    this->_experience -= experience;
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

