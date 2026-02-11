#ifndef UNIQUE_LOCK_HPP
# define UNIQUE_LOCK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "mutex.hpp"

template <typename MutexType>
class ft_unique_lock
{
    private:
        MutexType *_mutex;
        bool _owns_lock;
        bool _initialized;

    public:
        ft_unique_lock() noexcept;
        ~ft_unique_lock();

        ft_unique_lock(const ft_unique_lock &) = delete;
        ft_unique_lock &operator=(const ft_unique_lock &) = delete;

        ft_unique_lock(ft_unique_lock &&other) = delete;
        ft_unique_lock &operator=(ft_unique_lock &&other) = delete;

        void move_from(ft_unique_lock &other);

        int32_t initialize(MutexType &mutex);
        int32_t destroy();

        int32_t lock();
        int32_t unlock();

        bool owns_lock() const noexcept;
        bool is_initialized() const noexcept;
        MutexType *mutex() const noexcept;
};

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock() noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialized(false)
{
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    this->destroy();
    return ;
}


template <typename MutexType>
int32_t ft_unique_lock<MutexType>::initialize(MutexType &mutex)
{
    if (this->_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_mutex = &mutex;
    this->_initialized = true;
    return (FT_ERR_SUCCESSS);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::destroy()
{
    if (!this->_initialized)
        return (FT_ERR_SUCCESSS);
    if (this->_owns_lock && this->_mutex != ft_nullptr)
    {
        this->unlock();
    }
    this->_mutex = ft_nullptr;
    this->_owns_lock = false;
    this->_initialized = false;
    return (FT_ERR_SUCCESSS);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::lock()
{
    if (!this->_initialized || this->_mutex == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (this->_owns_lock)
    {
        return (FT_ERR_MUTEX_ALREADY_LOCKED);
    }
    int32_t mutex_error = this->_mutex->lock();
    this->_owns_lock = (mutex_error == FT_ERR_SUCCESSS);
    return (mutex_error);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::unlock()
{
    if (!this->_initialized || this->_mutex == ft_nullptr)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (!this->_owns_lock)
    {
        return (FT_ERR_INVALID_ARGUMENT);
    }
    int32_t mutex_error = this->_mutex->unlock();
    this->_owns_lock = false;
    return (mutex_error);
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const noexcept
{
    return (this->_owns_lock);
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::is_initialized() const noexcept
{
    return (this->_initialized);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const noexcept
{
    return (this->_mutex);
}

#endif
