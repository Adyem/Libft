#ifndef TIME_FPS_HPP
# define TIME_FPS_HPP

#include <chrono>

class time_fps
{
    private:
        long    _frame_duration_ms = 0;
        std::chrono::steady_clock::time_point _last_frame_time = std::chrono::steady_clock::time_point();
        mutable int _error_code;

        void    set_error(int error_code);

    public:
        time_fps(long frames_per_second);
        ~time_fps();
        long    get_frames_per_second();
        int     set_frames_per_second(long frames_per_second);
        void    sleep_to_next_frame();
        int     get_error() const;
        const char    *get_error_str() const;
};

#endif
