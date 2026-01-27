#ifndef TIME_TIMER_HPP
# define TIME_TIMER_HPP

#include <chrono>
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class time_timer
{
    private:
        long    _duration_ms = 0;
        std::chrono::steady_clock::time_point _start_time = std::chrono::steady_clock::time_point();
        bool    _running = false;
        mutable pt_mutex _mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};

        void    record_operation_error(int error_code) const noexcept;

    public:
        time_timer() noexcept;
        ~time_timer() noexcept;
        void    start(long duration_ms) noexcept;
        long    update() noexcept;
        long    add_time(long amount_ms) noexcept;
        long    remove_time(long amount_ms) noexcept;
        void    sleep_remaining() noexcept;
        int     get_error() const noexcept;
        const char  *get_error_str() const noexcept;
        pt_mutex    *get_mutex_for_validation() const noexcept;
        ft_operation_error_stack *operation_error_stack_handle() const noexcept;
};

#endif
