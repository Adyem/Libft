#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
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
        std::atomic<ft_bool> _ready;
        mutable pt_recursive_mutex *_mutex;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

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
        ft_bool is_ready() const;

        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;
        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;
    };

template <>
class ft_promise<void>
{
    private:
        std::atomic<ft_bool> _ready;
        mutable pt_recursive_mutex *_mutex;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    public:
        ft_promise();
        ~ft_promise();

        ft_promise(const ft_promise&) = delete;
        ft_promise& operator=(const ft_promise&) = delete;
        ft_promise(ft_promise&&) = delete;
        ft_promise& operator=(ft_promise&&) = delete;

        void set_value();
        void get() const;
        ft_bool is_ready() const;

        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;
        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;
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
int32_t ft_promise<ValueType>::lock(ft_bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

inline int32_t ft_promise<void>::lock(ft_bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

template <typename ValueType>
void ft_promise<ValueType>::unlock(ft_bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::unlock(ft_bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
int32_t ft_promise<ValueType>::lock_internal(ft_bool *lock_acquired) const
{
    int32_t mutex_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    mutex_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired != ft_nullptr && this->_mutex != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::lock_internal(ft_bool *lock_acquired) const
{
    int32_t mutex_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    mutex_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired != ft_nullptr && this->_mutex != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_promise<ValueType>::unlock_internal(ft_bool lock_acquired) const
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::unlock_internal(ft_bool lock_acquired) const
{
    if (!lock_acquired)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(const ValueType& value)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_value = value;
    this->_ready.store(true, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::set_value()
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_ready.store(true, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(ValueType&& value)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_value = ft_move(value);
    this->_ready.store(true, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    ft_bool lock_acquired;
    ValueType value_copy;

    if (!this->_ready.load(std::memory_order_acquire))
        return (ValueType());
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ValueType());
    value_copy = this->_value;
    (void)this->unlock_internal(lock_acquired);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    ft_bool lock_acquired;

    if (!this->_ready.load(std::memory_order_acquire))
        return ;
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ft_bool ft_promise<ValueType>::is_ready() const
{
    ft_bool ready;

    ready = this->_ready.load(std::memory_order_acquire);
    return (ready);
}

inline ft_bool ft_promise<void>::is_ready() const
{
    return (this->_ready.load(std::memory_order_acquire));
}

template <typename ValueType>
int32_t ft_promise<ValueType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int32_t mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    int32_t mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_promise<ValueType>::disable_thread_safety()
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = this->_mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::disable_thread_safety()
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = this->_mutex->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ft_bool ft_promise<ValueType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

inline ft_bool ft_promise<void>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}


#endif
