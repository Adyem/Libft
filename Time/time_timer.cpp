#include "time.hpp"
#include "timer.hpp"
#include <chrono>
#include <climits>
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

time_timer::time_timer() noexcept
{
    this->_mutex = ft_nullptr;
    this->_initialized_state = time_timer::_state_uninitialized;
    this->_duration_ms = 0;
    this->_start_time = std::chrono::steady_clock::time_point();
    this->_running = false;
    return ;
}

time_timer::~time_timer() noexcept
{
    if (this->_initialized_state == time_timer::_state_uninitialized)
    {
        this->abort_lifecycle_error("time_timer::~time_timer",
            "called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == time_timer::_state_initialized)
        (void)this->destroy();
    return ;
}

void    time_timer::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "time_timer lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void    time_timer::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == time_timer::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int     time_timer::lock_mutex(void) const noexcept
{
    this->abort_if_not_initialized("time_timer::lock_mutex");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->lock());
}

int     time_timer::unlock_mutex(void) const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int     time_timer::initialize(void) noexcept
{
    int enable_error;

    if (this->_initialized_state == time_timer::_state_initialized)
    {
        this->abort_lifecycle_error("time_timer::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_duration_ms = 0;
    this->_start_time = std::chrono::steady_clock::time_point();
    this->_running = false;
    this->_initialized_state = time_timer::_state_initialized;
    enable_error = this->enable_thread_safety();
    if (enable_error != FT_ERR_SUCCESS)
    {
        this->_initialized_state = time_timer::_state_destroyed;
        return (enable_error);
    }
    return (FT_ERR_SUCCESS);
}

int     time_timer::destroy(void) noexcept
{
    if (this->_initialized_state != time_timer::_state_initialized)
    {
        this->abort_lifecycle_error("time_timer::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->disable_thread_safety();
    this->_duration_ms = 0;
    this->_start_time = std::chrono::steady_clock::time_point();
    this->_running = false;
    this->_initialized_state = time_timer::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

void    time_timer::start(long duration_ms) noexcept
{
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("time_timer::start");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (duration_ms < 0)
        this->_running = false;
    else
    {
        this->_duration_ms = duration_ms;
        this->_start_time = std::chrono::steady_clock::now();
        this->_running = true;
    }
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return ;
}

long    time_timer::update(void) noexcept
{
    long remaining;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("time_timer::update");
    remaining = -1;
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (remaining);
    if (this->_running)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            remaining = 0;
        }
        else
            remaining = this->_duration_ms - elapsed;
    }
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return (remaining);
}

long time_timer::add_time(long amount_ms) noexcept
{
    long result;
    int lock_error;
    int unlock_error;

    result = -1;
    this->abort_if_not_initialized("time_timer::add_time");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (this->_running && amount_ms >= 0
        && !(amount_ms > 0 && this->_duration_ms > LONG_MAX - amount_ms))
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        this->_duration_ms += amount_ms;
        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            result = 0;
        }
        else
            result = this->_duration_ms - elapsed;
    }
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return (result);
}

long time_timer::remove_time(long amount_ms) noexcept
{
    long result;
    int lock_error;
    int unlock_error;

    result = -1;
    this->abort_if_not_initialized("time_timer::remove_time");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return (result);
    if (this->_running && amount_ms >= 0)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (amount_ms >= this->_duration_ms - elapsed)
        {
            this->_duration_ms = elapsed;
            this->_running = false;
            result = 0;
        }
        else
        {
            this->_duration_ms -= amount_ms;
            result = this->_duration_ms - elapsed;
        }
    }
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    return (result);
}

void    time_timer::sleep_remaining() noexcept
{
    long remaining;
    int lock_error;
    int unlock_error;

    remaining = -1;
    this->abort_if_not_initialized("time_timer::sleep_remaining");
    lock_error = this->lock_mutex();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_running)
    {
        std::chrono::steady_clock::time_point now;
        long elapsed;

        now = std::chrono::steady_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->_start_time).count();
        if (elapsed >= this->_duration_ms)
        {
            this->_running = false;
            remaining = 0;
        }
        else
            remaining = this->_duration_ms - elapsed;
    }
    unlock_error = this->unlock_mutex();
    (void)(unlock_error);
    if (remaining > 0)
        time_sleep_ms(static_cast<unsigned int>(remaining));
    return ;
}

int     time_timer::enable_thread_safety(void) noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("time_timer::enable_thread_safety");
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

void    time_timer::disable_thread_safety(void) noexcept
{
    this->abort_if_not_initialized("time_timer::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

bool    time_timer::is_thread_safe(void) const noexcept
{
    this->abort_if_not_initialized("time_timer::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *time_timer::get_mutex_for_validation(void) const noexcept
{
    this->abort_if_not_initialized("time_timer::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
