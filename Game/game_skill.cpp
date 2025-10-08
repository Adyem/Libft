#include "game_skill.hpp"
#include "../Libft/libft.hpp"

ft_skill::ft_skill() noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _error(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_skill::~ft_skill() noexcept
{
    return ;
}

ft_skill::ft_skill(const ft_skill &other) noexcept
    : _id(other._id), _level(other._level), _cooldown(other._cooldown), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    this->set_error(other._error);
    return ;
}

ft_skill &ft_skill::operator=(const ft_skill &other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_level = other._level;
        this->_cooldown = other._cooldown;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other._error);
    }
    return (*this);
}

ft_skill::ft_skill(ft_skill &&other) noexcept
    : _id(other._id), _level(other._level), _cooldown(other._cooldown), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    other._id = 0;
    other._level = 0;
    other._cooldown = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    this->set_error(this->_error);
    other.set_error(ER_SUCCESS);
    return ;
}

ft_skill &ft_skill::operator=(ft_skill &&other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_level = other._level;
        this->_cooldown = other._cooldown;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other._error);
        other._id = 0;
        other._level = 0;
        other._cooldown = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

void ft_skill::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

int ft_skill::get_id() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_id);
}

void ft_skill::set_id(int id) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_level() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_level);
}

void ft_skill::set_level(int level) noexcept
{
    if (level < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_level = level;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_cooldown() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_cooldown);
}

void ft_skill::set_cooldown(int cooldown) noexcept
{
    if (cooldown < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_cooldown = cooldown;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::add_cooldown(int cooldown) noexcept
{
    if (cooldown < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_cooldown += cooldown;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::sub_cooldown(int cooldown) noexcept
{
    if (cooldown < 0 || this->_cooldown < cooldown)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_cooldown -= cooldown;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_modifier1() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier1);
}

void ft_skill::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::add_modifier1(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::sub_modifier1(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_modifier2() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier2);
}

void ft_skill::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::add_modifier2(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::sub_modifier2(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_modifier3() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier3);
}

void ft_skill::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::add_modifier3(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::sub_modifier3(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_skill::get_modifier4() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier4);
}

void ft_skill::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::add_modifier4(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_skill::sub_modifier4(int mod) noexcept
{
    if (mod < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
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
