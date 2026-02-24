#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "pthread_lock_tracking.hpp"
#include <system_error>
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _native_mutex(ft_nullptr), _initialized_state(pt_mutex::_state_uninitialized),
    _state_mutex(ft_nullptr), _valid_state(false)
{
    return ;
}

pt_mutex::~pt_mutex()
{
    if (this->_initialized_state != pt_mutex::_state_initialized)
    {
        this->_initialized_state = pt_mutex::_state_destroyed;
        this->teardown_thread_safety();
        return ;
    }
    (void)this->destroy();
    this->teardown_thread_safety();
    return ;
}

int pt_mutex::ensure_native_mutex() const
{
    this->abort_if_not_initialized("pt_mutex::ensure_native_mutex");
    if (this->_native_mutex == ft_nullptr)
    {
        this->abort_lifecycle_error("pt_mutex::ensure_native_mutex",
            "native mutex pointer is null while object is initialized");
        return (FT_ERR_INVALID_STATE);
    }
    return (FT_ERR_SUCCESS);
}

int pt_mutex::initialize()
{
    if (this->_initialized_state == pt_mutex::_state_initialized)
    {
        this->abort_lifecycle_error("pt_mutex::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_native_mutex != ft_nullptr)
    {
        delete this->_native_mutex;
        this->_native_mutex = ft_nullptr;
    }
    this->_native_mutex = new (std::nothrow) std::mutex();
    if (this->_native_mutex == ft_nullptr)
    {
        this->_initialized_state = pt_mutex::_state_destroyed;
        this->_valid_state.store(false, std::memory_order_release);
        return (FT_ERR_NO_MEMORY);
    }
    this->_initialized_state = pt_mutex::_state_initialized;
    this->_valid_state.store(true, std::memory_order_release);
    return (FT_ERR_SUCCESS);
}

int pt_mutex::destroy()
{
    if (this->_initialized_state != pt_mutex::_state_initialized)
    {
        this->_initialized_state = pt_mutex::_state_destroyed;
        this->teardown_thread_safety();
        return (FT_ERR_SUCCESS);
    }
    if (this->_native_mutex == ft_nullptr)
    {
        this->_initialized_state = pt_mutex::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    if (this->_lock.load(std::memory_order_acquire))
        return (FT_ERR_THREAD_BUSY);
    delete this->_native_mutex;
    this->_native_mutex = ft_nullptr;
    this->_initialized_state = pt_mutex::_state_destroyed;
    this->_valid_state.store(false, std::memory_order_release);
    this->_lock.store(false, std::memory_order_release);
    this->_owner.store(0, std::memory_order_release);
    this->teardown_thread_safety();
    return (FT_ERR_SUCCESS);
}

void pt_mutex::abort_lifecycle_error(const char *method_name,
        const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "pt_mutex lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void pt_mutex::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == pt_mutex::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

bool pt_mutex::lockState() const
{
    this->abort_if_not_initialized("pt_mutex::lockState");
    return (this->_lock.load(std::memory_order_acquire));
}

int pt_mutex::lock_internal(bool *lock_acquired) const
{
    this->abort_if_not_initialized("pt_mutex::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int state_result = this->_state_mutex->lock_state(lock_acquired);
    if (state_result == 0)
    {
        if (lock_acquired != ft_nullptr)
            *lock_acquired = true;
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_INVALID_STATE);
}

int pt_mutex::unlock_internal(bool lock_acquired) const
{
    this->abort_if_not_initialized("pt_mutex::unlock_internal");
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    this->_state_mutex->unlock_state(lock_acquired);
    return (FT_ERR_SUCCESS);
}

void pt_mutex::teardown_thread_safety()
{
    if (this->_state_mutex == ft_nullptr)
        return ;
    (void)this->_state_mutex->destroy();
    delete this->_state_mutex;
    this->_state_mutex = ft_nullptr;
    return ;
}

int pt_mutex::lock_state(bool *lock_acquired) const
{
    this->abort_if_not_initialized("pt_mutex::lock_state");
    int lock_error = this->lock_internal(lock_acquired);

    if (lock_error == FT_ERR_SUCCESS)
        return (0);
    return (-1);
}

void pt_mutex::unlock_state(bool lock_acquired) const
{
    this->abort_if_not_initialized("pt_mutex::unlock_state");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

bool pt_mutex::is_owned_by_thread(pt_thread_id_type thread_id) const
{
    this->abort_if_not_initialized("pt_mutex::is_owned_by_thread");
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
    tracking_error = FT_ERR_SUCCESS;
    owned_mutexes = pt_lock_tracking::get_owned_mutexes(thread_id, &tracking_error);
    if (tracking_error != FT_ERR_SUCCESS)
        return (false);
    const void *mutex_handle = static_cast<const void *>(this->_native_mutex);
    index = 0;
    while (index < owned_mutexes.size)
    {
        if (owned_mutexes.data[index] == mutex_handle)
        {
            pt_buffer_destroy(owned_mutexes);
            return (true);
        }
        index += 1;
    }
    pt_buffer_destroy(owned_mutexes);
    return (false);
}
