#include "game_buff.hpp"
#include "../Errno/errno.hpp"

ft_buff::ft_buff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _error(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_buff::ft_buff(const ft_buff &other) noexcept
    : _id(other._id), _duration(other._duration), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _error(other._error)
{
    this->set_error(this->_error);
    return ;
}

ft_buff &ft_buff::operator=(const ft_buff &other) noexcept
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

ft_buff::ft_buff(ft_buff &&other) noexcept
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

ft_buff &ft_buff::operator=(ft_buff &&other) noexcept
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

int ft_buff::get_id() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_id);
}

void ft_buff::set_id(int id) noexcept
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

int ft_buff::get_duration() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_duration);
}

void ft_buff::set_duration(int duration) noexcept
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

void ft_buff::add_duration(int duration) noexcept
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

void ft_buff::sub_duration(int duration) noexcept
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

int ft_buff::get_modifier1() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier1);
}

void ft_buff::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_buff::get_modifier2() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier2);
}

void ft_buff::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_buff::get_modifier3() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier3);
}

void ft_buff::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_buff::get_modifier4() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_modifier4);
}

void ft_buff::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_buff::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    this->set_error(ER_SUCCESS);
    return ;
}

int ft_buff::get_error() const noexcept
{
    return (this->_error);
}

const char *ft_buff::get_error_str() const noexcept
{
    return (ft_strerror(this->_error));
}

void ft_buff::set_error(int err) const noexcept
{
    ft_errno = err;
    this->_error = err;
    return ;
}
