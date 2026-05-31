#ifndef THREADING_UNIQUE_LOCK_HPP
# define THREADING_UNIQUE_LOCK_HPP

#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Template/move.hpp"

template <typename MutexType>
class ft_unique_lock
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        MutexType *_mutex;
        bool _owns_lock;
        uint8_t _initialised_state;
        static thread_local uint32_t _last_error;
        static uint32_t set_error(uint32_t error_code) noexcept;
        static int32_t lock_if_not_null(const pt_mutex *mutex_pointer) noexcept;
        static int32_t unlock_if_not_null(const pt_mutex *mutex_pointer) noexcept;
        static int32_t lock_if_not_null(const pt_recursive_mutex *mutex_pointer) noexcept;
        static int32_t unlock_if_not_null(const pt_recursive_mutex *mutex_pointer) noexcept;

    public:
        ft_unique_lock() noexcept;
        explicit ft_unique_lock(MutexType &mutex) noexcept;
        ~ft_unique_lock();

        ft_unique_lock(const ft_unique_lock &) = delete;
        ft_unique_lock &operator=(const ft_unique_lock &) = delete;

        ft_unique_lock(ft_unique_lock &&other) noexcept = delete;
        ft_unique_lock &operator=(ft_unique_lock &&other) noexcept;

        void move_from(ft_unique_lock &other);

        int32_t initialize(MutexType &mutex);
        int32_t destroy();

        int32_t lock();
        int32_t unlock();

        bool owns_lock() const noexcept;
        bool is_initialised() const noexcept;
        MutexType *mutex() const noexcept;
        int32_t last_operation_error() const noexcept;
        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

template <typename MutexType>
thread_local uint32_t ft_unique_lock<MutexType>::_last_error = FT_ERR_SUCCESS;

template <typename MutexType>
uint32_t ft_unique_lock<MutexType>::set_error(uint32_t error_code) noexcept
{
    ft_unique_lock<MutexType>::_last_error = error_code;
    return (error_code);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::lock_if_not_null(const pt_mutex *mutex_pointer) noexcept
{
    return (pt_mutex_lock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::unlock_if_not_null(const pt_mutex *mutex_pointer) noexcept
{
    return (pt_mutex_unlock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::lock_if_not_null(
    const pt_recursive_mutex *mutex_pointer) noexcept
{
    return (pt_recursive_mutex_lock_if_not_null(mutex_pointer));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::unlock_if_not_null(
    const pt_recursive_mutex *mutex_pointer) noexcept
{
    return (pt_recursive_mutex_unlock_if_not_null(mutex_pointer));
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock() noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex) noexcept
    : _mutex(ft_nullptr)
    , _owns_lock(false)
    , _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    if (this->initialize(mutex) == FT_ERR_SUCCESS)
        (void)this->lock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    this->destroy();
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
    this->_initialised_state = other._initialised_state;
    (void)this->set_error(FT_ERR_SUCCESS);
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    (void)other.set_error(FT_ERR_SUCCESS);
    return (*this);
}

template <typename MutexType>
void ft_unique_lock<MutexType>::move_from(ft_unique_lock &other)
{
    (void)(*this = ft_move(other));
    return ;
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::initialize(MutexType &mutex)
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        return (this->set_error(FT_ERR_INVALID_STATE));
    }
    this->_mutex = &mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (this->set_error(FT_ERR_SUCCESS));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::destroy()
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->set_error(FT_ERR_SUCCESS));
    }
    if (this->_owns_lock && this->_mutex != ft_nullptr)
        (void)this->unlock();
    this->_mutex = ft_nullptr;
    this->_owns_lock = false;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (this->set_error(FT_ERR_SUCCESS));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::lock()
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        || this->_mutex == ft_nullptr)
    {
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    }
    if (this->_owns_lock)
    {
        return (this->set_error(FT_ERR_MUTEX_ALREADY_LOCKED));
    }
    int32_t mutex_error = this->lock_if_not_null(this->_mutex);

    this->_owns_lock = (mutex_error == FT_ERR_SUCCESS);
    return (this->set_error(static_cast<uint32_t>(mutex_error)));
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::unlock()
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        || this->_mutex == ft_nullptr)
    {
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    }
    if (!this->_owns_lock)
    {
        return (this->set_error(FT_ERR_INVALID_ARGUMENT));
    }
    int32_t mutex_error = this->unlock_if_not_null(this->_mutex);

    this->_owns_lock = false;
    return (this->set_error(static_cast<uint32_t>(mutex_error)));
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const noexcept
{
    (void)this->set_error(FT_ERR_SUCCESS);
    return (this->_owns_lock);
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::is_initialised() const noexcept
{
    (void)this->set_error(FT_ERR_SUCCESS);
    return (this->_initialised_state == FT_CLASS_STATE_INITIALISED);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const noexcept
{
    (void)this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::last_operation_error() const noexcept
{
    return (this->get_error());
}

template <typename MutexType>
int32_t ft_unique_lock<MutexType>::get_error() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_unique_lock::get_error");
    return (static_cast<int32_t>(_last_error));
}

template <typename MutexType>
const char *ft_unique_lock<MutexType>::get_error_str() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_unique_lock::get_error_str");
    return (ft_strerror(static_cast<int32_t>(_last_error)));
}

#endif
