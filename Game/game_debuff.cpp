#include "game_debuff.hpp"
#include "../Errno/errno.hpp"

ft_debuff::ft_debuff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _error(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_debuff::ft_debuff(const ft_debuff &other) noexcept
    : _id(other._id), _duration(other._duration), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    this->set_error(this->_error);
    return ;
}

ft_debuff &ft_debuff::operator=(const ft_debuff &other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_duration = other._duration;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other_error);
    }
    return (*this);
}

ft_debuff::ft_debuff(ft_debuff &&other) noexcept
    : _id(other._id), _duration(other._duration), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    other._id = 0;
    other._duration = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error);
    return ;
}

ft_debuff &ft_debuff::operator=(ft_debuff &&other) noexcept
{
    if (this != &other)
    {
        int other_error = other._error;
        this->_id = other._id;
        this->_duration = other._duration;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->set_error(other_error);
        other._id = 0;
        other._duration = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
        other.set_error(ER_SUCCESS);
    }
    return (*this);
}

int ft_debuff::get_id() const noexcept
{
    return (this->_id);
}

void ft_debuff::set_id(int id) noexcept
{
    if (id < 0)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_id = id;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_duration() const noexcept
{
    return (this->_duration);
}

void ft_debuff::set_duration(int duration) noexcept
{
    if (duration < 0)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_duration = duration;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::add_duration(int duration) noexcept
{
    if (duration < 0)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_duration += duration;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::sub_duration(int duration) noexcept
{
    if (duration < 0)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_duration -= duration;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_debuff::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_debuff::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_debuff::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_debuff::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_debuff::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_debuff::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_debuff::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_debuff::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
