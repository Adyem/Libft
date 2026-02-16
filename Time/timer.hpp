#ifndef TIME_TIMER_HPP
# define TIME_TIMER_HPP

#include <chrono>
#include "../PThread/mutex.hpp"

class time_timer
{
    private:
        long    _duration_ms = 0;
        std::chrono::steady_clock::time_point _start_time = std::chrono::steady_clock::time_point();
        bool    _running = false;
        mutable pt_mutex _mutex;

    public:
        time_timer() noexcept;
        ~time_timer() noexcept;
        void    start(long duration_ms) noexcept;
        long    update() noexcept;
        long    add_time(long amount_ms) noexcept;
        long    remove_time(long amount_ms) noexcept;
        void    sleep_remaining() noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_mutex    *get_mutex_for_validation() const noexcept;
#endif
};

#endif
