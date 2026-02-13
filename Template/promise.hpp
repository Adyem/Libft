#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
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
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

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

inline int ft_promise<void>::prepare_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

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

template <typename ValueType>
void ft_promise<ValueType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        (void)this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

inline void ft_promise<void>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        (void)this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}


template <typename ValueType>
int ft_promise<ValueType>::lock(bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

inline int ft_promise<void>::lock(bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

template <typename ValueType>
void ft_promise<ValueType>::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
int ft_promise<ValueType>::lock_internal(bool *lock_acquired) const
{
    int mutex_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->lock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

inline int ft_promise<void>::lock_internal(bool *lock_acquired) const
{
    int mutex_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->lock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int ft_promise<ValueType>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->unlock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    return (FT_ERR_SUCCESS);
}

inline int ft_promise<void>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->unlock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    return (FT_ERR_SUCCESS);
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
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::set_value()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_ready.store(true, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
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
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    bool lock_acquired;
    ValueType value_copy;

    if (!this->_ready.load(std::memory_order_acquire))
        return (ValueType());
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ValueType());
    value_copy = this->_value;
    (void)this->unlock_internal(lock_acquired);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    bool lock_acquired;

    if (!this->_ready.load(std::memory_order_acquire))
        return ;
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_ready() const
{
    bool ready;

    ready = this->_ready.load(std::memory_order_acquire);
    return (ready);
}

inline bool ft_promise<void>::is_ready() const
{
    return (this->_ready.load(std::memory_order_acquire));
}

template <typename ValueType>
int ft_promise<ValueType>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

inline int ft_promise<void>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename ValueType>
void ft_promise<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

inline void ft_promise<void>::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

inline bool ft_promise<void>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
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
