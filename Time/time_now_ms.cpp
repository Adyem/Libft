#include "time.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <chrono>
#include <climits>

static t_time_clock_now_hook    g_time_now_ms_hook = ft_nullptr;

void    time_set_clock_now_hook(t_time_clock_now_hook hook)
{
    if (hook != ft_nullptr)
    {
        g_time_now_ms_hook = hook;
        return ;
    }
    g_time_now_ms_hook = ft_nullptr;
    return ;
}

void    time_reset_clock_now_hook(void)
{
    g_time_now_ms_hook = ft_nullptr;
    return ;
}

long    time_now_ms(void)
{
    std::chrono::system_clock::time_point time_now;
    std::chrono::milliseconds milliseconds;
    long long milliseconds_count;

    if (g_time_now_ms_hook != ft_nullptr)
        time_now = g_time_now_ms_hook();
    else
        time_now = std::chrono::system_clock::now();
    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    milliseconds_count = milliseconds.count();
    if (milliseconds_count > static_cast<long long>(LONG_MAX))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (LONG_MAX);
    }
    if (milliseconds_count < static_cast<long long>(LONG_MIN))
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (LONG_MIN);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<long>(milliseconds_count));
}

