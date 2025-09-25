#ifndef UNIQUE_LOCK_HPP
#define UNIQUE_LOCK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "pthread.hpp"

template <typename MutexType>
class ft_unique_lock
{
    private:
        MutexType *_mutex;
        bool _owns_lock;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_unique_lock();
        explicit ft_unique_lock(MutexType &mutex);
        ~ft_unique_lock();

        ft_unique_lock(const ft_unique_lock &) = delete;
        ft_unique_lock &operator=(const ft_unique_lock &) = delete;

        ft_unique_lock(ft_unique_lock &&other);
        ft_unique_lock &operator=(ft_unique_lock &&other);

        void lock();
        void unlock();

        bool owns_lock() const;
        MutexType *mutex() const;
        int get_error() const;
        const char *get_error_str() const;
};

template <typename MutexType>
void ft_unique_lock<MutexType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock()
    : _mutex(ft_nullptr), _owns_lock(false), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex)
    : _mutex(&mutex), _owns_lock(false), _error_code(ER_SUCCESS)
{
    this->lock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    if (this->_owns_lock && this->_mutex)
        this->unlock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(ft_unique_lock &&other)
    : _mutex(other._mutex), _owns_lock(other._owns_lock), _error_code(other._error_code)
{
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType> &ft_unique_lock<MutexType>::operator=(ft_unique_lock &&other)
{
    if (this != &other)
    {
        if (this->_owns_lock && this->_mutex)
            this->unlock();
        this->_mutex = other._mutex;
        this->_owns_lock = other._owns_lock;
        this->_error_code = other._error_code;
        other._mutex = ft_nullptr;
        other._owns_lock = false;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename MutexType>
void ft_unique_lock<MutexType>::lock()
{
    if (!this->_mutex)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    if (this->_owns_lock)
    {
        this->set_error(PT_ERR_ALREADY_LOCKED);
        return ;
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() == ER_SUCCESS)
    {
        this->_owns_lock = true;
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->_owns_lock = false;
    this->set_error(this->_mutex->get_error());
    return ;
}

template <typename MutexType>
void ft_unique_lock<MutexType>::unlock()
{
    if (!this->_mutex)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    if (!this->_owns_lock)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() == ER_SUCCESS)
    {
        this->_owns_lock = false;
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->set_error(this->_mutex->get_error());
    return ;
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const
{
    return (this->_owns_lock);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const
{
    return (this->_mutex);
}

template <typename MutexType>
int ft_unique_lock<MutexType>::get_error() const
{
    return (this->_error_code);
}

template <typename MutexType>
const char *ft_unique_lock<MutexType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif

