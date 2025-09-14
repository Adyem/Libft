#include "skill.hpp"
#include "../Libft/libft.hpp"

ft_skill::ft_skill() noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _error(ER_SUCCESS)
{
    return ;
}

ft_skill::~ft_skill() noexcept
{
    return ;
}

void ft_skill::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

int ft_skill::get_id() const noexcept
{
    return (this->_id);
}

void ft_skill::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

int ft_skill::get_level() const noexcept
{
    return (this->_level);
}

void ft_skill::set_level(int level) noexcept
{
    this->_level = level;
    return ;
}

int ft_skill::get_cooldown() const noexcept
{
    return (this->_cooldown);
}

void ft_skill::set_cooldown(int cooldown) noexcept
{
    this->_cooldown = cooldown;
    return ;
}

void ft_skill::add_cooldown(int cooldown) noexcept
{
    this->_cooldown += cooldown;
    return ;
}

void ft_skill::sub_cooldown(int cooldown) noexcept
{
    this->_cooldown -= cooldown;
    return ;
}

int ft_skill::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_skill::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    return ;
}

void ft_skill::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    return ;
}

void ft_skill::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    return ;
}

int ft_skill::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_skill::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    return ;
}

void ft_skill::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    return ;
}

void ft_skill::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    return ;
}

int ft_skill::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_skill::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    return ;
}

void ft_skill::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    return ;
}

void ft_skill::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    return ;
}

int ft_skill::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_skill::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    return ;
}

void ft_skill::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    return ;
}

void ft_skill::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    return ;
}

int ft_skill::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_skill::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}
