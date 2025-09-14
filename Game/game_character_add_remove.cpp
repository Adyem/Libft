#include "character.hpp"
#include "../Errno/errno.hpp"

void ft_character::add_coins(int coins) noexcept
{
    this->_coins += coins;
    return ;
}

void ft_character::sub_coins(int coins) noexcept
{
    this->_coins -= coins;
    return ;
}

void ft_character::add_valor(int valor) noexcept
{
    this->_valor += valor;
    return ;
}

void ft_character::sub_valor(int valor) noexcept
{
    this->_valor -= valor;
    return ;
}

void ft_character::add_experience(int experience) noexcept
{
    this->_experience += experience;
    return ;
}

void ft_character::sub_experience(int experience) noexcept
{
    this->_experience -= experience;
    return ;
}

int ft_character::add_skill(const ft_skill &skill) noexcept
{
    this->_skills.insert(skill.get_id(), skill);
    if (this->_skills.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_skills.get_error());
        return (this->_error);
    }
    return (ER_SUCCESS);
}

void ft_character::remove_skill(int id) noexcept
{
    this->_skills.remove(id);
    return ;
}

