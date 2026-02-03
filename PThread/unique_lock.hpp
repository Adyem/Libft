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

        void push_result(int error_code) const noexcept
        {
            ft_global_error_stack_push(error_code);
        }

        int consume_mutex_error(void) const noexcept
        {
            return (ft_global_error_stack_pop_newest());
        }

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
        int last_operation_error() const noexcept;
        const char *last_operation_error_str() const noexcept;
};

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock()
    : _mutex(ft_nullptr)
    , _owns_lock(false)
{
    this->push_result(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex)
    : _mutex(&mutex)
    , _owns_lock(false)
{
    this->lock();
    this->push_result(FT_ERR_SUCCESSS);
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
    this->push_result(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(ft_unique_lock &&other)
    : _mutex(other._mutex)
    , _owns_lock(other._owns_lock)
{
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    this->push_result(FT_ERR_SUCCESSS);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType> &ft_unique_lock<MutexType>::operator=(ft_unique_lock &&other)
{
    if (this != &other)
    {
        if (this->_owns_lock && this->_mutex != ft_nullptr)
            this->unlock();
        this->_mutex = other._mutex;
        this->_owns_lock = other._owns_lock;
        other._mutex = ft_nullptr;
        other._owns_lock = false;
    }
    this->push_result(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename MutexType>
void ft_unique_lock<MutexType>::lock()
{
    int final_error;

    if (this->_mutex == ft_nullptr)
    {
        final_error = FT_ERR_INVALID_ARGUMENT;
        this->push_result(final_error);
        return ;
    }
    if (this->_owns_lock)
    {
        final_error = FT_ERR_MUTEX_ALREADY_LOCKED;
        this->push_result(final_error);
        return ;
    }
    this->_mutex->lock(THREAD_ID);
    final_error = this->consume_mutex_error();
    if (final_error == FT_ERR_SUCCESSS)
        this->_owns_lock = true;
    else
        this->_owns_lock = false;
    this->push_result(final_error);
    return ;
}

template <typename MutexType>
void ft_unique_lock<MutexType>::unlock()
{
    int final_error;

    if (this->_mutex == ft_nullptr)
    {
        final_error = FT_ERR_INVALID_ARGUMENT;
        this->push_result(final_error);
        return ;
    }
    if (!this->_owns_lock)
    {
        final_error = FT_ERR_INVALID_ARGUMENT;
        this->push_result(final_error);
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    final_error = this->consume_mutex_error();
    if (final_error == FT_ERR_SUCCESSS)
        this->_owns_lock = false;
    this->push_result(final_error);
    return ;
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const
{
    bool owns_value;

    owns_value = this->_owns_lock;
    this->push_result(FT_ERR_SUCCESSS);
    return (owns_value);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const
{
    MutexType *mutex_pointer;

    mutex_pointer = this->_mutex;
    this->push_result(FT_ERR_SUCCESSS);
    return (mutex_pointer);
}

template <typename MutexType>
int ft_unique_lock<MutexType>::last_operation_error() const noexcept
{
    return (ft_global_error_stack_last_error());
}

template <typename MutexType>
const char *ft_unique_lock<MutexType>::last_operation_error_str() const noexcept
{
    return (ft_global_error_stack_last_error_str());
}

#endif
