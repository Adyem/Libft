#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <chrono>
#include <utility>

#include "move.hpp"
template <typename ValueType>
class ft_future
{
    private:
        ft_promise<ValueType>* _promise;
        ft_sharedptr<ft_promise<ValueType> > _shared_promise;
        mutable int _error_code;
        mutable pt_mutex* _state_mutex;
        bool _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        void set_error(int error) const;
        bool wait_ready() const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<ValueType>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other);
        ft_future &operator=(ft_future &&other);
        ValueType get() const;
        void wait() const;
        bool valid() const;
        int get_error() const;
        const char* get_error_str() const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <>
class ft_future<void>
{
    private:
        ft_promise<void>* _promise;
        ft_sharedptr<ft_promise<void> > _shared_promise;
        mutable int _error_code;
        mutable pt_mutex* _state_mutex;
        bool _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        void set_error(int error) const;
        bool wait_ready() const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<void>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<void> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other);
        ft_future &operator=(ft_future &&other);
        void get() const;
        void wait() const;
        bool valid() const;
        int get_error() const;
        const char* get_error_str() const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename ValueType>
void ft_future<ValueType>::set_error_unlocked(int error) const
{
    ft_future<ValueType> *mutable_future;

    mutable_future = const_cast<ft_future<ValueType> *>(this);
    mutable_future->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ValueType>
void ft_future<ValueType>::set_error(int error) const
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
ft_future<ValueType>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(const ft_future<ValueType> &other)
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;
    ft_promise<ValueType> *other_promise;
    ft_sharedptr<ft_promise<ValueType> > other_shared;
    int other_error;
    bool other_thread_safe;

    lock_acquired = false;
    other_promise = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other.unlock_internal(lock_acquired);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(other_error);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_future<ValueType> &&other)
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;
    ft_promise<ValueType> *transferred_promise;
    ft_sharedptr<ft_promise<ValueType> > transferred_shared;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    lock_acquired = false;
    transferred_promise = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    this->_error_code = transferred_error;
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    other._promise = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other.unlock_internal(lock_acquired);
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other._shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(transferred_error);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_promise<ValueType>& promise)
    : _promise(&promise), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer),
      _error_code(ER_SUCCESS), _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    this->teardown_thread_safety();
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType> &ft_future<ValueType>::operator=(const ft_future<ValueType> &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<ValueType> *other_promise;
    ft_sharedptr<ft_promise<ValueType> > other_shared;
    int other_error;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    this->_error_code = other_error;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    if (other_thread_safe)
    {
        if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
    }
    else if (this->_thread_safe_enabled)
        this->disable_thread_safety();
    this->set_error(other_error);
    return (*this);
}

template <typename ValueType>
ft_future<ValueType> &ft_future<ValueType>::operator=(ft_future<ValueType> &&other)
{
    int entry_errno;
    bool this_lock_acquired;
    bool other_lock_acquired;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;
    ft_promise<ValueType> *transferred_promise;
    ft_sharedptr<ft_promise<ValueType> > transferred_shared;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    entry_errno = ft_errno;
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    previous_mutex = this->_state_mutex;
    previous_thread_safe = this->_thread_safe_enabled;
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    this->_error_code = transferred_error;
    other._promise = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other.unlock_internal(other_lock_acquired);
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other._shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    this->unlock_internal(this_lock_acquired);
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    if (previous_thread_safe && previous_mutex != ft_nullptr)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            this->_error_code = ft_errno;
            ft_errno = entry_errno;
            return (*this);
        }
    }
    this->_error_code = transferred_error;
    ft_errno = entry_errno;
    return (*this);
}

template <typename ValueType>
bool ft_future<ValueType>::wait_ready() const
{
    bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ValueType>
ValueType ft_future<ValueType>::get() const
{
    bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;
    ValueType value;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ValueType());
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (ValueType());
    }
    if (!this->wait_ready())
    {
        return (ValueType());
    }
    value = promise_pointer->get();
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ValueType>
void ft_future<ValueType>::wait() const
{
    bool lock_acquired;
    ft_promise<ValueType> *promise_pointer;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    (void)this->wait_ready();
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::valid() const
{
    bool is_valid;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    is_valid = (this->_promise != ft_nullptr);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return (is_valid);
}

template <typename ValueType>
int ft_future<ValueType>::get_error() const
{
    int error;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_errno);
    error = this->_error_code;
    this->unlock_internal(lock_acquired);
    this->set_error(error);
    return (error);
}

template <typename ValueType>
const char* ft_future<ValueType>::get_error_str() const
{
    const char* error_string;
    int error_value;

    error_value = this->get_error();
    error_string = ft_strerror(error_value);
    this->set_error(error_value);
    return (error_string);
}

inline void ft_future<void>::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error);
    this->unlock_internal(lock_acquired);
    return ;
}

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::ft_future(const ft_future<void> &other)
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;
    ft_promise<void> *other_promise;
    ft_sharedptr<ft_promise<void> > other_shared;
    int other_error;
    bool other_thread_safe;

    lock_acquired = false;
    other_promise = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other.unlock_internal(lock_acquired);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(other_error);
    return ;
}

