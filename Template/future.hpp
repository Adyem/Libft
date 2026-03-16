#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include <chrono>
#include <cstdlib>

#include "move.hpp"
template <typename ValueType>
class ft_future
{
    private:
        ft_promise<ValueType>* _promise;
        ft_sharedptr<ft_promise<ValueType> > _shared_promise;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        ft_bool wait_ready() const;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<ValueType>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other) = delete;
        ft_future &operator=(ft_future &&other) = delete;
        int32_t initialize();
        int32_t destroy();
        int32_t initialize(const ft_future &other);
        int32_t initialize(ft_future &&other);
        int32_t move(ft_future &other);
        ValueType get() const;
        void wait() const;
        ft_bool valid() const;
        uint32_t get_error() const;
        const char *get_error_str() const;
        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;
        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;


};

template <>
class ft_future<void>
{
    private:
        ft_promise<void>* _promise;
        ft_sharedptr<ft_promise<void> > _shared_promise;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        ft_bool wait_ready() const;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<void>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<void> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other) = delete;
        ft_future &operator=(ft_future &&other) = delete;
        int32_t initialize();
        int32_t destroy();
        int32_t initialize(const ft_future &other);
        int32_t initialize(ft_future &&other);
        int32_t move(ft_future &other);
        void get() const;
        void wait() const;
        ft_bool valid() const;
        uint32_t get_error() const;
        const char *get_error_str() const;
        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;
        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;

};

template <typename ValueType>
int32_t ft_future<ValueType>::enable_thread_safety()
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::enable_thread_safety");
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
int32_t ft_future<ValueType>::disable_thread_safety()
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
ft_bool ft_future<ValueType>::is_thread_safe() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

template <typename ValueType>
uint32_t ft_future<ValueType>::get_error() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::get_error");
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
const char *ft_future<ValueType>::get_error_str() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::get_error_str");
    return (ft_strerror(this->get_error()));
}

template <typename ValueType>
int32_t ft_future<ValueType>::initialize()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_future::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_promise = ft_nullptr;
    this->_shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_future<ValueType>::destroy()
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_promise = ft_nullptr;
    this->_shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ft_future<ValueType>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize();
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_promise<ValueType>& promise)
    : _promise(&promise), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (this->initialize() == FT_ERR_SUCCESS)
        this->_promise = &promise;
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(const ft_future<ValueType> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(other);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_future<ValueType> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(ft_move(other));
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer)
    : _promise(ft_nullptr), _shared_promise(),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (this->initialize() == FT_ERR_SUCCESS)
    {
        this->_shared_promise = promise_pointer;
        this->_promise = promise_pointer.get();
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    (void)this->destroy();
    return ;
}

