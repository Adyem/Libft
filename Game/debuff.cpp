#include "debuff.hpp"

ft_debuff::ft_debuff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0)
{
    return ;
}

int ft_debuff::get_id() const noexcept
{
    return (this->_id);
}

void ft_debuff::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

int ft_debuff::get_duration() const noexcept
{
    return (this->_duration);
}

void ft_debuff::set_duration(int duration) noexcept
{
    this->_duration = duration;
    return ;
}

void ft_debuff::add_duration(int duration) noexcept
{
    this->_duration += duration;
    return ;
}

void ft_debuff::sub_duration(int duration) noexcept
{
    this->_duration -= duration;
    return ;
}

int ft_debuff::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_debuff::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    return ;
}

void ft_debuff::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    return ;
}

void ft_debuff::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    return ;
}

int ft_debuff::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_debuff::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    return ;
}

void ft_debuff::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    return ;
}

void ft_debuff::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    return ;
}

int ft_debuff::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_debuff::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    return ;
}

void ft_debuff::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    return ;
}

void ft_debuff::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    return ;
}

int ft_debuff::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_debuff::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    return ;
}

void ft_debuff::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    return ;
}

void ft_debuff::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    return ;
}
