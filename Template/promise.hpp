#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
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
        uint8_t _initialised_state;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    protected:
    public:
        ft_promise();
        ft_promise(const ft_promise& other);
        ft_promise(ft_promise&& other);
        ~ft_promise();

        ft_promise& operator=(const ft_promise&) = delete;
        ft_promise& operator=(ft_promise&&) = delete;

        int32_t initialize();
        int32_t destroy();
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
        uint8_t _initialised_state;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    public:
        ft_promise();
        ft_promise(const ft_promise& other);
        ft_promise(ft_promise&& other);
        ~ft_promise();

        ft_promise& operator=(const ft_promise&) = delete;
        ft_promise& operator=(ft_promise&&) = delete;

        int32_t initialize();
        int32_t destroy();
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
    : _value(), _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize();
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::ft_promise(const ft_promise<ValueType>& other)
    : _value(), _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    ft_bool lock_acquired;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_promise::ft_promise(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_acquired = FT_FALSE;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_value = other._value;
    this->_ready.store(other._ready.load(std::memory_order_acquire),
        std::memory_order_release);
    (void)other.unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::ft_promise(ft_promise<ValueType>&& other)
    : _value(), _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    ft_bool lock_acquired;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_promise::ft_promise(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_acquired = FT_FALSE;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_value = ft_move(other._value);
    this->_ready.store(other._ready.load(std::memory_order_acquire),
        std::memory_order_release);
    other._ready.store(FT_FALSE, std::memory_order_release);
    (void)other.unlock_internal(lock_acquired);
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::~ft_promise()
{
    (void)this->destroy();
    return ;
}

inline ft_promise<void>::ft_promise()
    : _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize();
    return ;
}

inline ft_promise<void>::ft_promise(const ft_promise<void>& other)
    : _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    ft_bool lock_acquired;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_promise<void>::ft_promise(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_acquired = FT_FALSE;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_ready.store(other._ready.load(std::memory_order_acquire),
        std::memory_order_release);
    (void)other.unlock_internal(lock_acquired);
    return ;
}

inline ft_promise<void>::ft_promise(ft_promise<void>&& other)
    : _ready(FT_FALSE), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    ft_bool lock_acquired;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_promise<void>::ft_promise(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    lock_acquired = FT_FALSE;
    if (other.lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    this->_ready.store(other._ready.load(std::memory_order_acquire),
        std::memory_order_release);
    other._ready.store(FT_FALSE, std::memory_order_release);
    (void)other.unlock_internal(lock_acquired);
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

inline ft_promise<void>::~ft_promise()
{
    (void)this->destroy();
    return ;
}

template <typename ValueType>
int32_t ft_promise<ValueType>::initialize()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_promise::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_value = ValueType();
    this->_ready.store(FT_FALSE, std::memory_order_release);
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::initialize()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_promise<void>::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_ready.store(FT_FALSE, std::memory_order_release);
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_promise<ValueType>::destroy()
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_value = ValueType();
    this->_ready.store(FT_FALSE, std::memory_order_release);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::destroy()
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_ready.store(FT_FALSE, std::memory_order_release);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_promise<ValueType>::lock(ft_bool *lock_acquired) const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::lock");
    return (this->lock_internal(lock_acquired));
}

inline int32_t ft_promise<void>::lock(ft_bool *lock_acquired) const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::lock");
    return (this->lock_internal(lock_acquired));
}

template <typename ValueType>
void ft_promise<ValueType>::unlock(ft_bool lock_acquired) const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::unlock(ft_bool lock_acquired) const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::unlock");
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
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(const ValueType& value)
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::set_value(copy)");
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_value = value;
    this->_ready.store(FT_TRUE, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline void ft_promise<void>::set_value()
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::set_value");
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_ready.store(FT_TRUE, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(ValueType&& value)
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::set_value(move)");
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    this->_value = ft_move(value);
    this->_ready.store(FT_TRUE, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    ft_bool lock_acquired;
    ValueType value_copy;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::get");
    if (!this->_ready.load(std::memory_order_acquire))
        return (ValueType());
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (ValueType());
    value_copy = this->_value;
    (void)this->unlock_internal(lock_acquired);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::get");
    if (!this->_ready.load(std::memory_order_acquire))
        return ;
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
ft_bool ft_promise<ValueType>::is_ready() const
{
    ft_bool ready;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::is_ready");
    ready = this->_ready.load(std::memory_order_acquire);
    return (ready);
}

inline ft_bool ft_promise<void>::is_ready() const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::is_ready");
    return (this->_ready.load(std::memory_order_acquire));
}

template <typename ValueType>
int32_t ft_promise<ValueType>::enable_thread_safety()
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::enable_thread_safety");
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
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::enable_thread_safety");
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
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        && this->_initialised_state != FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_promise<void>::disable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        && this->_initialised_state != FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_INVALID_STATE);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ft_bool ft_promise<ValueType>::is_thread_safe() const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

inline ft_bool ft_promise<void>::is_thread_safe() const
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_promise<void>::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}


#endif
