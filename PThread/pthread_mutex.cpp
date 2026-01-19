#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "pthread_lock_tracking.hpp"

thread_local ft_operation_error_stack pt_mutex::_operation_errors = {{}, {}, 0};

void pt_mutex::record_error(ft_operation_error_stack &error_stack, int error_code,
        bool push_global)
{
    unsigned long long operation_id;

    if (push_global)
        operation_id = ft_global_error_stack_push_entry(error_code);
    else
        operation_id = 0;
    ft_operation_error_stack_push(error_stack, error_code, operation_id);
    return ;
}

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _error(FT_ERR_SUCCESSS), _native_initialized(false),
    _state_mutex(ft_nullptr)
{
    ft_bzero(&this->_native_mutex, sizeof(pthread_mutex_t));
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_native_initialized = true;
    this->set_error(FT_ERR_SUCCESSS);
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
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        pt_mutex::record_error(pt_mutex::_operation_errors, error, true);
        return ;
    }
    this->_error = error;
    pt_mutex::record_error(pt_mutex::_operation_errors, error, true);
    this->unlock_internal(lock_acquired);
    return ;
}

bool pt_mutex::ensure_native_mutex() const
{
    bool lock_acquired;
    bool initialized;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
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
    if (this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
        return (this->_state_mutex->get_error());
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int pt_mutex::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ERR_SUCCESSS)
        return (this->_state_mutex->get_error());
    return (FT_ERR_SUCCESSS);
}

void pt_mutex::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    return ;
}

int pt_mutex::lock_state(bool *lock_acquired) const
{
    int result;
    int lock_error;

    lock_error = this->lock_internal(lock_acquired);
    result = (lock_error == FT_ERR_SUCCESSS ? 0 : -1);
    if (lock_error != FT_ERR_SUCCESSS)
        const_cast<pt_mutex *>(this)->set_error(lock_error);
    else
        const_cast<pt_mutex *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

void pt_mutex::unlock_state(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
        const_cast<pt_mutex *>(this)->set_error(unlock_error);
    else
        const_cast<pt_mutex *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

bool pt_mutex::is_owned_by_thread(pthread_t thread_id) const
{
    pthread_t owner_thread;
    pt_mutex_vector owned_mutexes;
    ft_size_t index;
    bool lock_flag;
    int tracking_error;

    lock_flag = this->_lock.load(std::memory_order_acquire);
    if (!lock_flag)
    {
        return (false);
    }
    owner_thread = this->_owner.load(std::memory_order_relaxed);
    if (owner_thread != 0)
    {
        bool matches_owner;

        matches_owner = (pt_thread_equal(owner_thread, thread_id) != 0);
        return (matches_owner);
    }
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id);
    tracking_error = ft_global_error_stack_pop_newest();
    if (tracking_error != FT_ERR_SUCCESSS)
        return (false);
    index = 0;
    while (index < owned_mutexes.size())
    {
        if (owned_mutexes[index] == &this->_native_mutex)
        {
            return (true);
        }
        index += 1;
    }
    return (false);
}

int pt_mutex::get_error() const
{
    bool lock_acquired;
    int error_value;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
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

pthread_mutex_t   *pt_mutex::get_native_mutex() const
{
    if (!this->ensure_native_mutex())
        return (ft_nullptr);
    this->set_error(FT_ERR_SUCCESSS);
    return (&this->_native_mutex);
}
