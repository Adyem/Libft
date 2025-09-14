#include "game_event.hpp"

ft_event::ft_event() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0), _modifier4(0), _callback()
{
    return ;
}

ft_event::~ft_event() noexcept
{
    return ;
}

ft_event::ft_event(const ft_event &other) noexcept
    : _id(other._id), _duration(other._duration), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _callback(other._callback)
{
    return ;
}

ft_event &ft_event::operator=(const ft_event &other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_duration = other._duration;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->_callback = other._callback;
    }
    return (*this);
}

ft_event::ft_event(ft_event &&other) noexcept
    : _id(other._id), _duration(other._duration), _modifier1(other._modifier1), _modifier2(other._modifier2), _modifier3(other._modifier3), _modifier4(other._modifier4), _callback(ft_move(other._callback))
{
    other._id = 0;
    other._duration = 0;
    other._modifier1 = 0;
    other._modifier2 = 0;
    other._modifier3 = 0;
    other._modifier4 = 0;
    return ;
}

ft_event &ft_event::operator=(ft_event &&other) noexcept
{
    if (this != &other)
    {
        this->_id = other._id;
        this->_duration = other._duration;
        this->_modifier1 = other._modifier1;
        this->_modifier2 = other._modifier2;
        this->_modifier3 = other._modifier3;
        this->_modifier4 = other._modifier4;
        this->_callback = ft_move(other._callback);
        other._id = 0;
        other._duration = 0;
        other._modifier1 = 0;
        other._modifier2 = 0;
        other._modifier3 = 0;
        other._modifier4 = 0;
    }
    return (*this);
}

int ft_event::get_id() const noexcept
{
    return (this->_id);
}

void ft_event::set_id(int id) noexcept
{
    this->_id = id;
    return ;
}

int ft_event::get_duration() const noexcept
{
    return (this->_duration);
}

void ft_event::set_duration(int duration) noexcept
{
    this->_duration = duration;
    return ;
}

void ft_event::add_duration(int duration) noexcept
{
    this->_duration += duration;
    return ;
}

void ft_event::sub_duration(int duration) noexcept
{
    this->_duration -= duration;
    return ;
}

int ft_event::get_modifier1() const noexcept
{
    return (this->_modifier1);
}

void ft_event::set_modifier1(int mod) noexcept
{
    this->_modifier1 = mod;
    return ;
}

void ft_event::add_modifier1(int mod) noexcept
{
    this->_modifier1 += mod;
    return ;
}

void ft_event::sub_modifier1(int mod) noexcept
{
    this->_modifier1 -= mod;
    return ;
}

int ft_event::get_modifier2() const noexcept
{
    return (this->_modifier2);
}

void ft_event::set_modifier2(int mod) noexcept
{
    this->_modifier2 = mod;
    return ;
}

void ft_event::add_modifier2(int mod) noexcept
{
    this->_modifier2 += mod;
    return ;
}

void ft_event::sub_modifier2(int mod) noexcept
{
    this->_modifier2 -= mod;
    return ;
}

int ft_event::get_modifier3() const noexcept
{
    return (this->_modifier3);
}

void ft_event::set_modifier3(int mod) noexcept
{
    this->_modifier3 = mod;
    return ;
}

void ft_event::add_modifier3(int mod) noexcept
{
    this->_modifier3 += mod;
    return ;
}

void ft_event::sub_modifier3(int mod) noexcept
{
    this->_modifier3 -= mod;
    return ;
}

int ft_event::get_modifier4() const noexcept
{
    return (this->_modifier4);
}

void ft_event::set_modifier4(int mod) noexcept
{
    this->_modifier4 = mod;
    return ;
}

void ft_event::add_modifier4(int mod) noexcept
{
    this->_modifier4 += mod;
    return ;
}

void ft_event::sub_modifier4(int mod) noexcept
{
    this->_modifier4 -= mod;
    return ;
}

const ft_function<void(ft_world&, ft_event&)> &ft_event::get_callback() const noexcept
{
    return (this->_callback);
}

void ft_event::set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept
{
    this->_callback = ft_move(callback);
    return ;
}
