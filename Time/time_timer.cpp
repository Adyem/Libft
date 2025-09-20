#include "time.hpp"
#include "timer.hpp"
#include <chrono>
#include "../Errno/errno.hpp"

time_timer::time_timer() noexcept
    : _duration_ms(0), _start_time(std::chrono::steady_clock::time_point()), _running(false), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

time_timer::~time_timer() noexcept
{
    this->set_error(ER_SUCCESS);
    return ;
}

void    time_timer::start(long duration_ms) noexcept
{
    if (duration_ms < 0)
    {
        this->_running = false;
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_duration_ms = duration_ms;
    this->_start_time = std::chrono::steady_clock::now();
    this->_running = true;
    this->set_error(ER_SUCCESS);
    return ;
}

long time_timer::update() noexcept
{
    if (!this->_running)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (elapsed >= this->_duration_ms)
    {
        this->_running = false;
        this->set_error(ER_SUCCESS);
        return (0);
    }
    long remaining = this->_duration_ms - elapsed;
    this->set_error(ER_SUCCESS);
    return (remaining);
}

long time_timer::add_time(long amount_ms) noexcept
{
    if (!this->_running || amount_ms < 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->_duration_ms += amount_ms;
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (elapsed >= this->_duration_ms)
    {
        this->_running = false;
        this->set_error(ER_SUCCESS);
        return (0);
    }
    long remaining = this->_duration_ms - elapsed;
    this->set_error(ER_SUCCESS);
    return (remaining);
}

long time_timer::remove_time(long amount_ms) noexcept
{
    if (!this->_running || amount_ms < 0)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
    if (amount_ms >= this->_duration_ms - elapsed)
    {
        this->_duration_ms = elapsed;
        this->_running = false;
        this->set_error(ER_SUCCESS);
        return (0);
    }
    this->_duration_ms -= amount_ms;
    long remaining = this->_duration_ms - elapsed;
    this->set_error(ER_SUCCESS);
    return (remaining);
}

void    time_timer::sleep_remaining() noexcept
{
    long remaining = this->update();
    if (remaining < 0)
        return ;
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    this->set_error(ER_SUCCESS);
    return ;
}

int time_timer::get_error() const noexcept
{
    return (this->_error_code);
}

const char  *time_timer::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void    time_timer::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}


