#ifndef TIME_FPS_HPP
# define TIME_FPS_HPP

#include <chrono>
#include "../PThread/mutex.hpp"

class time_fps
{
    private:
        double  _frame_duration_ms;
        long    _frames_per_second;
        std::chrono::steady_clock::time_point _last_frame_time;
        mutable pt_mutex _mutex;

    public:
        time_fps(long frames_per_second);
        ~time_fps();
        long    get_frames_per_second();
        int     set_frames_per_second(long frames_per_second);
        void    sleep_to_next_frame();
};

#endif
