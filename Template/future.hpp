#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
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
        bool wait_ready() const;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety() const;
        void teardown_thread_safety() const;

    public:
        ft_future();
        ft_future(const ft_future &other) = delete;
        ft_future(ft_future &&other) = delete;
        explicit ft_future(ft_promise<ValueType>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other) = delete;
        ft_future &operator=(ft_future &&other) = delete;
        int initialize(const ft_future &other);
        int initialize(ft_future &&other);
        int move(ft_future &other);
        ValueType get() const;
        void wait() const;
        bool valid() const;
        int last_error_code() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() noexcept;
#endif

};

template <>
class ft_future<void>
{
    private:
        ft_promise<void>* _promise;
        ft_sharedptr<ft_promise<void> > _shared_promise;
        mutable pt_recursive_mutex *_mutex;
        bool wait_ready() const;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety() const;
        void teardown_thread_safety() const;

    public:
        ft_future();
        ft_future(const ft_future &other) = delete;
        ft_future(ft_future &&other) = delete;
        explicit ft_future(ft_promise<void>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<void> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other) = delete;
        ft_future &operator=(ft_future &&other) = delete;
        int initialize(const ft_future &other);
        int initialize(ft_future &&other);
        int move(ft_future &other);
        void get() const;
        void wait() const;
        bool valid() const;
        int last_error_code() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() noexcept;
#endif
};

template <typename ValueType>
int ft_future<ValueType>::prepare_thread_safety() const
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
void ft_future<ValueType>::teardown_thread_safety() const
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
int ft_future<ValueType>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename ValueType>
void ft_future<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

template <typename ValueType>
int ft_future<ValueType>::last_error_code() const
{
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ft_future<ValueType>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_promise<ValueType>& promise)
    : _promise(&promise), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer)
    : _promise(ft_nullptr), _shared_promise(),
      _mutex(ft_nullptr)
{
    this->_promise = promise_pointer.get();
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    this->disable_thread_safety();
    this->_promise = ft_nullptr;
    return ;
}

template <typename ValueType>
int ft_future<ValueType>::initialize(const ft_future<ValueType> &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<ValueType> *other_promise;
    int other_error;

    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESS)
    {
        return (this_lock_result);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        return (other_lock_result);
    }
    other_promise = other._promise;
    other_error = other.last_error_code();
    this->_promise = other_promise;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (other_error);
}

template <typename ValueType>
int ft_future<ValueType>::initialize(ft_future<ValueType> &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<ValueType> *transferred_promise;
    int transferred_error;

    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESS)
    {
        return (this_lock_result);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        return (other_lock_result);
    }
    transferred_promise = other._promise;
    transferred_error = other.last_error_code();
    this->_promise = transferred_promise;
    other._promise = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (transferred_error);
}

template <typename ValueType>
int ft_future<ValueType>::move(ft_future<ValueType> &other)
{
    return (this->initialize(ft_move(other)));
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
        return (false);
    }
    const auto start = std::chrono::steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1))
        {
            return (false);
        }
        pt_thread_yield();
    }
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
    return (is_valid);
}

inline int ft_future<void>::prepare_thread_safety() const
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

inline void ft_future<void>::teardown_thread_safety() const
{
    if (this->_mutex != ft_nullptr)
    {
        (void)this->_mutex->destroy();
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return ;
}

inline int ft_future<void>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

inline void ft_future<void>::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

inline bool ft_future<void>::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

inline int ft_future<void>::last_error_code() const
{
    return (FT_ERR_SUCCESS);
}

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}


inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(ft_nullptr), _shared_promise(),
      _mutex(ft_nullptr)
{
    this->_promise = promise_pointer.get();
    return ;
}

inline ft_future<void>::~ft_future()
{
    this->disable_thread_safety();
    this->_promise = ft_nullptr;
    return ;
}

inline int ft_future<void>::initialize(const ft_future<void> &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<void> *other_promise;
    int other_error;

    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESS)
    {
        return (this_lock_result);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        return (other_lock_result);
    }
    other_promise = other._promise;
    other_error = other.last_error_code();
    this->_promise = other_promise;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (other_error);
}

inline int ft_future<void>::initialize(ft_future<void> &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<void> *transferred_promise;
    int transferred_error;

    if (this == &other)
    {
        return (FT_ERR_SUCCESS);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESS)
    {
        return (this_lock_result);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        return (other_lock_result);
    }
    transferred_promise = other._promise;
    transferred_error = other.last_error_code();
    this->_promise = transferred_promise;
    other._promise = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (transferred_error);
}

inline int ft_future<void>::move(ft_future<void> &other)
{
    return (this->initialize(ft_move(other)));
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
        return (false);
    }
    const auto start = std::chrono::steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (std::chrono::steady_clock::now() - start > std::chrono::seconds(1))
        {
            return (false);
        }
        pt_thread_yield();
    }
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
    return (is_valid);
}

template <typename ValueType>
int ft_future<ValueType>::lock(bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

template <typename ValueType>
void ft_future<ValueType>::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ValueType>
int ft_future<ValueType>::lock_internal(bool *lock_acquired) const
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
int ft_future<ValueType>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->unlock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    return (FT_ERR_SUCCESS);
}

inline int ft_future<void>::lock(bool *lock_acquired) const
{
    return (this->lock_internal(lock_acquired));
}

inline void ft_future<void>::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

inline int ft_future<void>::lock_internal(bool *lock_acquired) const
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

inline int ft_future<void>::unlock_internal(bool lock_acquired) const
{
    int mutex_result;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_result = this->_mutex->unlock();
    if (mutex_result != FT_ERR_SUCCESS)
        return (mutex_result);
    return (FT_ERR_SUCCESS);
}

#ifdef LIBFT_TEST_BUILD
template <typename ValueType>
pt_recursive_mutex *ft_future<ValueType>::get_mutex_for_validation() noexcept
{
    return (this->_mutex);
}

inline pt_recursive_mutex *ft_future<void>::get_mutex_for_validation() noexcept
{
    return (this->_mutex);
}
#endif

#endif
