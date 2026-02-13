#ifndef FT_CANCELLATION_HPP
#define FT_CANCELLATION_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "function.hpp"
#include "shared_ptr.hpp"
#include "vector.hpp"
#include "move.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <atomic>
#include <new>

class ft_cancellation_state
{
    private:
        std::atomic<bool> _cancelled;
        ft_vector<ft_function<void()> > _callbacks;
        mutable pt_recursive_mutex* _mutex;

        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        ft_cancellation_state() noexcept;
        ~ft_cancellation_state() noexcept;

        ft_cancellation_state(const ft_cancellation_state&) = delete;
        ft_cancellation_state &operator=(const ft_cancellation_state&) = delete;

        int register_callback(const ft_function<void()> &callback) noexcept;
        int request_cancel() noexcept;
        bool is_cancelled() const noexcept;
};

class ft_cancellation_source;

class ft_cancellation_token
{
    private:
        ft_sharedptr<ft_cancellation_state> _state;

        friend class ft_cancellation_source;

    public:
        ft_cancellation_token() noexcept;
        ~ft_cancellation_token() noexcept;

        ft_cancellation_token(const ft_cancellation_token &other) noexcept;
        ft_cancellation_token &operator=(const ft_cancellation_token &other) noexcept;
        ft_cancellation_token(ft_cancellation_token &&other) noexcept;
        ft_cancellation_token &operator=(ft_cancellation_token &&other) noexcept;

        bool is_valid() const noexcept;
        bool is_cancellation_requested() const noexcept;
        int  register_callback(const ft_function<void()> &callback) const noexcept;
};

class ft_cancellation_source
{
    private:
        ft_sharedptr<ft_cancellation_state> _state;

    public:
        ft_cancellation_source() noexcept;
        ~ft_cancellation_source() noexcept;

        ft_cancellation_source(const ft_cancellation_source &other) noexcept;
        ft_cancellation_source &operator=(const ft_cancellation_source &other) noexcept;
        ft_cancellation_source(ft_cancellation_source &&other) noexcept;
        ft_cancellation_source &operator=(ft_cancellation_source &&other) noexcept;

        ft_cancellation_token get_token() const noexcept;
        void request_cancel() noexcept;
        bool is_cancellation_requested() const noexcept;
};

inline int ft_cancellation_state::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int mutex_result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    ft_global_error_stack_push(mutex_result);
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

inline int ft_cancellation_state::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int mutex_result = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    ft_global_error_stack_push(mutex_result);
    return (mutex_result);
}

inline int ft_cancellation_state::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int creation_result = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (creation_result != FT_ERR_SUCCESS)
        return (creation_result);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

inline void ft_cancellation_state::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

inline ft_cancellation_state::ft_cancellation_state() noexcept
    : _cancelled(false), _callbacks(), _mutex(ft_nullptr)
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return ;
}

inline ft_cancellation_state::~ft_cancellation_state() noexcept
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline int ft_cancellation_state::register_callback(const ft_function<void()> &callback) noexcept
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    bool already_cancelled = this->_cancelled.load(std::memory_order_acquire);
    if (!already_cancelled)
    {
        this->_callbacks.push_back(callback);
        int push_error = ft_global_error_stack_drop_last_error();
        if (push_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(push_error);
            return (push_error);
        }
    }
    int unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    if (already_cancelled)
        callback();
    return (FT_ERR_SUCCESS);
}

