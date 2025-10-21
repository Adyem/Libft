#include "time.hpp"
#include "fps.hpp"
#include "../Errno/errno.hpp"
#include <chrono>

time_fps::time_fps(long frames_per_second)
{
    this->_frame_duration_ms = 0.0;
    this->_frames_per_second = 0;
    this->_last_frame_time = std::chrono::steady_clock::now();
    this->set_error(ER_SUCCESS);
    if (frames_per_second < 24)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
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
    final_error = ER_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
        }
        else if (this->_frames_per_second <= 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
        }
        else
        {
            frames_per_second_value = this->_frames_per_second;
            this->set_error(ER_SUCCESS);
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return (frames_per_second_value);
}

int     time_fps::set_frames_per_second(long frames_per_second)
{
    int final_error;
    int result;

    final_error = ER_SUCCESS;
    result = -1;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
            result = -1;
        }
        else if (frames_per_second < 24)
        {
            this->_frames_per_second = 0;
            this->_frame_duration_ms = 0.0;
            this->_last_frame_time = std::chrono::steady_clock::now();
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
            result = -1;
        }
        else
        {
            this->_frames_per_second = frames_per_second;
            this->_frame_duration_ms = 1000.0 / static_cast<double>(frames_per_second);
            this->_last_frame_time = std::chrono::steady_clock::now();
            this->set_error(ER_SUCCESS);
            final_error = this->_error_code;
            result = 0;
        }
    }
    ft_errno = final_error;
    return (result);
}

void    time_fps::sleep_to_next_frame()
{
    int final_error;

    final_error = ER_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            final_error = this->_error_code;
        }
        else if (this->_frame_duration_ms <= 0.0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            final_error = this->_error_code;
        }
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
            this->set_error(ER_SUCCESS);
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
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
    int error_code_value;
    int final_error;

    error_code_value = ER_SUCCESS;
    final_error = ER_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<time_fps *>(this)->set_error(guard.get_error());
            error_code_value = guard.get_error();
            final_error = guard.get_error();
        }
        else
        {
            error_code_value = this->_error_code;
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return (error_code_value);
}

const char  *time_fps::get_error_str() const
{
    const char  *error_string;
    int final_error;

    error_string = ft_strerror(ER_SUCCESS);
    final_error = ER_SUCCESS;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<time_fps *>(this)->set_error(guard.get_error());
            error_string = ft_strerror(guard.get_error());
            final_error = guard.get_error();
        }
        else
        {
            error_string = ft_strerror(this->_error_code);
            final_error = this->_error_code;
        }
    }
    ft_errno = final_error;
    return (error_string);
}
