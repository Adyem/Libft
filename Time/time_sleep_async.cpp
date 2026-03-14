#include "time.hpp"
#include "../Errno/errno.hpp"
#include <climits>

int32_t event_loop_run(event_loop *loop, int32_t timeout_milliseconds);

void    time_async_sleep_init(t_time_async_sleep *sleep_state, int64_t delay_milliseconds)
{
    t_monotonic_time_point    now_point;
    int64_t                 normalized_delay;

    if (!sleep_state)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    now_point = time_monotonic_point_now();
    normalized_delay = delay_milliseconds;
    if (normalized_delay < 0)
        normalized_delay = 0;
    sleep_state->deadline = time_monotonic_point_add_ms(now_point, normalized_delay);
    if (normalized_delay == 0)
        sleep_state->completed = FT_TRUE;
    else
        sleep_state->completed = FT_FALSE;
    (void)(FT_ERR_SUCCESS);
    return ;
}

ft_bool    time_async_sleep_is_complete(const t_time_async_sleep *sleep_state)
{
    if (!sleep_state)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_TRUE);
    }
    (void)(FT_ERR_SUCCESS);
    return (sleep_state->completed);
}

int64_t   time_async_sleep_remaining_ms(t_time_async_sleep *sleep_state)
{
    t_monotonic_time_point    now_point;
    int64_t                 remaining;

    if (!sleep_state)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (sleep_state->completed)
    {
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    now_point = time_monotonic_point_now();
    remaining = time_monotonic_point_diff_ms(now_point, sleep_state->deadline);
    if (remaining <= 0)
    {
        sleep_state->completed = FT_TRUE;
        (void)(FT_ERR_SUCCESS);
        return (0);
    }
    (void)(FT_ERR_SUCCESS);
    return (remaining);
}

int32_t time_async_sleep_poll(event_loop *loop, t_time_async_sleep *sleep_state)
{
    int64_t   remaining;
    int32_t         timeout;
    int32_t         poll_result;
    int32_t         error_code;

    if (!loop || !sleep_state)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    remaining = time_async_sleep_remaining_ms(sleep_state);
    error_code = FT_ERR_SUCCESS;
    if (error_code != FT_ERR_SUCCESS)
    {
        (void)(error_code);
        return (error_code);
    }
    if (sleep_state->completed)
    {
        (void)(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (remaining > static_cast<int64_t>(INT_MAX))
        timeout = INT_MAX;
    else
        timeout = static_cast<int32_t>(remaining);
    poll_result = event_loop_run(loop, timeout);
    if (poll_result < 0)
    {
        (void)(FT_ERR_IO);
        return (FT_ERR_IO);
    }
    if (poll_result == 0)
    {
        remaining = time_async_sleep_remaining_ms(sleep_state);
        error_code = FT_ERR_SUCCESS;
        if (error_code != FT_ERR_SUCCESS)
        {
            (void)(error_code);
            return (error_code);
        }
        if (sleep_state->completed)
        {
            (void)(FT_ERR_SUCCESS);
            return (FT_ERR_SUCCESS);
        }
    }
    (void)(FT_ERR_TIMEOUT);
    return (FT_ERR_TIMEOUT);
}
