#ifndef TIME_TIMER_HPP
# define TIME_TIMER_HPP

#include <chrono>
#include <cstdint>
#include "../PThread/recursive_mutex.hpp"

class time_timer
{
    private:
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        long    _duration_ms;
        std::chrono::steady_clock::time_point _start_time;
        bool    _running;
        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void    abort_if_not_initialized(const char *method_name) const noexcept;
        int     lock_mutex(void) const noexcept;
        int     unlock_mutex(void) const noexcept;

    public:
        time_timer() noexcept;
        time_timer(const time_timer &) = delete;
        time_timer &operator=(const time_timer &) = delete;
        time_timer(time_timer &&) = delete;
        time_timer &operator=(time_timer &&) = delete;
        ~time_timer() noexcept;
        int     initialize(void) noexcept;
        int     destroy(void) noexcept;
        void    start(long duration_ms) noexcept;
        long    update(void) noexcept;
        long    add_time(long amount_ms) noexcept;
        long    remove_time(long amount_ms) noexcept;
        void    sleep_remaining(void) noexcept;
        int     enable_thread_safety(void) noexcept;
        void    disable_thread_safety(void) noexcept;
        bool    is_thread_safe(void) const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation(void) const noexcept;
#endif
};

#endif
