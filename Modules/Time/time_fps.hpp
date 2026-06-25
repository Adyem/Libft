#ifndef TIME_FPS_HPP
# define TIME_FPS_HPP

#include <chrono>
#include <cstdint>
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class time_fps
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        double _frame_duration_ms;
        int64_t _frames_per_second;
        std::chrono::steady_clock::time_point _last_frame_time;

    public:
        time_fps(void) noexcept;
        time_fps(const time_fps &other) noexcept = delete;
        time_fps(time_fps &&other) noexcept = delete;
        ~time_fps() noexcept;

        time_fps &operator=(const time_fps &) = delete;
        time_fps &operator=(time_fps &&) = delete;

        int32_t initialize(void) noexcept;
        int32_t initialize(const time_fps &other) noexcept;
        int32_t initialize(time_fps &&other) noexcept;
        int32_t destroy(void) noexcept;
        int32_t move(time_fps &other) noexcept;

        int64_t get_frames_per_second(void) noexcept;
        int32_t set_frames_per_second(int64_t frames_per_second) noexcept;
        void    sleep_to_next_frame(void) noexcept;

        int32_t enable_thread_safety(void) noexcept;
        int32_t disable_thread_safety(void) noexcept;
        ft_bool is_thread_safe(void) const noexcept;
};

#endif
