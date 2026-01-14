#ifndef TEMPLATE_FUNCTION_HPP
#define TEMPLATE_FUNCTION_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <utility>
#include <new>
#include <type_traits>
#include <stdint.h>
#include "move.hpp"

template <typename Signature>
class ft_function;

template <typename ReturnType, typename... Args>
class ft_function<ReturnType(Args...)>
{
    private:
        void *_callable;
        ReturnType (*_invoke)(void *, Args...);
        void (*_destroy)(void *);
        void *(*_clone)(void *);
        mutable int _error_code;
        mutable pt_mutex *_state_mutex;
        bool _thread_safe_enabled;

        void set_error(int error) const;
        void clear_callable_unlocked();
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

        template <typename FunctionType>
        static ReturnType invoke(void *callable, Args... args);

        template <typename FunctionType>
        static void destroy(void *callable);

        template <typename FunctionType>
        static void *clone(void *callable);

    public:
        ft_function();

        template <typename FunctionType>
        ft_function(FunctionType function);

        ft_function(const ft_function &other);
        ft_function(ft_function &&other);
        ~ft_function();

        ft_function &operator=(const ft_function &other);
        ft_function &operator=(ft_function &&other);

        ReturnType operator()(Args... args) const;
        explicit operator bool() const;

        int get_error() const;
        const char *get_error_str() const;
        void *get_mutex_address_debug() const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::set_error(int error) const
{
    this->_error_code = error;
    return ;
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::clear_callable_unlocked()
{
    if (this->_destroy && this->_callable)
    {
        this->_destroy(this->_callable);
    }
    this->_callable = ft_nullptr;
    this->_invoke = ft_nullptr;
    this->_destroy = ft_nullptr;
    this->_clone = ft_nullptr;
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
ReturnType ft_function<ReturnType(Args...)>::invoke(void *callable, Args... args)
{
    FunctionType *function;

    function = static_cast<FunctionType *>(callable);
    return ((*function)(args...));
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
void ft_function<ReturnType(Args...)>::destroy(void *callable)
{
    FunctionType *function;

    function = static_cast<FunctionType *>(callable);
    delete function;
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
void *ft_function<ReturnType(Args...)>::clone(void *callable)
{
    FunctionType *function;
    FunctionType *copy;

    function = static_cast<FunctionType *>(callable);
    copy = new (std::nothrow) FunctionType(*function);
    return (copy);
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function()
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(FT_ERR_SUCCESSS), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
template <typename FunctionType>
ft_function<ReturnType(Args...)>::ft_function(FunctionType function)
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(FT_ERR_SUCCESSS), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    FunctionType *copy;

    copy = new (std::nothrow) FunctionType(ft_move(function));
    if (!copy)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_callable = copy;
    this->_invoke = &ft_function<ReturnType(Args...)>::template invoke<FunctionType>;
    this->_destroy = &ft_function<ReturnType(Args...)>::template destroy<FunctionType>;
    this->_clone = &ft_function<ReturnType(Args...)>::template clone<FunctionType>;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function(const ft_function &other)
    : _callable(ft_nullptr), _invoke(ft_nullptr), _destroy(ft_nullptr),
      _clone(ft_nullptr), _error_code(FT_ERR_SUCCESSS), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    int other_error_code;
    bool other_thread_safe;
    void *new_callable;

    other_lock_acquired = false;
    other_error_code = FT_ERR_SUCCESSS;
    other_thread_safe = false;
    new_callable = ft_nullptr;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (other._callable != ft_nullptr)
    {
        new_callable = other._clone(other._callable);
        if (new_callable == ft_nullptr)
        {
            other.unlock_internal(other_lock_acquired);
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
    }
    this->_callable = new_callable;
    this->_invoke = other._invoke;
    this->_destroy = other._destroy;
    this->_clone = other._clone;
    other_error_code = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other.unlock_internal(other_lock_acquired);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            if (this->_destroy != ft_nullptr && this->_callable != ft_nullptr)
            {
                this->_destroy(this->_callable);
            }
            this->_callable = ft_nullptr;
            this->_invoke = ft_nullptr;
            this->_destroy = ft_nullptr;
            this->_clone = ft_nullptr;
            return ;
        }
    }
    this->_error_code = other_error_code;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::ft_function(ft_function &&other)
    : _callable(ft_nullptr), _invoke(ft_nullptr),
      _destroy(ft_nullptr), _clone(ft_nullptr), _error_code(FT_ERR_SUCCESSS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    void *transferred_callable;
    ReturnType (*transferred_invoke)(void *, Args...);
    void (*transferred_destroy)(void *);
    void *(*transferred_clone)(void *);
    int other_error_code;
    bool other_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    transferred_callable = other._callable;
    transferred_invoke = other._invoke;
    transferred_destroy = other._destroy;
    transferred_clone = other._clone;
    other_error_code = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other._callable = ft_nullptr;
    other._invoke = ft_nullptr;
    other._destroy = ft_nullptr;
    other._clone = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    other._thread_safe_enabled = false;
    other._error_code = FT_ERR_SUCCESSS;
    this->_callable = transferred_callable;
    this->_invoke = transferred_invoke;
    this->_destroy = transferred_destroy;
    this->_clone = transferred_clone;
    this->_error_code = other_error_code;
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::~ft_function()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->clear_callable_unlocked();
        this->unlock_internal(lock_acquired);
    }
    else
        this->clear_callable_unlocked();
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)> &ft_function<ReturnType(Args...)>::operator=(const ft_function &other)
{
    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;
    void *new_callable;
    ReturnType (*new_invoke)(void *, Args...);
    void (*new_destroy)(void *);
    void *(*new_clone)(void *);
    int other_error_code;
    bool other_thread_safe;
    pt_mutex *previous_mutex;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(this->get_error());
        return (*this);
    }
    new_callable = ft_nullptr;
    if (other._callable != ft_nullptr)
    {
        new_callable = other._clone(other._callable);
        if (new_callable == ft_nullptr)
        {
            other.unlock_internal(other_lock_acquired);
            this->unlock_internal(this_lock_acquired);
            this->set_error(FT_ERR_NO_MEMORY);
            return (*this);
        }
    }
    new_invoke = other._invoke;
    new_destroy = other._destroy;
    new_clone = other._clone;
    other_error_code = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other.unlock_internal(other_lock_acquired);
    previous_mutex = this->_state_mutex;
    this->clear_callable_unlocked();
    this->_callable = new_callable;
    this->_invoke = new_invoke;
    this->_destroy = new_destroy;
    this->_clone = new_clone;
    this->_error_code = other_error_code;
    this->unlock_internal(this_lock_acquired);
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            if (previous_mutex != ft_nullptr)
            {
                previous_mutex->~pt_mutex();
                cma_free(previous_mutex);
            }
            return (*this);
        }
    }
    if (previous_mutex != ft_nullptr)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)> &ft_function<ReturnType(Args...)>::operator=(ft_function &&other)
{
    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;
    void *transferred_callable;
    ReturnType (*transferred_invoke)(void *, Args...);
    void (*transferred_destroy)(void *);
    void *(*transferred_clone)(void *);
    int other_error_code;
    bool other_thread_safe;
    pt_mutex *previous_mutex;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(this->get_error());
        return (*this);
    }
    this->clear_callable_unlocked();
    transferred_callable = other._callable;
    transferred_invoke = other._invoke;
    transferred_destroy = other._destroy;
    transferred_clone = other._clone;
    other_error_code = other._error_code;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other._callable = ft_nullptr;
    other._invoke = ft_nullptr;
    other._destroy = ft_nullptr;
    other._clone = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    other._thread_safe_enabled = false;
    other._error_code = FT_ERR_SUCCESSS;
    previous_mutex = this->_state_mutex;
    this->_callable = transferred_callable;
    this->_invoke = transferred_invoke;
    this->_destroy = transferred_destroy;
    this->_clone = transferred_clone;
    this->_error_code = other_error_code;
    this->unlock_internal(this_lock_acquired);
    this->_state_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            if (previous_mutex != ft_nullptr)
            {
                previous_mutex->~pt_mutex();
                cma_free(previous_mutex);
            }
            return (*this);
        }
    }
    if (previous_mutex != ft_nullptr)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ReturnType, typename... Args>
