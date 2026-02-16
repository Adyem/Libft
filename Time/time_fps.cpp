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
    int lock_error;
    int unlock_error;

    frames_per_second_value = 0;
    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (frames_per_second_value);
    if (this->_frames_per_second > 0)
        frames_per_second_value = this->_frames_per_second;
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return (frames_per_second_value);
}

int     time_fps::set_frames_per_second(long frames_per_second)
{
    int lock_error;
    int unlock_error;
    int result;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    result = -1;
    if (frames_per_second < 24)
    {
        this->_frames_per_second = 0;
        this->_frame_duration_ms = 0.0;
        this->_last_frame_time = std::chrono::steady_clock::now();
        result = -1;
    }
    else
    {
        this->_frames_per_second = frames_per_second;
        this->_frame_duration_ms = 1000.0 / static_cast<double>(frames_per_second);
        this->_last_frame_time = std::chrono::steady_clock::now();
        result = 0;
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return (result);
}

void    time_fps::sleep_to_next_frame()
{
    int lock_error;
    int unlock_error;

    lock_error = this->_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_frame_duration_ms > 0.0)
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
    }
    unlock_error = this->_mutex.unlock();
    (void)(unlock_error);
    return ;
}
