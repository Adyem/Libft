#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <atomic>
#include <utility>
#include <new>
#include <cstdlib>
#include "move.hpp"

template <typename ValueType>
class ft_promise
{
    private:
        ValueType _value;
        std::atomic<bool> _ready;
        mutable pt_recursive_mutex *_mutex;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    protected:
    public:
        ft_promise();
        ~ft_promise();

        ft_promise(const ft_promise&) = delete;
        ft_promise& operator=(const ft_promise&) = delete;
        ft_promise(ft_promise&&) = delete;
        ft_promise& operator=(ft_promise&&) = delete;

        void set_value(const ValueType& value);
        void set_value(ValueType&& value);
        ValueType get() const;
        bool is_ready() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
    #endif
};

template <>
class ft_promise<void>
{
    private:
        std::atomic<bool> _ready;
        mutable pt_recursive_mutex *_mutex;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    public:
        ft_promise();
        ~ft_promise();

        ft_promise(const ft_promise&) = delete;
        ft_promise& operator=(const ft_promise&) = delete;
        ft_promise(ft_promise&&) = delete;
        ft_promise& operator=(ft_promise&&) = delete;

        void set_value();
        void get() const;
        bool is_ready() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
    #endif
};

template <typename ValueType>
ft_promise<ValueType>::ft_promise()
    : _value(), _ready(false), _mutex(ft_nullptr)
{
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::~ft_promise()
{
    this->disable_thread_safety();
    return ;
}

inline ft_promise<void>::ft_promise()
    : _ready(false), _mutex(ft_nullptr)
{
    return ;
}

inline ft_promise<void>::~ft_promise()
{
    this->disable_thread_safety();
    return ;
}

template <typename ValueType>
int ft_promise<ValueType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

inline int ft_promise<void>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ValueType>
void ft_promise<ValueType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

inline void ft_promise<void>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}


template <typename ValueType>
int ft_promise<ValueType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

inline int ft_promise<void>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
void ft_promise<ValueType>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_promise<void>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
int ft_promise<ValueType>::lock_internal(bool *lock_acquired) const
{
    int mutex_result;
    int stack_error;
    int operation_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    stack_error = ft_global_error_stack_drop_last_error();
    operation_error = stack_error;
    if (stack_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(stack_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (operation_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

inline int ft_promise<void>::lock_internal(bool *lock_acquired) const
{
    int mutex_result;
    int stack_error;
    int operation_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    stack_error = ft_global_error_stack_drop_last_error();
    operation_error = stack_error;
    if (stack_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(stack_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (operation_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename ValueType>
int ft_promise<ValueType>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;
    int stack_error;
    int operation_error;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_result = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    stack_error = ft_global_error_stack_drop_last_error();
    operation_error = stack_error;
    if (stack_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(stack_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (operation_error);
    return (FT_ERR_SUCCESSS);
}

inline int ft_promise<void>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;
    int stack_error;
    int operation_error;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    mutex_result = pt_recursive_mutex_unlock_with_error(*this->_mutex);
    stack_error = ft_global_error_stack_drop_last_error();
    operation_error = stack_error;
    if (stack_error == FT_ERR_SUCCESSS)
        operation_error = mutex_result;
    else
        ft_global_error_stack_push(stack_error);
    if (operation_error != FT_ERR_SUCCESSS)
        return (operation_error);
    return (FT_ERR_SUCCESSS);
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(const ValueType& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_value = value;
    this->_ready.store(true, std::memory_order_release);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_promise<void>::set_value()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_ready.store(true, std::memory_order_release);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(ValueType&& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_value = ft_move(value);
    this->_ready.store(true, std::memory_order_release);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    bool lock_acquired;
    ValueType value_copy;

    if (!this->_ready.load(std::memory_order_acquire))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ValueType());
    }
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ValueType());
    value_copy = this->_value;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    bool lock_acquired;

    if (!this->_ready.load(std::memory_order_acquire))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_ready() const
{
    bool ready;

    ready = this->_ready.load(std::memory_order_acquire);
    if (!ready)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

inline bool ft_promise<void>::is_ready() const
{
    if (!this->_ready.load(std::memory_order_acquire))
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

template <typename ValueType>
int ft_promise<ValueType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

inline int ft_promise<void>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ValueType>
void ft_promise<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_promise<void>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

inline bool ft_promise<void>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

#ifdef LIBFT_TEST_BUILD
template <typename ValueType>
pt_recursive_mutex *ft_promise<ValueType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}

inline pt_recursive_mutex *ft_promise<void>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
