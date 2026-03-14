#ifndef FT_CANCELLATION_HPP
#define FT_CANCELLATION_HPP

#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "class_nullptr.hpp"
#include "../Template/function.hpp"
#include "../Template/vector.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <atomic>
#include <new>

class ft_cancellation_state
{
    private:
        std::atomic<ft_bool>             _cancelled;
        ft_vector<ft_function<void()> >  _callbacks;
        mutable pt_recursive_mutex       *_mutex;
        uint8_t                          _initialised_state;
        static thread_local uint32_t     _last_error;

        static uint32_t set_error(uint32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    public:
        ft_cancellation_state() noexcept;
        ft_cancellation_state(const ft_cancellation_state &other) noexcept;
        ft_cancellation_state(ft_cancellation_state &&other) noexcept;
        ~ft_cancellation_state() noexcept;

        ft_cancellation_state &operator=(const ft_cancellation_state&) = delete;
        ft_cancellation_state &operator=(ft_cancellation_state&&) = delete;

        int32_t initialize() noexcept;
        int32_t destroy() noexcept;
        uint32_t move(ft_cancellation_state &other) noexcept;
        int32_t register_callback(const ft_function<void()> &callback) noexcept;
        int32_t request_cancel() noexcept;
        ft_bool is_cancelled() const noexcept;

        int32_t enable_thread_safety() noexcept;
        int32_t disable_thread_safety() noexcept;

        uint32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class ft_cancellation_source;

class ft_cancellation_token
{
    private:
        ft_cancellation_state *_state;

        friend class ft_cancellation_source;

        explicit ft_cancellation_token(ft_cancellation_state *state_pointer) noexcept;

    public:
        ft_cancellation_token() noexcept;
        ft_cancellation_token(const ft_cancellation_token &other) noexcept;
        ft_cancellation_token(ft_cancellation_token &&other) noexcept;
        ~ft_cancellation_token() noexcept;

        ft_cancellation_token &operator=(const ft_cancellation_token&) = delete;
        ft_cancellation_token &operator=(ft_cancellation_token&&) = delete;

        ft_bool is_valid() const noexcept;
        ft_bool is_cancellation_requested() const noexcept;
        int32_t register_callback(const ft_function<void()> &callback) const noexcept;
};

class ft_cancellation_source
{
    private:
        ft_cancellation_state *_state;
        uint8_t               _initialised_state;

    public:
        ft_cancellation_source() noexcept;
        ft_cancellation_source(const ft_cancellation_source &other) noexcept;
        ft_cancellation_source(ft_cancellation_source &&other) noexcept;
        ~ft_cancellation_source() noexcept;

        ft_cancellation_source &operator=(const ft_cancellation_source&) = delete;
        ft_cancellation_source &operator=(ft_cancellation_source&&) = delete;

        int32_t initialize() noexcept;
        int32_t destroy() noexcept;
        uint32_t move(ft_cancellation_source &other) noexcept;
        ft_cancellation_token get_token() const noexcept;
        int32_t request_cancel() noexcept;
        ft_bool is_cancellation_requested() const noexcept;
};

#endif
