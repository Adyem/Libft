#include "time.hpp"
#include "time_fps.hpp"
#include "../Errno/errno.hpp"
#include <chrono>

time_fps::time_fps(long frames_per_second)
{
    this->set_error(ER_SUCCESS);
    if (frames_per_second < 24)
    {
        this->set_error(FT_EINVAL);
        this->_frame_duration_ms = 0;
        this->_last_frame_time = std::chrono::steady_clock::now();
        return ;
    }
    this->_frame_duration_ms = 1000 / frames_per_second;
    this->_last_frame_time = std::chrono::steady_clock::now();
    return ;
}

time_fps::~time_fps()
{
    return ;
}

long    time_fps::get_frames_per_second()
{
    if (this->_frame_duration_ms <= 0)
    {
        this->set_error(FT_EINVAL);
        return (0);
    }
    return (1000 / this->_frame_duration_ms);
}

int     time_fps::set_frames_per_second(long frames_per_second)
{
    if (frames_per_second < 24)
    {
        this->set_error(FT_EINVAL);
        this->_frame_duration_ms = 0;
        this->_last_frame_time = std::chrono::steady_clock::now();
        return (1);
    }
    this->_frame_duration_ms = 1000 / frames_per_second;
    this->_last_frame_time = std::chrono::steady_clock::now();
    this->set_error(ER_SUCCESS);
    return (0);
}

void    time_fps::sleep_to_next_frame()
{
    std::chrono::steady_clock::time_point now;
    long elapsed;

    if (this->_frame_duration_ms <= 0)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    now = std::chrono::steady_clock::now();
    elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_last_frame_time).count();
    if (elapsed < this->_frame_duration_ms)
        time_sleep_ms(static_cast<unsigned int>(this->_frame_duration_ms - elapsed));
    this->_last_frame_time = std::chrono::steady_clock::now();
    return ;
}

void    time_fps::set_error(int error_code)
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int     time_fps::get_error() const
{
    return (this->_error_code);
}

const char  *time_fps::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

