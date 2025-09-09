#include "upgrade.hpp"

ft_upgrade::ft_upgrade() noexcept
    : _id(0), _current_level(0), _max_level(0),
      _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0)
{
    return ;
}

int ft_upgrade::get_id() const noexcept
{
    return (this->_id);
}

void ft_upgrade::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

uint16_t ft_upgrade::get_current_level() const noexcept
{
    return (this->_current_level);
}

void ft_upgrade::set_current_level(uint16_t level) noexcept
{
    this->_current_level = level;
    return ;
}

void ft_upgrade::add_level(uint16_t level) noexcept
{
    this->_current_level += level;
    if (this->_current_level > this->_max_level)
        this->_current_level = this->_max_level;
    return ;
}

void ft_upgrade::sub_level(uint16_t level) noexcept
{
    if (level > this->_current_level)
        this->_current_level = 0;
    else
        this->_current_level -= level;
    return ;
}

uint16_t ft_upgrade::get_max_level() const noexcept
{
    return (this->_max_level);
}

void ft_upgrade::set_max_level(uint16_t level) noexcept
{
    this->_max_level = level;
    return ;
}

int ft_upgrade::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_upgrade::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    return ;
}

void ft_upgrade::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    return ;
}

void ft_upgrade::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    return ;
}

int ft_upgrade::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_upgrade::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    return ;
}

void ft_upgrade::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    return ;
}

void ft_upgrade::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    return ;
}

int ft_upgrade::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_upgrade::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    return ;
}

void ft_upgrade::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    return ;
}

void ft_upgrade::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    return ;
}

int ft_upgrade::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_upgrade::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    return ;
}

void ft_upgrade::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    return ;
}

void ft_upgrade::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    return ;
}

