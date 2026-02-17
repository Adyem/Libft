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
        int32_t _last_error;

    public:
        ft_unique_lock() noexcept;
        explicit ft_unique_lock(MutexType &mutex) noexcept;
        ~ft_unique_lock();

        ft_unique_lock(const ft_unique_lock &) = delete;
        ft_unique_lock &operator=(const ft_unique_lock &) = delete;

        ft_unique_lock(ft_unique_lock &&other) noexcept;
        ft_unique_lock &operator=(ft_unique_lock &&other) noexcept;

        void move_from(ft_unique_lock &other);

        int32_t initialize(MutexType &mutex);
        int32_t destroy();

        int32_t lock();
        int32_t unlock();

        bool owns_lock() const noexcept;
        bool is_initialized() const noexcept;
        MutexType *mutex() const noexcept;
        int32_t last_operation_error() const noexcept;
};

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock() noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialized(false)
    , _last_error(FT_ERR_SUCCESS)
{
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex) noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialized(false)
    , _last_error(FT_ERR_SUCCESS)
{
    this->_last_error = this->initialize(mutex);
    if (this->_last_error == FT_ERR_SUCCESS)
        this->_last_error = this->lock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    this->destroy();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(ft_unique_lock &&other) noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialized(false)
    , _last_error(FT_ERR_SUCCESS)
{
    this->_mutex = other._mutex;
    this->_owns_lock = other._owns_lock;
    this->_initialized = other._initialized;
    this->_last_error = other._last_error;
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._initialized = false;
    other._last_error = FT_ERR_SUCCESS;
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType> &ft_unique_lock<MutexType>::operator=(ft_unique_lock &&other) noexcept
{
    if (this == &other)
        return (*this);
    (void)this->destroy();
    this->_mutex = other._mutex;
    this->_owns_lock = other._owns_lock;
    this->_initialized = other._initialized;
    this->_last_error = other._last_error;
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._initialized = false;
    other._last_error = FT_ERR_SUCCESS;
    return (*this);
}


template <typename MutexType>
int32_t ft_unique_lock<MutexType>::initialize(MutexType &mutex)
{
    if (this->_initialized)
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (this->_last_error);
    }
    this->_mutex = &mutex;
    this->_initialized = true;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::destroy()
{
    if (!this->_initialized)
    {
        this->_last_error = FT_ERR_SUCCESS;
        return (this->_last_error);
    }
    if (this->_owns_lock && this->_mutex != ft_nullptr)
        (void)this->unlock();
    this->_mutex = ft_nullptr;
    this->_owns_lock = false;
    this->_initialized = false;
    this->_last_error = FT_ERR_SUCCESS;
    return (this->_last_error);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::lock()
{
    if (!this->_initialized || this->_mutex == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        return (this->_last_error);
    }
    if (this->_owns_lock)
    {
        this->_last_error = FT_ERR_MUTEX_ALREADY_LOCKED;
        return (this->_last_error);
    }
    int32_t mutex_error = this->_mutex->lock();
    this->_owns_lock = (mutex_error == FT_ERR_SUCCESS);
    this->_last_error = mutex_error;
    return (this->_last_error);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::unlock()
{
    if (!this->_initialized || this->_mutex == ft_nullptr)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        return (this->_last_error);
    }
    if (!this->_owns_lock)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        return (this->_last_error);
    }
    int32_t mutex_error = this->_mutex->unlock();
    this->_owns_lock = false;
    this->_last_error = mutex_error;
    return (this->_last_error);
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

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::last_operation_error() const noexcept
{
    return (this->_last_error);
}

#endif
