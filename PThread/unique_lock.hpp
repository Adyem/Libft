#ifndef UNIQUE_LOCK_HPP
# define UNIQUE_LOCK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "mutex.hpp"
#include "pthread.hpp"

template <typename MutexType>
class ft_unique_lock
{
    private:
        MutexType *_mutex;
        bool _owns_lock;

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
};

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock()
    : _mutex(ft_nullptr)
    , _owns_lock(false)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex)
    : _mutex(&mutex)
    , _owns_lock(false)
{
    this->lock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        this->unlock();
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(ft_unique_lock &&other)
    : _mutex(other._mutex)
    , _owns_lock(other._owns_lock)
{
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType> &ft_unique_lock<MutexType>::operator=(ft_unique_lock &&other)
{
    int assignment_error;

    assignment_error = FT_ERR_SUCCESSS;
    if (this != &other)
    {
        if (this->_owns_lock && this->_mutex != ft_nullptr)
        {
            this->unlock();
            assignment_error = ft_global_error_stack_drop_last_error();
            if (assignment_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(assignment_error);
                return (*this);
            }
        }
        this->_mutex = other._mutex;
        this->_owns_lock = other._owns_lock;
        other._mutex = ft_nullptr;
        other._owns_lock = false;
    }
    ft_global_error_stack_push(assignment_error);
    return (*this);
}

template <typename MutexType>
void ft_unique_lock<MutexType>::lock()
{
    int mutex_error;
    int stack_error;
    int final_error;

    if (this->_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (this->_owns_lock)
    {
        ft_global_error_stack_push(FT_ERR_MUTEX_ALREADY_LOCKED);
        return ;
    }
    mutex_error = this->_mutex->lock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    final_error = stack_error != FT_ERR_SUCCESSS ? stack_error : mutex_error;
    if (mutex_error == FT_ERR_SUCCESSS)
        this->_owns_lock = true;
    else
        this->_owns_lock = false;
    ft_global_error_stack_push(final_error);
    return ;
}

template <typename MutexType>
void ft_unique_lock<MutexType>::unlock()
{
    int mutex_error;
    int stack_error;
    int final_error;

    if (this->_mutex == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (!this->_owns_lock)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    mutex_error = this->_mutex->unlock(THREAD_ID);
    stack_error = ft_global_error_stack_drop_last_error();
    final_error = stack_error != FT_ERR_SUCCESSS ? stack_error : mutex_error;
    if (final_error == FT_ERR_SUCCESSS)
        this->_owns_lock = false;
    else
        this->_owns_lock = false;
    ft_global_error_stack_push(final_error);
    return ;
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const
{
    bool owns_value;

    owns_value = this->_owns_lock;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (owns_value);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const
{
    MutexType *mutex_pointer;

    mutex_pointer = this->_mutex;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (mutex_pointer);
}

#endif