template <typename ValueType>
int32_t ft_future<ValueType>::initialize(const ft_future<ValueType> &other)
{
    ft_bool this_lock_acquired;
    ft_bool other_lock_acquired;
    const ft_future<ValueType> *first_lock_target;
    const ft_future<ValueType> *second_lock_target;
    int32_t first_lock_result;
    int32_t second_lock_result;
    ft_promise<ValueType> *other_promise;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_future::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_promise = ft_nullptr;
        this->_shared_promise = ft_sharedptr<ft_promise<ValueType> >();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = FT_FALSE;
    other_lock_acquired = FT_FALSE;
    if (this < &other)
    {
        first_lock_target = this;
        second_lock_target = &other;
    }
    else
    {
        first_lock_target = &other;
        second_lock_target = this;
    }
    if (first_lock_target == this)
        first_lock_result = this->lock_internal(&this_lock_acquired);
    else
        first_lock_result = other.lock_internal(&other_lock_acquired);
    if (first_lock_result != FT_ERR_SUCCESS)
    {
        return (first_lock_result);
    }
    if (second_lock_target == this)
        second_lock_result = this->lock_internal(&this_lock_acquired);
    else
        second_lock_result = other.lock_internal(&other_lock_acquired);
    if (second_lock_result != FT_ERR_SUCCESS)
    {
        if (first_lock_target == this)
            this->unlock_internal(this_lock_acquired);
        else
            other.unlock_internal(other_lock_acquired);
        return (second_lock_result);
    }
    other_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->_promise = other_promise;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (second_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    if (first_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_future<ValueType>::initialize(ft_future<ValueType> &&other)
{
    ft_bool this_lock_acquired;
    ft_bool other_lock_acquired;
    ft_future<ValueType> *first_lock_target;
    ft_future<ValueType> *second_lock_target;
    int32_t first_lock_result;
    int32_t second_lock_result;
    ft_promise<ValueType> *transferred_promise;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_future::initialize(move)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_promise = ft_nullptr;
        this->_shared_promise = ft_sharedptr<ft_promise<ValueType> >();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = FT_FALSE;
    other_lock_acquired = FT_FALSE;
    if (this < &other)
    {
        first_lock_target = this;
        second_lock_target = &other;
    }
    else
    {
        first_lock_target = &other;
        second_lock_target = this;
    }
    if (first_lock_target == this)
        first_lock_result = this->lock_internal(&this_lock_acquired);
    else
        first_lock_result = other.lock_internal(&other_lock_acquired);
    if (first_lock_result != FT_ERR_SUCCESS)
    {
        return (first_lock_result);
    }
    if (second_lock_target == this)
        second_lock_result = this->lock_internal(&this_lock_acquired);
    else
        second_lock_result = other.lock_internal(&other_lock_acquired);
    if (second_lock_result != FT_ERR_SUCCESS)
    {
        if (first_lock_target == this)
            this->unlock_internal(this_lock_acquired);
        else
            other.unlock_internal(other_lock_acquired);
        return (second_lock_result);
    }
    transferred_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->_promise = transferred_promise;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    if (second_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    if (first_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t ft_future<ValueType>::move(ft_future<ValueType> &other)
{
    return (this->initialize(ft_move(other)));
}

template <typename ValueType>
ft_bool ft_future<ValueType>::wait_ready() const
{
    ft_bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;

    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return (FT_FALSE);
    }
    const auto start = std::chrono::steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1))
        {
            return (FT_FALSE);
        }
        pt_thread_yield();
    }
    return (FT_TRUE);
}

template <typename ValueType>
ValueType ft_future<ValueType>::get() const
{
    ft_bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;
    ValueType value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::get");
    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (ValueType());
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return (ValueType());
    }
    if (!this->wait_ready())
    {
        return (ValueType());
    }
    value = promise_pointer->get();
    return (value);
}

template <typename ValueType>
void ft_future<ValueType>::wait() const
{
    ft_bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::wait");
    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return ;
    }
    (void)this->wait_ready();
    return ;
}

template <typename ValueType>
ft_bool ft_future<ValueType>::valid() const
{
    ft_bool is_valid;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::valid");
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    is_valid = (this->_promise != ft_nullptr);
    this->unlock_internal(lock_acquired);
    return (is_valid);
}

inline int32_t ft_future<void>::enable_thread_safety()
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::enable_thread_safety");
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

inline int32_t ft_future<void>::disable_thread_safety()
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

inline ft_bool ft_future<void>::is_thread_safe() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

inline uint32_t ft_future<void>::get_error() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::get_error");
    return (FT_ERR_SUCCESS);
}

inline const char *ft_future<void>::get_error_str() const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::get_error_str");
    return (ft_strerror(this->get_error()));
}

inline int32_t ft_future<void>::initialize()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_future<void>::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_promise = ft_nullptr;
    this->_shared_promise = ft_sharedptr<ft_promise<void> >();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_future<void>::destroy()
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_promise = ft_nullptr;
    this->_shared_promise = ft_sharedptr<ft_promise<void> >();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    return (FT_ERR_SUCCESS);
}

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize();
    return ;
}


inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (this->initialize() == FT_ERR_SUCCESS)
        this->_promise = &promise;
    return ;
}

inline ft_future<void>::ft_future(const ft_future<void> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(other);
    return ;
}

