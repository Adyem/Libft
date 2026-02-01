#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "pthread_lock_tracking.hpp"

unsigned long long pt_mutex::operation_error_push_entry_with_id(int error_code,
        unsigned long long operation_id) const
{
    return (ft_global_error_stack_push_entry_with_id(error_code, operation_id));
}

unsigned long long pt_mutex::operation_error_push_entry(int error_code) const
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    return (this->operation_error_push_entry_with_id(error_code, operation_id));
}

void pt_mutex::operation_error_push(int error_code) const
{
    ft_global_error_stack_push(error_code);
    return ;
}

int pt_mutex::operation_error_pop_last() const
{
    return (ft_global_error_stack_pop_last());
}

int pt_mutex::operation_error_pop_newest() const
{
    return (ft_global_error_stack_pop_newest());
}

void pt_mutex::operation_error_pop_all() const
{
    ft_global_error_stack_pop_all();
    return ;
}

int pt_mutex::operation_error_error_at(ft_size_t index) const
{
    return (ft_global_error_stack_error_at(index));
}

int pt_mutex::operation_error_last_error() const
{
    return (ft_global_error_stack_last_error());
}

ft_size_t pt_mutex::operation_error_depth() const
{
    return (ft_global_error_stack_depth());
}

unsigned long long pt_mutex::operation_error_get_id_at(ft_size_t index) const
{
    return (ft_global_error_stack_get_id_at(index));
}

ft_size_t pt_mutex::operation_error_find_by_id(unsigned long long operation_id) const
{
    return (ft_global_error_stack_find_by_id(operation_id));
}

const char *pt_mutex::operation_error_error_str_at(ft_size_t index) const
{
    return (ft_global_error_stack_error_str_at(index));
}

const char *pt_mutex::operation_error_last_error_str() const
{
    return (ft_global_error_stack_last_error_str());
}

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _native_initialized(false),
    _state_mutex(ft_nullptr), _valid_state(false)
{
    ft_bzero(&this->_native_mutex, sizeof(pthread_mutex_t));
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->operation_error_push(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_native_initialized = true;
    this->_valid_state.store(true, std::memory_order_release);
    this->operation_error_push(FT_ERR_SUCCESSS);
    return ;
}

pt_mutex::~pt_mutex()
{
    this->_valid_state.store(false, std::memory_order_release);
    if (this->_native_initialized)
    {
        pthread_mutex_destroy(&this->_native_mutex);
        this->_native_initialized = false;
    }
    this->teardown_thread_safety();
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
        this->_valid_state.store(false, std::memory_order_release);
        this->unlock_internal(lock_acquired);
        this->operation_error_push(FT_ERR_INVALID_STATE);
        return (false);
    }
    this->_native_initialized = true;
    this->_valid_state.store(true, std::memory_order_release);
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
    int state_error = this->_state_mutex->operation_error_last_error();
    if (state_error != FT_ERR_SUCCESSS)
        return (state_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

int pt_mutex::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_state_mutex->unlock(THREAD_ID);
    int state_error = this->_state_mutex->operation_error_last_error();
    if (state_error != FT_ERR_SUCCESSS)
        return (state_error);
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
        this->operation_error_push(lock_error);
    else
        this->operation_error_push(FT_ERR_SUCCESSS);
    return (result);
}

void pt_mutex::unlock_state(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
        this->operation_error_push(unlock_error);
    else
        this->operation_error_push(FT_ERR_SUCCESSS);
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

pthread_mutex_t   *pt_mutex::get_native_mutex() const
{
    if (!this->ensure_native_mutex())
        return (ft_nullptr);
    this->operation_error_push(FT_ERR_SUCCESSS);
    return (&this->_native_mutex);
}
