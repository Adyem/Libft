#ifndef THREADING_LOCK_GUARD_HPP
#define THREADING_LOCK_GUARD_HPP

#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>
#include <memory>

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
    int mutex_error = this->lock_if_not_null(this->_mutex);
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->_mutex = ft_nullptr;
        this->_owns_lock = false;
        return ;
    }
    this->_owns_lock = true;
    return ;
}

template <typename MutexType>
ft_lock_guard<MutexType>::~ft_lock_guard()
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        int mutex_error = this->unlock_if_not_null(this->_mutex);
        if (mutex_error != FT_ERR_SUCCESS)
            return ;
    }
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