inline ft_future<void>::ft_future(ft_future<void> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(ft_move(other));
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(ft_nullptr), _shared_promise(),
      _mutex(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (this->initialize() == FT_ERR_SUCCESS)
    {
        this->_shared_promise = promise_pointer;
        this->_promise = promise_pointer.get();
    }
    return ;
}

inline ft_future<void>::~ft_future()
{
    (void)this->destroy();
    return ;
}

inline int32_t ft_future<void>::initialize(const ft_future<void> &other)
{
    ft_bool this_lock_acquired;
    ft_bool other_lock_acquired;
    const ft_future<void> *first_lock_target;
    const ft_future<void> *second_lock_target;
    int32_t first_lock_result;
    int32_t second_lock_result;
    ft_promise<void> *other_promise;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_future<void>::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_promise = ft_nullptr;
        this->_shared_promise = ft_sharedptr<ft_promise<void> >();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = FT_FALSE;
    other_lock_acquired = FT_FALSE;
    if (this < &other)
    {
        first_lock_target = this;
        second_lock_target = &other;
    }
    else
    {
        first_lock_target = &other;
        second_lock_target = this;
    }
    if (first_lock_target == this)
        first_lock_result = this->lock_internal(&this_lock_acquired);
    else
        first_lock_result = other.lock_internal(&other_lock_acquired);
    if (first_lock_result != FT_ERR_SUCCESS)
    {
        return (first_lock_result);
    }
    if (second_lock_target == this)
        second_lock_result = this->lock_internal(&this_lock_acquired);
    else
        second_lock_result = other.lock_internal(&other_lock_acquired);
    if (second_lock_result != FT_ERR_SUCCESS)
    {
        if (first_lock_target == this)
            this->unlock_internal(this_lock_acquired);
        else
            other.unlock_internal(other_lock_acquired);
        return (second_lock_result);
    }
    other_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->_promise = other_promise;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (second_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    if (first_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_future<void>::initialize(ft_future<void> &&other)
{
    ft_bool this_lock_acquired;
    ft_bool other_lock_acquired;
    ft_future<void> *first_lock_target;
    ft_future<void> *second_lock_target;
    int32_t first_lock_result;
    int32_t second_lock_result;
    ft_promise<void> *transferred_promise;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_future<void>::initialize(move)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        if (this->destroy() != FT_ERR_SUCCESS)
            return (FT_ERR_INVALID_STATE);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_promise = ft_nullptr;
        this->_shared_promise = ft_sharedptr<ft_promise<void> >();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = FT_FALSE;
    other_lock_acquired = FT_FALSE;
    if (this < &other)
    {
        first_lock_target = this;
        second_lock_target = &other;
    }
    else
    {
        first_lock_target = &other;
        second_lock_target = this;
    }
    if (first_lock_target == this)
        first_lock_result = this->lock_internal(&this_lock_acquired);
    else
        first_lock_result = other.lock_internal(&other_lock_acquired);
    if (first_lock_result != FT_ERR_SUCCESS)
    {
        return (first_lock_result);
    }
    if (second_lock_target == this)
        second_lock_result = this->lock_internal(&this_lock_acquired);
    else
        second_lock_result = other.lock_internal(&other_lock_acquired);
    if (second_lock_result != FT_ERR_SUCCESS)
    {
        if (first_lock_target == this)
            this->unlock_internal(this_lock_acquired);
        else
            other.unlock_internal(other_lock_acquired);
        return (second_lock_result);
    }
    transferred_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->_promise = transferred_promise;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<void> >();
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    if (second_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    if (first_lock_target == this)
        this->unlock_internal(this_lock_acquired);
    else
        other.unlock_internal(other_lock_acquired);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_future<void>::move(ft_future<void> &other)
{
    return (this->initialize(ft_move(other)));
}

inline ft_bool ft_future<void>::wait_ready() const
{
    ft_bool lock_acquired;
    ft_promise<void> *promise_pointer;

    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return (FT_FALSE);
    }
    const auto start = std::chrono::steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1))
        {
            return (FT_FALSE);
        }
        pt_thread_yield();
    }
    return (FT_TRUE);
}

inline void ft_future<void>::get() const
{
    ft_bool lock_acquired;
    ft_promise<void> *promise_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::get");
    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return ;
    }
    if (!this->wait_ready())
    {
        return ;
    }
    return ;
}

inline void ft_future<void>::wait() const
{
    ft_bool lock_acquired;
    ft_promise<void> *promise_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::wait");
    lock_acquired = FT_FALSE;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        return ;
    }
    (void)this->wait_ready();
    return ;
}

inline ft_bool ft_future<void>::valid() const
{
    ft_bool is_valid;
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::valid");
    lock_acquired = FT_FALSE;
    if (this->lock_internal(&lock_acquired) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    is_valid = (this->_promise != ft_nullptr);
    this->unlock_internal(lock_acquired);
    return (is_valid);
}

template <typename ValueType>
int32_t ft_future<ValueType>::lock(ft_bool *lock_acquired) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::lock");
    return (this->lock_internal(lock_acquired));
}

template <typename ValueType>
void ft_future<ValueType>::unlock(ft_bool lock_acquired) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
int32_t ft_future<ValueType>::lock_internal(ft_bool *lock_acquired) const
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
int32_t ft_future<ValueType>::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

inline int32_t ft_future<void>::lock(ft_bool *lock_acquired) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::lock");
    return (this->lock_internal(lock_acquired));
}

inline void ft_future<void>::unlock(ft_bool lock_acquired) const
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_future<void>::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline int32_t ft_future<void>::lock_internal(ft_bool *lock_acquired) const
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

inline int32_t ft_future<void>::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}


#endif
