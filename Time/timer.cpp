#include "time.hpp"
#include "timer.hpp"
#include <chrono>

ft_timer::ft_timer()
{
    return ;
}

ft_timer::~ft_timer()
{
    return ;
}

void    ft_timer::start(long duration_ms)
{
    if (duration_ms < 0)
    {
        this->_running = false;
        return ;
    }
    this->_duration_ms = duration_ms;
    this->_start_time = std::chrono::steady_clock::now();
    this->_running = true;
    return ;
}

long ft_timer::update()
{
    if (!this->_running)
        return (-1);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (elapsed >= this->_duration_ms)
    {
        this->_running = false;
        return (0);
    }
    return (this->_duration_ms - elapsed);
}

long ft_timer::add_time(long amount_ms)
{
    if (!this->_running || amount_ms < 0)
        return (-1);
    this->_duration_ms += amount_ms;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (elapsed >= this->_duration_ms)
    {
        this->_running = false;
        return (0);
    }
    return (this->_duration_ms - elapsed);
}

long ft_timer::remove_time(long amount_ms)
{
    if (!this->_running || amount_ms < 0)
        return (-1);
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (amount_ms >= this->_duration_ms - elapsed)
    {
        this->_duration_ms = elapsed;
        this->_running = false;
        return (0);
    }
    this->_duration_ms -= amount_ms;
    return (this->_duration_ms - elapsed);
}

void    ft_timer::sleep_remaining()
{
    long remaining = this->update();
    if (remaining > 0)
        ft_sleep_ms(static_cast<unsigned int>(remaining));
    return ;
}