inline int ft_cancellation_state::request_cancel() noexcept
{
    if (this->_cancelled.exchange(true, std::memory_order_acq_rel))
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (lock_error);
    }
    ft_vector<ft_function<void()> > callbacks_to_run;
    size_t callback_index = 0;
    while (callback_index < this->_callbacks.size())
    {
        callbacks_to_run.push_back(this->_callbacks[callback_index]);
        int push_error = ft_global_error_stack_drop_last_error();
        if (push_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(push_error);
            return (push_error);
        }
        ++callback_index;
    }
    this->_callbacks.clear();
    int clear_error = ft_global_error_stack_drop_last_error();
    if (clear_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(clear_error);
        return (clear_error);
    }
    int unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (unlock_error);
    }
    size_t invoke_index = 0;
    while (invoke_index < callbacks_to_run.size())
    {
        callbacks_to_run[invoke_index]();
        ++invoke_index;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

inline bool ft_cancellation_state::is_cancelled() const noexcept
{
    bool result = this->_cancelled.load(std::memory_order_acquire);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

inline ft_cancellation_token::ft_cancellation_token() noexcept
    : _state()
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_token::~ft_cancellation_token() noexcept
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_token::ft_cancellation_token(const ft_cancellation_token &other) noexcept
    : _state(other._state)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_token &ft_cancellation_token::operator=(const ft_cancellation_token &other) noexcept
{
    if (this != &other)
        this->_state = other._state;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

inline ft_cancellation_token::ft_cancellation_token(ft_cancellation_token &&other) noexcept
    : _state(ft_move(other._state))
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_token &ft_cancellation_token::operator=(ft_cancellation_token &&other) noexcept
{
    if (this != &other)
        this->_state = ft_move(other._state);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

inline bool ft_cancellation_token::is_valid() const noexcept
{
    bool result = static_cast<bool>(this->_state);
    if (result)
        ft_global_error_stack_push(FT_ERR_SUCCESS);
    else
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
    return (result);
}

inline bool ft_cancellation_token::is_cancellation_requested() const noexcept
{
    if (!static_cast<bool>(this->_state))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (true);
    }
    const ft_cancellation_state *state_pointer = this->_state.get();
    if (!state_pointer)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (true);
    }
    return (state_pointer->is_cancelled());
}

inline int ft_cancellation_token::register_callback(const ft_function<void()> &callback) const noexcept
{
    if (!this->is_valid())
    {
        return (FT_ERR_INVALID_STATE);
    }
    const ft_cancellation_state *state_pointer = this->_state.get();
    if (!state_pointer)
    {
        return (FT_ERR_INVALID_STATE);
    }
    return (const_cast<ft_cancellation_state *>(state_pointer)->register_callback(callback));
}

inline ft_cancellation_source::ft_cancellation_source() noexcept
    : _state()
{
    ft_cancellation_state *state_pointer;

    state_pointer = new (std::nothrow) ft_cancellation_state();
    if (!state_pointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    int construction_error = ft_global_error_stack_drop_last_error();
    if (construction_error != FT_ERR_SUCCESS)
    {
        delete state_pointer;
        ft_global_error_stack_push(construction_error);
        return ;
    }
    this->_state.reset(state_pointer, 1, false);
    int reset_error = ft_global_error_stack_drop_last_error();
    if (reset_error != FT_ERR_SUCCESS)
    {
        delete state_pointer;
        ft_global_error_stack_push(reset_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_source::~ft_cancellation_source() noexcept
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_source::ft_cancellation_source(const ft_cancellation_source &other) noexcept
    : _state(other._state)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_source &ft_cancellation_source::operator=(const ft_cancellation_source &other) noexcept
{
    if (this != &other)
        this->_state = other._state;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

inline ft_cancellation_source::ft_cancellation_source(ft_cancellation_source &&other) noexcept
    : _state(ft_move(other._state))
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

inline ft_cancellation_source &ft_cancellation_source::operator=(ft_cancellation_source &&other) noexcept
{
    if (this != &other)
        this->_state = ft_move(other._state);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

inline ft_cancellation_token ft_cancellation_source::get_token() const noexcept
{
    ft_cancellation_token token;

    if (!static_cast<bool>(this->_state))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (token);
    }
    token._state = this->_state;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (token);
}

inline void ft_cancellation_source::request_cancel() noexcept
{
    if (!static_cast<bool>(this->_state))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    ft_cancellation_state *state_pointer = this->_state.get();
    if (!state_pointer)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    state_pointer->request_cancel();
    int state_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(state_error);
    return ;
}

inline bool ft_cancellation_source::is_cancellation_requested() const noexcept
{
    if (!static_cast<bool>(this->_state))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (true);
    }
    const ft_cancellation_state *state_pointer = this->_state.get();
    if (!state_pointer)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (true);
    }
    bool cancelled = state_pointer->is_cancelled();
    int state_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_push(state_error);
    return (cancelled);
}

#endif
