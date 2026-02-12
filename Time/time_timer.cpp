#include "time.hpp"
#include "timer.hpp"
#include <chrono>
#include <climits>
#include "../Errno/errno.hpp"

time_timer::time_timer() noexcept
    : _duration_ms(0), _start_time(std::chrono::steady_clock::time_point()), _running(false)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

time_timer::~time_timer() noexcept
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

void    time_timer::start(long duration_ms) noexcept
{
    int final_error;

    final_error = FT_ERR_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        int guard_error = ft_global_error_stack_drop_last_error();

        if (guard_error != FT_ERR_SUCCESS)
            final_error = guard_error;
        else if (duration_ms < 0)
        {
            this->_running = false;
            final_error = FT_ERR_INVALID_ARGUMENT;
        }
        else
        {
            this->_duration_ms = duration_ms;
            this->_start_time = std::chrono::steady_clock::now();
            this->_running = true;
            final_error = FT_ERR_SUCCESS;
        }
    }
    ft_global_error_stack_push(final_error);
    return ;
}

long time_timer::update() noexcept
{
    long remaining;
    int final_error;

    remaining = -1;
    final_error = FT_ERR_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        int guard_error = ft_global_error_stack_drop_last_error();

        if (guard_error != FT_ERR_SUCCESS)
        {
            final_error = guard_error;
            remaining = -1;
        }
        else if (!this->_running)
        {
            final_error = FT_ERR_INVALID_ARGUMENT;
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
                final_error = FT_ERR_SUCCESS;
                remaining = 0;
            }
            else
            {
                remaining = this->_duration_ms - elapsed;
                final_error = FT_ERR_SUCCESS;
            }
        }
    }
    ft_global_error_stack_push(final_error);
    return (remaining);
}

long time_timer::add_time(long amount_ms) noexcept
{
    long result;
    int final_error;

    result = -1;
    final_error = FT_ERR_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        int guard_error = ft_global_error_stack_drop_last_error();

        if (guard_error != FT_ERR_SUCCESS)
        {
            final_error = guard_error;
            result = -1;
        }
        else if (!this->_running || amount_ms < 0)
        {
            final_error = FT_ERR_INVALID_ARGUMENT;
            result = -1;
        }
        else if (amount_ms > 0 && this->_duration_ms > LONG_MAX - amount_ms)
        {
            final_error = FT_ERR_OUT_OF_RANGE;
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
                final_error = FT_ERR_SUCCESS;
                result = 0;
            }
            else
            {
                result = this->_duration_ms - elapsed;
                final_error = FT_ERR_SUCCESS;
            }
        }
    }
    ft_global_error_stack_push(final_error);
    return (result);
}

long time_timer::remove_time(long amount_ms) noexcept
{
    long result;
    int final_error;

    result = -1;
    final_error = FT_ERR_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        int guard_error = ft_global_error_stack_drop_last_error();

        if (guard_error != FT_ERR_SUCCESS)
        {
            final_error = guard_error;
            result = -1;
        }
        else if (!this->_running || amount_ms < 0)
        {
            final_error = FT_ERR_INVALID_ARGUMENT;
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
                final_error = FT_ERR_SUCCESS;
                result = 0;
            }
            else
            {
                this->_duration_ms -= amount_ms;
                result = this->_duration_ms - elapsed;
                final_error = FT_ERR_SUCCESS;
            }
        }
    }
    ft_global_error_stack_push(final_error);
    return (result);
}

void    time_timer::sleep_remaining() noexcept
{
    int final_error;
    long remaining;

    final_error = FT_ERR_SUCCESS;
    remaining = -1;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        int guard_error = ft_global_error_stack_drop_last_error();

        if (guard_error != FT_ERR_SUCCESS)
            final_error = guard_error;
        else if (!this->_running)
            final_error = FT_ERR_INVALID_ARGUMENT;
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;

            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
            if (elapsed >= this->_duration_ms)
            {
                this->_running = false;
                final_error = FT_ERR_SUCCESS;
                remaining = 0;
            }
            else
            {
                remaining = this->_duration_ms - elapsed;
                final_error = FT_ERR_SUCCESS;
            }
        }
    }
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    ft_global_error_stack_push(final_error);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_mutex *time_timer::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}
#endif
