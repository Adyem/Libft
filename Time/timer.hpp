#ifndef TIMER_HPP
# define TIMER_HPP

#include <chrono>

class ft_timer
{
    private:
        long    _duration_ms = 0;
        std::chrono::steady_clock::time_point _start_time = std::chrono::steady_clock::time_point();
        bool    _running = false;

    public:
        ft_timer();
        ~ft_timer();
        void    start(long duration_ms);
        long    update();
        long    add_time(long amount_ms);
        long    remove_time(long amount_ms);
        void    sleep_remaining();
};

#endif
