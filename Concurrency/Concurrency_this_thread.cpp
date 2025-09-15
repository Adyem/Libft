#include "this_thread.hpp"

std::thread::id ft_this_thread_get_id()
{
    return (std::this_thread::get_id());
}

void ft_this_thread_sleep_for(std::chrono::milliseconds duration)
{
    std::this_thread::sleep_for(duration);
    return ;
}

void ft_this_thread_sleep_until(std::chrono::steady_clock::time_point time_point)
{
    std::this_thread::sleep_until(time_point);
    return ;
}

void ft_this_thread_yield()
{
    std::this_thread::yield();
    return ;
}
