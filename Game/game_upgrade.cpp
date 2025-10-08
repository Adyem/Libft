#include "game_upgrade.hpp"

ft_upgrade::ft_upgrade() noexcept
    : _id(0), _current_level(0), _max_level(0),
      _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _error(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_upgrade::ft_upgrade(const ft_upgrade &other) noexcept
    : _id(other._id), _current_level(other._current_level), _max_level(other._max_level),
      _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    this->set_error(this->_error);
    return ;
}

ft_upgrade &ft_upgrade::operator=(const ft_upgrade &other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_current_level = other._current_level;
        this->_max_level = other._max_level;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other_error);
    }
    return (*this);
}

ft_upgrade::ft_upgrade(ft_upgrade &&other) noexcept
    : _id(other._id), _current_level(other._current_level), _max_level(other._max_level),
      _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    other._id = 0;
    other._current_level = 0;
    other._max_level = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error);
    return ;
}

ft_upgrade &ft_upgrade::operator=(ft_upgrade &&other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_current_level = other._current_level;
        this->_max_level = other._max_level;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other_error);
        other._id = 0;
        other._current_level = 0;
        other._max_level = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

int ft_upgrade::get_id() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_id);
}

void ft_upgrade::set_id(int id) noexcept
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

uint16_t ft_upgrade::get_current_level() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_current_level);
}

void ft_upgrade::set_current_level(uint16_t level) noexcept
{
    if (this->_max_level != 0 && level > this->_max_level)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_current_level = level;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::add_level(uint16_t level) noexcept
{
    this->_current_level += level;
    if (this->_current_level > this->_max_level)
        this->_current_level = this->_max_level;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::sub_level(uint16_t level) noexcept
{
    if (level > this->_current_level)
        this->_current_level = 0;
    else
        this->_current_level -= level;
    this->set_error(ER_SUCCESS);
    return ;
}

uint16_t ft_upgrade::get_max_level() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_max_level);
}

void ft_upgrade::set_max_level(uint16_t level) noexcept
{
    this->_max_level = level;
    if (this->_max_level != 0 && this->_current_level > this->_max_level)
        this->_current_level = this->_max_level;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier1() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier1);
}

void ft_upgrade::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier2() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier2);
}

void ft_upgrade::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier3() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier3);
}

void ft_upgrade::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier4() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier4);
}

void ft_upgrade::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_upgrade::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_upgrade::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_upgrade::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}

