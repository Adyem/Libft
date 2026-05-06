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
#include "pthread_internal.hpp"

template <typename MutexType>
class ft_lock_guard
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        MutexType *_mutex;
        bool _owns_lock;
        static int32_t lock_if_not_null(const pt_mutex *mutex_pointer) noexcept;
        static int32_t unlock_if_not_null(const pt_mutex *mutex_pointer) noexcept;
        static int32_t lock_if_not_null(const pt_recursive_mutex *mutex_pointer) noexcept;
        static int32_t unlock_if_not_null(const pt_recursive_mutex *mutex_pointer) noexcept;

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
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    int mutex_error = this->lock_if_not_null(this->_mutex);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->_mutex = ft_nullptr;
        this->_owns_lock = false;
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    this->_owns_lock = true;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename MutexType>
ft_lock_guard<MutexType>::~ft_lock_guard()
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        int mutex_error = this->unlock_if_not_null(this->_mutex);
        if (mutex_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(mutex_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename MutexType>
int32_t ft_lock_guard<MutexType>::lock_if_not_null(const pt_mutex *mutex_pointer) noexcept
{
    return (pt_mutex_lock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_lock_guard<MutexType>::unlock_if_not_null(const pt_mutex *mutex_pointer) noexcept
{
    return (pt_mutex_unlock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_lock_guard<MutexType>::lock_if_not_null(
    const pt_recursive_mutex *mutex_pointer) noexcept
{
    return (pt_recursive_mutex_lock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_lock_guard<MutexType>::unlock_if_not_null(
    const pt_recursive_mutex *mutex_pointer) noexcept
{
    return (pt_recursive_mutex_unlock_if_not_null(mutex_pointer));
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
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        pt_recursive_mutex *_mutex;

    public:
        explicit ft_recursive_lock_guard(pt_recursive_mutex &mutex);
        ~ft_recursive_lock_guard();

        ft_recursive_lock_guard(const ft_recursive_lock_guard &) = delete;
        ft_recursive_lock_guard &operator=(const ft_recursive_lock_guard &) = delete;
};

class ft_recursive_mutex_pair_guard
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        pt_recursive_mutex *_first;
        pt_recursive_mutex *_second;
        ft_recursive_mutex_pair_lock_order _lock_order;

    public:
        ft_recursive_mutex_pair_guard(pt_recursive_mutex &first, pt_recursive_mutex &second);
        ~ft_recursive_mutex_pair_guard();

        ft_recursive_mutex_pair_guard(const ft_recursive_mutex_pair_guard &) = delete;
        ft_recursive_mutex_pair_guard &operator=(const ft_recursive_mutex_pair_guard &) = delete;
};

inline ft_recursive_lock_guard::ft_recursive_lock_guard(pt_recursive_mutex &mutex)
    : _mutex(&mutex)
{
    (void)pt_recursive_mutex_lock_if_not_null(this->_mutex);
    return ;
}

inline ft_recursive_lock_guard::~ft_recursive_lock_guard()
{
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

inline ft_recursive_mutex_pair_guard::ft_recursive_mutex_pair_guard(pt_recursive_mutex &first,
        pt_recursive_mutex &second)
    : _first(&first)
    , _second(&second)
    , _lock_order(ft_recursive_mutex_pair_lock_order::same_mutex)
{
    this->_lock_order = ft_recursive_mutex_lock_pair(first, second);
    return ;
}

inline ft_recursive_mutex_pair_guard::~ft_recursive_mutex_pair_guard()
{
    ft_recursive_mutex_unlock_pair(*this->_first, *this->_second, this->_lock_order);
    return ;
}

#endif
