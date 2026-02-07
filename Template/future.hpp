#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
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
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int creation_result = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (creation_result != FT_ERR_SUCCESSS)
        return (creation_result);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESSS);
}

template <typename ValueType>
void ft_future<ValueType>::teardown_thread_safety() const
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename ValueType>
int ft_future<ValueType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ValueType>
void ft_future<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ValueType>
int ft_future<ValueType>::last_error_code() const
{
    return (ft_global_error_stack_peek_last_error());
}

template <typename ValueType>
ft_future<ValueType>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(const ft_future<ValueType> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    bool lock_acquired;
    ft_promise<ValueType> *other_promise;
    ft_sharedptr<ft_promise<ValueType> > other_shared;
    int other_error;

    lock_acquired = false;
    other_promise = ft_nullptr;
    int lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other.last_error_code();
    other.unlock_internal(lock_acquired);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    ft_global_error_stack_push(other_error);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_future<ValueType> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    bool other_lock_acquired;
    ft_promise<ValueType> *transferred_promise;
    ft_sharedptr<ft_promise<ValueType> > transferred_shared;
    int transferred_error;

    other_lock_acquired = false;
    int lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other.last_error_code();
    other.unlock_internal(other_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    ft_global_error_stack_push(transferred_error);
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
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer),
      _mutex(ft_nullptr)
{
    if (!this->_promise)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    {
        int shared_error = ft_global_error_stack_peek_last_error();

        if (shared_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(shared_error);
            return ;
        }
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    this->disable_thread_safety();
    this->_promise = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (*this);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (*this);
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other.last_error_code();
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    ft_global_error_stack_push(other_error);
    return (*this);
}

template <typename ValueType>
ft_future<ValueType> &ft_future<ValueType>::operator=(ft_future<ValueType> &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<ValueType> *transferred_promise;
    ft_sharedptr<ft_promise<ValueType> > transferred_shared;
    int transferred_error;
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (*this);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (*this);
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other.last_error_code();
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<ValueType> >();
    other.unlock_internal(other_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(this_lock_acquired);
    ft_global_error_stack_push(transferred_error);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            ft_global_error_stack_push(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (ValueType());
    }
    if (!this->wait_ready())
    {
        return (ValueType());
    }
    value = promise_pointer->get();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (is_valid);
}

inline int ft_future<void>::prepare_thread_safety() const
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int creation_result = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (creation_result != FT_ERR_SUCCESSS)
        return (creation_result);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESSS);
}

inline void ft_future<void>::teardown_thread_safety() const
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

inline int ft_future<void>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

inline void ft_future<void>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline bool ft_future<void>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

inline int ft_future<void>::last_error_code() const
{
    return (ft_global_error_stack_peek_last_error());
}

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

inline ft_future<void>::ft_future(const ft_future<void> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    bool lock_acquired;
    ft_promise<void> *other_promise;
    ft_sharedptr<ft_promise<void> > other_shared;
    int other_error;

    lock_acquired = false;
    other_promise = ft_nullptr;
    int lock_result = other.lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other.last_error_code();
    other.unlock_internal(lock_acquired);
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    ft_global_error_stack_push(other_error);
    return ;
}

inline ft_future<void>::ft_future(ft_future<void> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(ft_nullptr)
{
    bool lock_acquired;
    ft_promise<void> *transferred_promise;
    ft_sharedptr<ft_promise<void> > transferred_shared;
    int transferred_error;

    lock_acquired = false;
    int lock_result = other.lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other.last_error_code();
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<void> >();
    other.unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    ft_global_error_stack_push(transferred_error);
    return ;
}

inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _mutex(ft_nullptr)
{
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer),
      _mutex(ft_nullptr)
{
    if (!this->_promise)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    {
        int shared_error = ft_global_error_stack_peek_last_error();

        if (shared_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(shared_error);
            return ;
        }
    }
    return ;
}

inline ft_future<void>::~ft_future()
{
    this->disable_thread_safety();
    this->_promise = ft_nullptr;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline ft_future<void> &ft_future<void>::operator=(const ft_future<void> &other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<void> *other_promise;
    ft_sharedptr<ft_promise<void> > other_shared;
    int other_error;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (*this);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (*this);
    }
    other_promise = other._promise;
    other_shared = other._shared_promise;
    other_error = other.last_error_code();
    this->_promise = other_promise;
    this->_shared_promise = other_shared;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    ft_global_error_stack_push(other_error);
    return (*this);
}

inline ft_future<void> &ft_future<void>::operator=(ft_future<void> &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    ft_promise<void> *transferred_promise;
    ft_sharedptr<ft_promise<void> > transferred_shared;
    int transferred_error;

    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    this_lock_acquired = false;
    int this_lock_result = this->lock_internal(&this_lock_acquired);
    if (this_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(this_lock_result);
        return (*this);
    }
    other_lock_acquired = false;
    int other_lock_result = other.lock_internal(&other_lock_acquired);
    if (other_lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_result);
        return (*this);
    }
    transferred_promise = other._promise;
    transferred_shared = ft_move(other._shared_promise);
    transferred_error = other.last_error_code();
    this->_promise = transferred_promise;
    this->_shared_promise = ft_move(transferred_shared);
    other._promise = ft_nullptr;
    other._shared_promise = ft_sharedptr<ft_promise<void> >();
    other.unlock_internal(other_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(this_lock_acquired);
    ft_global_error_stack_push(transferred_error);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!promise_pointer->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            ft_global_error_stack_push(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
        return ;
    }
    if (!this->wait_ready())
    {
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_INVALID_STATE);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (is_valid);
}

template <typename ValueType>
int ft_future<ValueType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
void ft_future<ValueType>::unlock(bool lock_acquired) const
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
int ft_future<ValueType>::lock_internal(bool *lock_acquired) const
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
int ft_future<ValueType>::unlock_internal(bool lock_acquired) const
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

inline int ft_future<void>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

inline void ft_future<void>::unlock(bool lock_acquired) const
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

inline int ft_future<void>::lock_internal(bool *lock_acquired) const
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

inline int ft_future<void>::unlock_internal(bool lock_acquired) const
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
