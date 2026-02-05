#include "time.hpp"
#include "fps.hpp"
#include "../Errno/errno.hpp"
#include <chrono>

time_fps::time_fps(long frames_per_second)
{
    this->_frame_duration_ms = 0.0;
    this->_frames_per_second = 0;
    this->_last_frame_time = std::chrono::steady_clock::now();
    if (frames_per_second < 24)
        return ;
    this->_frames_per_second = frames_per_second;
    this->_frame_duration_ms = 1000.0 / static_cast<double>(frames_per_second);
    return ;
}

time_fps::~time_fps()
{
    return ;
}

long    time_fps::get_frames_per_second()
{
    long frames_per_second_value;
    int final_error;

    frames_per_second_value = 0;
    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        int guard_error = ft_global_error_stack_pop_newest();

        if (guard_error != FT_ERR_SUCCESSS)
            final_error = guard_error;
        else if (this->_frames_per_second <= 0)
            final_error = FT_ERR_INVALID_ARGUMENT;
        else
        {
            frames_per_second_value = this->_frames_per_second;
            final_error = FT_ERR_SUCCESSS;
        }
    }
    ft_global_error_stack_push(final_error);
    return (frames_per_second_value);
}

int     time_fps::set_frames_per_second(long frames_per_second)
{
    int final_error;
    int result;

    final_error = FT_ERR_SUCCESSS;
    result = -1;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        int guard_error = ft_global_error_stack_pop_newest();

        if (guard_error != FT_ERR_SUCCESSS)
        {
            final_error = guard_error;
            result = -1;
        }
        else if (frames_per_second < 24)
        {
            this->_frames_per_second = 0;
            this->_frame_duration_ms = 0.0;
            this->_last_frame_time = std::chrono::steady_clock::now();
            final_error = FT_ERR_INVALID_ARGUMENT;
            result = -1;
        }
        else
        {
            this->_frames_per_second = frames_per_second;
            this->_frame_duration_ms = 1000.0 / static_cast<double>(frames_per_second);
            this->_last_frame_time = std::chrono::steady_clock::now();
            final_error = FT_ERR_SUCCESSS;
            result = 0;
        }
    }
    ft_global_error_stack_push(final_error);
    return (result);
}

void    time_fps::sleep_to_next_frame()
{
    int final_error;

    final_error = FT_ERR_SUCCESSS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        int guard_error = ft_global_error_stack_pop_newest();

        if (guard_error != FT_ERR_SUCCESSS)
            final_error = guard_error;
        else if (this->_frame_duration_ms <= 0.0)
            final_error = FT_ERR_INVALID_ARGUMENT;
        else
        {
            std::chrono::steady_clock::time_point now;
            long elapsed;
            double remaining_ms;

            now = std::chrono::steady_clock::now();
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_last_frame_time).count();
            remaining_ms = this->_frame_duration_ms - static_cast<double>(elapsed);
            if (remaining_ms > 0.0)
                time_sleep_ms(static_cast<unsigned int>(remaining_ms));
            this->_last_frame_time = std::chrono::steady_clock::now();
            final_error = FT_ERR_SUCCESSS;
        }
    }
    ft_global_error_stack_push(final_error);
    return ;
}
