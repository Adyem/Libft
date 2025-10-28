#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _error(ER_SUCCESS), _native_initialized(false),
    _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_native_initialized = true;
    this->set_error(ER_SUCCESS);
    return ;
}

pt_mutex::~pt_mutex()
{
    if (this->_native_initialized)
    {
        pthread_mutex_destroy(&this->_native_mutex);
        this->_native_initialized = false;
    }
    this->teardown_thread_safety();
    return ;
}

void    pt_mutex::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        ft_errno = error;
        return ;
    }
    this->_error = error;
    ft_errno = error;
    this->unlock_internal(lock_acquired);
    return ;
}

bool pt_mutex::ensure_native_mutex() const
{
    bool lock_acquired;
    bool initialized;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (false);
    initialized = this->_native_initialized;
    if (initialized)
    {
        this->unlock_internal(lock_acquired);
        return (true);
    }
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->unlock_internal(lock_acquired);
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    this->_native_initialized = true;
    this->unlock_internal(lock_acquired);
    return (true);
}

bool pt_mutex::lockState() const
{
    return (this->_lock.load(std::memory_order_acquire));
}

int pt_mutex::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void pt_mutex::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void pt_mutex::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

int pt_mutex::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void pt_mutex::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool pt_mutex::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<pt_mutex *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int pt_mutex::lock_state(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<pt_mutex *>(this)->set_error(ft_errno);
    else
        const_cast<pt_mutex *>(this)->set_error(ER_SUCCESS);
    return (result);
}

void pt_mutex::unlock_state(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
        const_cast<pt_mutex *>(this)->set_error(this->_state_mutex->get_error());
    else
        const_cast<pt_mutex *>(this)->set_error(entry_errno);
    return ;
}

int pt_mutex::get_error() const
{
    bool lock_acquired;
    int error_value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return (this->_error);
    error_value = this->_error;
    this->unlock_internal(lock_acquired);
    return (error_value);
}

const char *pt_mutex::get_error_str() const
{
    int error_value;

    error_value = this->get_error();
    return (ft_strerror(error_value));
}

