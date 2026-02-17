#include "time.hpp"
#include "fps.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <chrono>
#include <new>

void    time_fps::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "time_fps lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void    time_fps::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == time_fps::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int     time_fps::lock_mutex(void) const noexcept
{
    this->abort_if_not_initialized("time_fps::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int     time_fps::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

time_fps::time_fps(void) noexcept
{
    this->_mutex = ft_nullptr;
    this->_initialized_state = time_fps::_state_uninitialized;
    this->_frame_duration_ms = 0.0;
    this->_frames_per_second = 0;
    this->_last_frame_time = std::chrono::steady_clock::time_point();
    return ;
}

time_fps::~time_fps() noexcept
{
    if (this->_initialized_state == time_fps::_state_uninitialized)
    {
        this->abort_lifecycle_error("time_fps::~time_fps",
            "called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == time_fps::_state_initialized)
        (void)this->destroy();
    return ;
}

int     time_fps::initialize(long frames_per_second) noexcept
{
    int enable_error;

    if (this->_initialized_state == time_fps::_state_initialized)
    {
        this->abort_lifecycle_error("time_fps::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_frame_duration_ms = 0.0;
    this->_frames_per_second = 0;
    this->_last_frame_time = std::chrono::steady_clock::now();
    if (frames_per_second >= 24)
    {
        this->_frames_per_second = frames_per_second;
        this->_frame_duration_ms = 1000.0 / static_cast<double>(frames_per_second);
    }
    this->_initialized_state = time_fps::_state_initialized;
    enable_error = this->enable_thread_safety();
    if (enable_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = time_fps::_state_destroyed;
        return (enable_error);
    }
    return (FT_ERR_SUCCESS);
}

int     time_fps::destroy(void) noexcept
{
    if (this->_initialized_state != time_fps::_state_initialized)
    {
        this->abort_lifecycle_error("time_fps::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->disable_thread_safety();
    this->_frame_duration_ms = 0.0;
    this->_frames_per_second = 0;
    this->_last_frame_time = std::chrono::steady_clock::time_point();
    this->_initialized_state = time_fps::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

long    time_fps::get_frames_per_second(void) noexcept
{
    long frames_per_second_value;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("time_fps::get_frames_per_second");
    frames_per_second_value = 0;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (frames_per_second_value);
    if (this->_frames_per_second > 0)
        frames_per_second_value = this->_frames_per_second;
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return (frames_per_second_value);
}

int     time_fps::set_frames_per_second(long frames_per_second) noexcept
{
    int lock_error;
    int unlock_error;
    int result;

    this->abort_if_not_initialized("time_fps::set_frames_per_second");
    lock_error = this->lock_mutex();
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
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return (result);
}

void    time_fps::sleep_to_next_frame(void) noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("time_fps::sleep_to_next_frame");
    lock_error = this->lock_mutex();
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
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return ;
}

int     time_fps::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("time_fps::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

void    time_fps::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("time_fps::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

bool    time_fps::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("time_fps::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *time_fps::get_mutex_for_validation(void) const noexcept
{
    this->abort_if_not_initialized("time_fps::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
