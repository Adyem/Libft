#include "time.hpp"
#include "../Errno/errno.hpp"
#include "../Networking/networking.hpp"
#include <climits>

void    time_async_sleep_init(t_time_async_sleep *sleep_state, long long delay_milliseconds)
{
    t_monotonic_time_point    now_point;
    long long                 normalized_delay;

    if (!sleep_state)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    now_point = time_monotonic_point_now();
    normalized_delay = delay_milliseconds;
    if (normalized_delay < 0)
        normalized_delay = 0;
    sleep_state->deadline = time_monotonic_point_add_ms(now_point, normalized_delay);
    if (normalized_delay == 0)
        sleep_state->completed = true;
    else
        sleep_state->completed = false;
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

bool    time_async_sleep_is_complete(const t_time_async_sleep *sleep_state)
{
    if (!sleep_state)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (true);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (sleep_state->completed);
}

long long   time_async_sleep_remaining_ms(t_time_async_sleep *sleep_state)
{
    t_monotonic_time_point    now_point;
    long long                 remaining;

    if (!sleep_state)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    if (sleep_state->completed)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    now_point = time_monotonic_point_now();
    remaining = time_monotonic_point_diff_ms(now_point, sleep_state->deadline);
    if (remaining <= 0)
    {
        sleep_state->completed = true;
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (remaining);
}

int time_async_sleep_poll(event_loop *loop, t_time_async_sleep *sleep_state)
{
    long long   remaining;
    int         timeout;
    int         poll_result;

    if (!loop || !sleep_state)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    remaining = time_async_sleep_remaining_ms(sleep_state);
    if (sleep_state->completed)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    if (remaining > static_cast<long long>(INT_MAX))
        timeout = INT_MAX;
    else
        timeout = static_cast<int>(remaining);
    poll_result = event_loop_run(loop, timeout);
    if (poll_result < 0)
        return (-1);
    if (poll_result == 0)
    {
        remaining = time_async_sleep_remaining_ms(sleep_state);
        if (sleep_state->completed)
        {
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
    }
    ft_errno = FT_ER_SUCCESSS;
    return (1);
}

