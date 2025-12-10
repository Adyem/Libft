#ifndef LOCK_GUARD_HPP
#define LOCK_GUARD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

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
        mutable int _error_code;

        void set_error(int error) const;

    public:
        explicit ft_lock_guard(MutexType &mutex);
        ~ft_lock_guard();

        ft_lock_guard(const ft_lock_guard &) = delete;
        ft_lock_guard &operator=(const ft_lock_guard &) = delete;

        int get_error() const;
        const char *get_error_str() const;
};

template <typename MutexType>
void ft_lock_guard<MutexType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename MutexType>
ft_lock_guard<MutexType>::ft_lock_guard(MutexType &mutex)
    : _mutex(&mutex), _owns_lock(false), _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ER_SUCCESSS)
    {
        int mutex_error;

        mutex_error = this->_mutex->get_error();
        this->_mutex = ft_nullptr;
        this->_owns_lock = false;
        this->set_error(mutex_error);
        return ;
    }
    this->_owns_lock = true;
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_lock_guard<MutexType>::~ft_lock_guard()
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        this->_mutex->unlock(THREAD_ID);
        if (this->_mutex->get_error() != FT_ER_SUCCESSS)
        {
            this->set_error(this->_mutex->get_error());
            return ;
        }
    }
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename MutexType>
int ft_lock_guard<MutexType>::get_error() const
{
    return (this->_error_code);
}

template <typename MutexType>
const char *ft_lock_guard<MutexType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif
