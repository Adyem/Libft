#ifndef TIME_FPS_HPP
# define TIME_FPS_HPP

#include <chrono>
#include <cstdint>
#include "../PThread/recursive_mutex.hpp"

class time_fps
{
    private:
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        double  _frame_duration_ms;
        long    _frames_per_second;
        std::chrono::steady_clock::time_point _last_frame_time;
        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void    abort_if_not_initialized(const char *method_name) const noexcept;
        int     lock_mutex(void) const noexcept;
        int     unlock_mutex(void) const noexcept;

    public:
        time_fps(void) noexcept;
        time_fps(const time_fps &) = delete;
        time_fps &operator=(const time_fps &) = delete;
        time_fps(time_fps &&) = delete;
        time_fps &operator=(time_fps &&) = delete;
        ~time_fps() noexcept;
        int     initialize(long frames_per_second) noexcept;
        int     destroy(void) noexcept;
        long    get_frames_per_second(void) noexcept;
        int     set_frames_per_second(long frames_per_second) noexcept;
        void    sleep_to_next_frame(void) noexcept;
        int     enable_thread_safety(void) noexcept;
        int     disable_thread_safety(void) noexcept;
        bool    is_thread_safe(void) const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation(void) const noexcept;
#endif
};

#endif
