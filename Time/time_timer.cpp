#include "time.hpp"
#include "timer.hpp"
#include <chrono>
#include <climits>
#include "../Errno/errno.hpp"

time_timer::time_timer() noexcept
    : _duration_ms(0), _start_time(std::chrono::steady_clock::time_point()), _running(false)
{
    (void)(FT_ERR_SUCCESS);
    return ;
}

time_timer::~time_timer() noexcept
{
    (void)(FT_ERR_SUCCESS);
    return ;
}

void    time_timer::start(long duration_ms) noexcept
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (duration_ms < 0)
        this->_running = false;
    else
    {
        this->_duration_ms = duration_ms;
        this->_start_time = std::chrono::steady_clock::now();
        this->_running = true;
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return ;
}

long time_timer::update() noexcept
{
    long remaining;
    int lock_error;
    int unlock_error;

    remaining = -1;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (remaining);
    if (this->_running)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            remaining = 0;
        }
        else
            remaining = this->_duration_ms - elapsed;
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return (remaining);
}

long time_timer::add_time(long amount_ms) noexcept
{
    long result;
    int lock_error;
    int unlock_error;

    result = -1;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (this->_running && amount_ms >= 0
        && !(amount_ms > 0 && this->_duration_ms > LONG_MAX - amount_ms))
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        this->_duration_ms += amount_ms;
        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            result = 0;
        }
        else
            result = this->_duration_ms - elapsed;
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return (result);
}

long time_timer::remove_time(long amount_ms) noexcept
{
    long result;
    int lock_error;
    int unlock_error;

    result = -1;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (this->_running && amount_ms >= 0)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (amount_ms >= this->_duration_ms - elapsed)
        {
            this->_duration_ms = elapsed;
            this->_running = false;
            result = 0;
        }
        else
        {
            this->_duration_ms -= amount_ms;
            result = this->_duration_ms - elapsed;
        }
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return (result);
}

void    time_timer::sleep_remaining() noexcept
{
    long remaining;
    int lock_error;
    int unlock_error;

    remaining = -1;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_running)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            remaining = 0;
        }
        else
            remaining = this->_duration_ms - elapsed;
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *time_timer::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}
#endif
