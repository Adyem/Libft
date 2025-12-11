#include "time.hpp"
#include "timer.hpp"
#include <chrono>
#include <climits>
#include "../Errno/errno.hpp"

time_timer::time_timer() noexcept
    : _duration_ms(0), _start_time(std::chrono::steady_clock::time_point()), _running(false), _error_code(FT_ERR_SUCCESSS)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

time_timer::~time_timer() noexcept
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

void    time_timer::start(long duration_ms) noexcept
{
    int final_error;

    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
        }
        else if (duration_ms < 0)
        {
            this->_running = false;
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
        }
        else
        {
            this->_duration_ms = duration_ms;
            this->_start_time = std::chrono::steady_clock::now();
            this->_running = true;
            this->set_error(FT_ERR_SUCCESSS);
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return ;
}

long time_timer::update() noexcept
{
    long remaining;
    int final_error;

    remaining = -1;
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
            remaining = -1;
        }
        else if (!this->_running)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
            remaining = -1;
        }
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;

            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
            if (elapsed >= this->_duration_ms)
            {
                this->_running = false;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
                remaining = 0;
            }
            else
            {
                remaining = this->_duration_ms - elapsed;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
            }
        }
    }
    ft_errno = final_error;
    return (remaining);
}

long time_timer::add_time(long amount_ms) noexcept
{
    long result;
    int final_error;

    result = -1;
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
            result = -1;
        }
        else if (!this->_running || amount_ms < 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
            result = -1;
        }
        else if (amount_ms > 0 && this->_duration_ms > LONG_MAX - amount_ms)
        {
            this->set_error(FT_ERR_OUT_OF_RANGE);
            final_error = this->_error_code;
            result = -1;
        }
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;

            this->_duration_ms += amount_ms;
            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
            if (elapsed >= this->_duration_ms)
            {
                this->_running = false;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
                result = 0;
            }
            else
            {
                result = this->_duration_ms - elapsed;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
            }
        }
    }
    ft_errno = final_error;
    return (result);
}

long time_timer::remove_time(long amount_ms) noexcept
{
    long result;
    int final_error;

    result = -1;
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
            result = -1;
        }
        else if (!this->_running || amount_ms < 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
            result = -1;
        }
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;

            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
            if (amount_ms >= this->_duration_ms - elapsed)
            {
                this->_duration_ms = elapsed;
                this->_running = false;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
                result = 0;
            }
            else
            {
                this->_duration_ms -= amount_ms;
                result = this->_duration_ms - elapsed;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
            }
        }
    }
    ft_errno = final_error;
    return (result);
}

void    time_timer::sleep_remaining() noexcept
{
    int final_error;
    long remaining;

    final_error = FT_ERR_SUCCESSS;
    remaining = -1;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
        }
        else if (!this->_running)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
        }
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;

            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
            if (elapsed >= this->_duration_ms)
            {
                this->_running = false;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
                remaining = 0;
            }
            else
            {
                remaining = this->_duration_ms - elapsed;
                this->set_error(FT_ERR_SUCCESSS);
                final_error = this->_error_code;
            }
        }
    }
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    ft_errno = final_error;
    return ;
}

int time_timer::get_error() const noexcept
{
    int error_code_value;
    int final_error;

    error_code_value = FT_ERR_SUCCESSS;
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<time_timer *>(this)->set_error(guard.get_error());
            error_code_value = guard.get_error();
            final_error = guard.get_error();
        }
        else
        {
            error_code_value = this->_error_code;
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return (error_code_value);
}

const char  *time_timer::get_error_str() const noexcept
{
    const char  *error_string;
    int final_error;

    error_string = ft_strerror(FT_ERR_SUCCESSS);
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<time_timer *>(this)->set_error(guard.get_error());
            error_string = ft_strerror(guard.get_error());
            final_error = guard.get_error();
        }
        else
        {
            error_string = ft_strerror(this->_error_code);
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return (error_string);
}

void    time_timer::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
