#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
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
        mutable int _error_code;
        mutable pt_mutex *_mutex;
        bool _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    protected:
        void set_error(int error) const;

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
        int get_error() const;
        const char* get_error_str() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <>
class ft_promise<void>
{
    private:
        std::atomic<bool> _ready;
        mutable int _error_code;
        mutable pt_mutex *_mutex;
        bool _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

        void set_error(int error) const;

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
        int get_error() const;
        const char *get_error_str() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename ValueType>
ft_promise<ValueType>::ft_promise()
    : _value(), _ready(false), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    int entry_errno;

    entry_errno = ft_errno;
    if (this->prepare_thread_safety() != 0)
        return ;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::~ft_promise()
{
    int entry_errno;

    entry_errno = ft_errno;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->teardown_thread_safety();
    ft_errno = entry_errno;
    return ;
}

inline ft_promise<void>::ft_promise()
    : _ready(false), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    int entry_errno;

    entry_errno = ft_errno;
    if (this->prepare_thread_safety() != 0)
        return ;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    ft_errno = entry_errno;
    return ;
}

inline ft_promise<void>::~ft_promise()
{
    int entry_errno;

    entry_errno = ft_errno;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->teardown_thread_safety();
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error);
    this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_error_unlocked(int error) const
{
    ft_promise<ValueType> *mutable_promise;

    mutable_promise = const_cast<ft_promise<ValueType> *>(this);
    mutable_promise->_error_code = error;
    ft_errno = error;
    return ;
}

inline void ft_promise<void>::set_error_unlocked(int error) const
{
    ft_promise<void> *mutable_promise;

    mutable_promise = const_cast<ft_promise<void> *>(this);
    mutable_promise->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ValueType>
int ft_promise<ValueType>::prepare_thread_safety()
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return (0);
    }
    memory_pointer = std::malloc(sizeof(pt_mutex));
    if (memory_pointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory_pointer);
        this->set_error_unlocked(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

inline int ft_promise<void>::prepare_thread_safety()
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return (0);
    }
    memory_pointer = std::malloc(sizeof(pt_mutex));
    if (memory_pointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory_pointer);
        this->set_error_unlocked(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ValueType>
void ft_promise<ValueType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_promise<void>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
int ft_promise<ValueType>::lock_internal(bool *lock_acquired) const
{
    ft_promise<ValueType> *mutable_promise;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    mutable_promise = const_cast<ft_promise<ValueType> *>(this);
    mutable_promise->_mutex->lock(THREAD_ID);
    if (mutable_promise->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        mutable_promise->set_error_unlocked(mutable_promise->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

inline int ft_promise<void>::lock_internal(bool *lock_acquired) const
{
    ft_promise<void> *mutable_promise;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    mutable_promise = const_cast<ft_promise<void> *>(this);
    mutable_promise->_mutex->lock(THREAD_ID);
    if (mutable_promise->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        mutable_promise->set_error_unlocked(mutable_promise->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template <typename ValueType>
void ft_promise<ValueType>::unlock_internal(bool lock_acquired) const
{
    ft_promise<ValueType> *mutable_promise;

    if (!lock_acquired)
        return ;
    mutable_promise = const_cast<ft_promise<ValueType> *>(this);
    if (mutable_promise->_mutex == ft_nullptr)
        return ;
    ft_errno = FT_ERR_SUCCESSS;
    mutable_promise->_mutex->unlock(THREAD_ID);
    if (mutable_promise->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        mutable_promise->set_error_unlocked(mutable_promise->_mutex->get_error());
        return ;
    }
    return ;
}

inline void ft_promise<void>::unlock_internal(bool lock_acquired) const
{
    ft_promise<void> *mutable_promise;

    if (!lock_acquired)
        return ;
    mutable_promise = const_cast<ft_promise<void> *>(this);
    if (mutable_promise->_mutex == ft_nullptr)
        return ;
    ft_errno = FT_ERR_SUCCESSS;
    mutable_promise->_mutex->unlock(THREAD_ID);
    if (mutable_promise->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        mutable_promise->set_error_unlocked(mutable_promise->_mutex->get_error());
        return ;
    }
    return ;
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
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::set_value()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->_ready.store(true, std::memory_order_release);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
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
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    bool lock_acquired;
    ValueType value_copy;

    if (!this->_ready.load(std::memory_order_acquire))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ValueType());
    }
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ValueType());
    value_copy = this->_value;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    bool lock_acquired;

    if (!this->_ready.load(std::memory_order_acquire))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_ready() const
{
    bool ready;

    ready = this->_ready.load(std::memory_order_acquire);
    if (!ready)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

inline bool ft_promise<void>::is_ready() const
{
    if (!this->_ready.load(std::memory_order_acquire))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

template <typename ValueType>
int ft_promise<ValueType>::get_error() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (this->_error_code);
    error_code = this->_error_code;
    ft_errno = error_code;
    this->unlock_internal(lock_acquired);
    return (error_code);
}

inline int ft_promise<void>::get_error() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (this->_error_code);
    error_code = this->_error_code;
    ft_errno = error_code;
    this->unlock_internal(lock_acquired);
    return (error_code);
}

template <typename ValueType>
const char* ft_promise<ValueType>::get_error_str() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_strerror(this->_error_code));
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    return (ft_strerror(error_code));
}

inline const char *ft_promise<void>::get_error_str() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_strerror(this->_error_code));
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    return (ft_strerror(error_code));
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
    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
        return (true);
    return (false);
}

inline bool ft_promise<void>::is_thread_safe() const
{
    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
        return (true);
    return (false);
}

template <typename ValueType>
int ft_promise<ValueType>::lock(bool *lock_acquired) const
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_promise<ValueType> *>(this)->set_error_unlocked(ft_errno);
        return (result);
    }
    const_cast<ft_promise<ValueType> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

inline int ft_promise<void>::lock(bool *lock_acquired) const
{
    int result;

    ft_errno = FT_ERR_SUCCESSS;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_promise<void> *>(this)->set_error_unlocked(ft_errno);
        return (result);
    }
    const_cast<ft_promise<void> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
void ft_promise<ValueType>::unlock(bool lock_acquired) const
{
    int mutex_error;

    ft_errno = FT_ERR_SUCCESSS;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_mutex == ft_nullptr)
    {
        const_cast<ft_promise<ValueType> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    mutex_error = this->_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_promise<ValueType> *>(this)->set_error_unlocked(mutex_error);
        return ;
    }
    const_cast<ft_promise<ValueType> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_promise<void>::unlock(bool lock_acquired) const
{
    int mutex_error;

    ft_errno = FT_ERR_SUCCESSS;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_mutex == ft_nullptr)
    {
        const_cast<ft_promise<void> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
        return ;
    }
    mutex_error = this->_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_promise<void> *>(this)->set_error_unlocked(mutex_error);
        return ;
    }
    const_cast<ft_promise<void> *>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

#endif