inline ft_future<void>::ft_future(ft_future<void> &&other)
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;
    ft_promise<void> *transferred_promise;
    ft_sharedptr<ft_promise<void> > transferred_shared;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    lock_acquired = false;
    transferred_promise = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    this->_error_code = transferred_error;
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    other._promise = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other.unlock_internal(lock_acquired);
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other._shared_promise = ft_sharedptr<ft_promise<void> >();
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(transferred_error);
    return ;
}

inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer),
      _error_code(ER_SUCCESS), _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::~ft_future()
{
    this->teardown_thread_safety();
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void> &ft_future<void>::operator=(const ft_future<void> &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<void> *other_promise;
    ft_sharedptr<ft_promise<void> > other_shared;
    int other_error;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    this->_error_code = other_error;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    if (other_thread_safe)
    {
        if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
    }
    else if (this->_thread_safe_enabled)
        this->disable_thread_safety();
    this->set_error(other_error);
    return (*this);
}

inline ft_future<void> &ft_future<void>::operator=(ft_future<void> &&other)
{
    int entry_errno;
    bool this_lock_acquired;
    bool other_lock_acquired;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;
    ft_promise<void> *transferred_promise;
    ft_sharedptr<ft_promise<void> > transferred_shared;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    entry_errno = ft_errno;
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    previous_mutex = this->_state_mutex;
    previous_thread_safe = this->_thread_safe_enabled;
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    this->_error_code = transferred_error;
    other._promise = ft_nullptr;
    other._error_code = ER_SUCCESS;
    other.unlock_internal(other_lock_acquired);
    other._state_mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other._shared_promise = ft_sharedptr<ft_promise<void> >();
    this->unlock_internal(this_lock_acquired);
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    if (previous_thread_safe && previous_mutex != ft_nullptr)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            this->_error_code = ft_errno;
            ft_errno = entry_errno;
            return (*this);
        }
    }
    this->_error_code = transferred_error;
    ft_errno = entry_errno;
    return (*this);
}

inline bool ft_future<void>::wait_ready() const
{
    bool lock_acquired;
    ft_promise<void> *promise_pointer;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

inline void ft_future<void>::get() const
{
    bool lock_acquired;
    ft_promise<void> *promise_pointer;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (!this->wait_ready())
    {
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_future<void>::wait() const
{
    bool lock_acquired;
    ft_promise<void> *promise_pointer;

    lock_acquired = false;
    promise_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    promise_pointer = this->_promise;
    this->unlock_internal(lock_acquired);
    if (promise_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    (void)this->wait_ready();
    return ;
}

inline bool ft_future<void>::valid() const
{
    bool is_valid;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    is_valid = (this->_promise != ft_nullptr);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return (is_valid);
}

inline int ft_future<void>::get_error() const
{
    int error;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (ft_errno);
    error = this->_error_code;
    this->unlock_internal(lock_acquired);
    this->set_error(error);
    return (error);
}

inline const char *ft_future<void>::get_error_str() const
{
    const char *error_string;
    int error_value;

    error_value = this->get_error();
    error_string = ft_strerror(error_value);
    this->set_error(error_value);
    return (error_string);
}

template <typename ValueType>
int ft_future<ValueType>::enable_thread_safety()
{
    void *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename ValueType>
void ft_future<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

template <typename ValueType>
int ft_future<ValueType>::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(ft_errno);
        return (result);
    }
    const_cast<ft_future<ValueType> *>(this)->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return (result);
}

template <typename ValueType>
void ft_future<ValueType>::unlock(bool lock_acquired) const
{
    int entry_errno;
    int mutex_error;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        const_cast<ft_future<ValueType> *>(this)->_error_code = ER_SUCCESS;
        ft_errno = entry_errno;
        return ;
    }
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(mutex_error);
        return ;
    }
    const_cast<ft_future<ValueType> *>(this)->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
int ft_future<ValueType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename ValueType>
void ft_future<ValueType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
void ft_future<ValueType>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

inline void ft_future<void>::set_error_unlocked(int error) const
{
    ft_future<void> *mutable_future;

    mutable_future = const_cast<ft_future<void> *>(this);
    mutable_future->_error_code = error;
    ft_errno = error;
    return ;
}

inline int ft_future<void>::enable_thread_safety()
{
    void *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

inline void ft_future<void>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

inline bool ft_future<void>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

inline int ft_future<void>::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_future<void> *>(this)->set_error(ft_errno);
        return (result);
    }
    const_cast<ft_future<void> *>(this)->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return (result);
}

inline void ft_future<void>::unlock(bool lock_acquired) const
{
    int entry_errno;
    int mutex_error;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        const_cast<ft_future<void> *>(this)->_error_code = ER_SUCCESS;
        ft_errno = entry_errno;
        return ;
    }
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(mutex_error);
        return ;
    }
    const_cast<ft_future<void> *>(this)->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return ;
}

inline int ft_future<void>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

inline void ft_future<void>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

inline void ft_future<void>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
