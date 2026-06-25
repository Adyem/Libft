#ifndef TIME_TIMER_HPP
# define TIME_TIMER_HPP

#include <chrono>
#include <cstdint>
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class time_timer
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        int64_t _duration_ms;
        std::chrono::steady_clock::time_point _start_time;
        ft_bool _running;

    public:
        time_timer() noexcept;
        time_timer(const time_timer &other) noexcept = delete;
        time_timer(time_timer &&other) noexcept = delete;
        ~time_timer() noexcept;

        time_timer &operator=(const time_timer &) = delete;
        time_timer &operator=(time_timer &&) = delete;

        int32_t initialize(void) noexcept;
        int32_t initialize(const time_timer &other) noexcept;
        int32_t initialize(time_timer &&other) noexcept;
        int32_t destroy(void) noexcept;
        int32_t move(time_timer &other) noexcept;

        void    start(int64_t duration_ms) noexcept;
        int64_t update(void) noexcept;
        int64_t add_time(int64_t amount_ms) noexcept;
        int64_t remove_time(int64_t amount_ms) noexcept;
        void    sleep_remaining(void) noexcept;

        int32_t enable_thread_safety(void) noexcept;
        int32_t disable_thread_safety(void) noexcept;
        ft_bool is_thread_safe(void) const noexcept;
};

#endif