ReturnType ft_function<ReturnType(Args...)>::operator()(Args... args) const
{
    ReturnType (*invoke_target)(void *, Args...);
    void *callable;
    bool lock_acquired;

    invoke_target = ft_nullptr;
    callable = ft_nullptr;
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        if constexpr (std::is_void<ReturnType>::value)
        {
            return ;
        }
        return (ReturnType());
    }
    invoke_target = this->_invoke;
    callable = this->_callable;
    if (invoke_target == ft_nullptr || callable == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        if constexpr (std::is_void<ReturnType>::value)
        {
            return ;
        }
        return (ReturnType());
    }
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    if constexpr (std::is_void<ReturnType>::value)
    {
        invoke_target(callable, args...);
        return ;
    }
    return (invoke_target(callable, args...));
}

template <typename ReturnType, typename... Args>
ft_function<ReturnType(Args...)>::operator bool() const
{
    bool has_callable;
    bool lock_acquired;

    has_callable = false;
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (false);
    }
    if (this->_callable && this->_invoke)
    {
        has_callable = true;
    }
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ERR_SUCCESSS);
    return (has_callable);
}

template <typename ReturnType, typename... Args>
int ft_function<ReturnType(Args...)>::get_error() const
{
    return (this->_error_code);
}

template <typename ReturnType, typename... Args>
const char *ft_function<ReturnType(Args...)>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ReturnType, typename... Args>
void *ft_function<ReturnType(Args...)>::get_mutex_address_debug() const
{
    return (this->_state_mutex);
}

template <typename ReturnType, typename... Args>
int ft_function<ReturnType(Args...)>::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
bool ft_function<ReturnType(Args...)>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_function<ReturnType(Args...)> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ReturnType, typename... Args>
int ft_function<ReturnType(Args...)>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_function<ReturnType(Args...)> *>(this)->set_error(this->get_error());
        return (result);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::unlock(bool lock_acquired) const
{
    int mutex_error;

    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        const_cast<ft_function<ReturnType(Args...)> *>(this)->set_error(mutex_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
int ft_function<ReturnType(Args...)>::lock_internal(bool *lock_acquired) const
{
    int mutex_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (this->_state_mutex->is_owned_by_thread(THREAD_ID))
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::unlock_internal(bool lock_acquired) const
{
    int mutex_error;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    this->_state_mutex->unlock(THREAD_ID);
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ReturnType, typename... Args>
void ft_function<ReturnType(Args...)>::teardown_thread_safety()
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
