#ifndef TIME_TIMER_HPP
# define TIME_TIMER_HPP

#include <chrono>

class time_timer
{
    private:
        long    _duration_ms = 0;
        std::chrono::steady_clock::time_point _start_time = std::chrono::steady_clock::time_point();
        bool    _running = false;
        mutable int _error_code;

        void    set_error(int error_code) const noexcept;

    public:
        time_timer() noexcept;
        ~time_timer() noexcept;
        void    start(long duration_ms) noexcept;
        long    update() noexcept;
        long    add_time(long amount_ms) noexcept;
        long    remove_time(long amount_ms) noexcept;
        void    sleep_remaining() noexcept;
        int     get_error() const noexcept;
        const char  *get_error_str() const noexcept;
};

#endif
