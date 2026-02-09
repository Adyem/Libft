#ifndef LOCK_GUARD_HPP
#define LOCK_GUARD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>
#include <memory>

#ifndef PTHREAD_NO_PROMISE
#define LOCK_GUARD_DEFINED_PTHREAD_NO_PROMISE
#define PTHREAD_NO_PROMISE
#endif
#include "pthread.hpp"
#ifdef LOCK_GUARD_DEFINED_PTHREAD_NO_PROMISE
#undef PTHREAD_NO_PROMISE
#undef LOCK_GUARD_DEFINED_PTHREAD_NO_PROMISE
#endif

template <typename MutexType>
class ft_lock_guard
{
    private:
        MutexType *_mutex;
        bool _owns_lock;

    public:
        explicit ft_lock_guard(MutexType &mutex);
        ~ft_lock_guard();

        ft_lock_guard(const ft_lock_guard &) = delete;
        ft_lock_guard &operator=(const ft_lock_guard &) = delete;
};

template <typename MutexType>
ft_lock_guard<MutexType>::ft_lock_guard(MutexType &mutex)
    : _mutex(&mutex)
    , _owns_lock(false)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->_mutex->lock(THREAD_ID);
    int mutex_error = ft_global_error_stack_peek_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->_mutex = ft_nullptr;
        this->_owns_lock = false;
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    this->_owns_lock = true;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_lock_guard<MutexType>::~ft_lock_guard()
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        this->_mutex->unlock(THREAD_ID);
        int mutex_error = ft_global_error_stack_peek_last_error();
        if (mutex_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(mutex_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

#include "recursive_mutex.hpp"

enum class ft_recursive_mutex_pair_lock_order
{
    same_mutex,
    first_then_second,
    second_then_first
};

static inline ft_recursive_mutex_pair_lock_order ft_recursive_mutex_lock_pair(
        pt_recursive_mutex &first_mutex, pt_recursive_mutex &second_mutex)
{
    std::uintptr_t first_address = reinterpret_cast<std::uintptr_t>(std::addressof(first_mutex));
    std::uintptr_t second_address = reinterpret_cast<std::uintptr_t>(std::addressof(second_mutex));

    if (first_address == second_address)
    {
        first_mutex.lock();
        return (ft_recursive_mutex_pair_lock_order::same_mutex);
    }
    if (first_address < second_address)
    {
        first_mutex.lock();
        second_mutex.lock();
        return (ft_recursive_mutex_pair_lock_order::first_then_second);
    }
    second_mutex.lock();
    first_mutex.lock();
    return (ft_recursive_mutex_pair_lock_order::second_then_first);
}

static inline void ft_recursive_mutex_unlock_pair(pt_recursive_mutex &first_mutex,
        pt_recursive_mutex &second_mutex, ft_recursive_mutex_pair_lock_order order)
{
    if (order == ft_recursive_mutex_pair_lock_order::same_mutex)
    {
        first_mutex.unlock();
        return ;
    }
    if (order == ft_recursive_mutex_pair_lock_order::first_then_second)
    {
        second_mutex.unlock();
        first_mutex.unlock();
        return ;
    }
    first_mutex.unlock();
    second_mutex.unlock();
    return ;
}

class ft_recursive_lock_guard
{
    private:
        pt_recursive_mutex *_mutex;

    public:
        explicit ft_recursive_lock_guard(pt_recursive_mutex &mutex)
            : _mutex(&mutex)
        {
            this->_mutex->lock(THREAD_ID);
        }

        ~ft_recursive_lock_guard()
        {
            this->_mutex->unlock(THREAD_ID);
        }

        ft_recursive_lock_guard(const ft_recursive_lock_guard &) = delete;
        ft_recursive_lock_guard &operator=(const ft_recursive_lock_guard &) = delete;
};

class ft_recursive_mutex_pair_guard
{
    private:
        pt_recursive_mutex *_first;
        pt_recursive_mutex *_second;
        ft_recursive_mutex_pair_lock_order _lock_order;

    public:
        ft_recursive_mutex_pair_guard(pt_recursive_mutex &first, pt_recursive_mutex &second)
            : _first(&first)
            , _second(&second)
        {
            this->_lock_order = ft_recursive_mutex_lock_pair(first, second);
        }

        ~ft_recursive_mutex_pair_guard()
        {
            ft_recursive_mutex_unlock_pair(*this->_first, *this->_second, this->_lock_order);
        }

        ft_recursive_mutex_pair_guard(const ft_recursive_mutex_pair_guard &) = delete;
        ft_recursive_mutex_pair_guard &operator=(const ft_recursive_mutex_pair_guard &) = delete;
};

#endif
