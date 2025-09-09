#include "time.hpp"
#include "timer.hpp"
#include <chrono>

time_timer::time_timer()
{
    return ;
}

time_timer::~time_timer()
{
    return ;
}

void    time_timer::start(long duration_ms)
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

long time_timer::update()
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

long time_timer::add_time(long amount_ms)
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

long time_timer::remove_time(long amount_ms)
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

void    time_timer::sleep_remaining()
{
    long remaining;

    remaining = this->update();
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    return ;
}

