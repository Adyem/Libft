#ifndef THIS_THREAD_HPP
#define THIS_THREAD_HPP

#include <thread>
#include <chrono>

std::thread::id ft_this_thread_get_id();
void ft_this_thread_sleep_for(std::chrono::milliseconds duration);
void ft_this_thread_sleep_until(std::chrono::steady_clock::time_point time_point);
void ft_this_thread_yield();

#endif
