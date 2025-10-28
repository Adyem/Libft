#include "api_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

api_connection_pool_handle::api_connection_pool_handle()
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(), socket(),
        tls_session(ft_nullptr), tls_context(ft_nullptr),
        security_mode(api_connection_security_mode::PLAIN), has_socket(false),
        from_pool(false), should_store(false), negotiated_http2(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

api_connection_pool_handle::api_connection_pool_handle(
        api_connection_pool_handle &&other)
        : _error_code(ER_SUCCESS), _mutex(ft_nullptr),
        _thread_safe_enabled(false), key(), socket(),
        tls_session(ft_nullptr), tls_context(ft_nullptr),
        security_mode(api_connection_security_mode::PLAIN), has_socket(false),
        from_pool(false), should_store(false), negotiated_http2(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->key = ft_move(other.key);
    this->socket = ft_move(other.socket);
    this->tls_session = other.tls_session;
    this->tls_context = other.tls_context;
    this->security_mode = other.security_mode;
    this->has_socket = other.has_socket;
    this->from_pool = other.from_pool;
    this->should_store = other.should_store;
    this->negotiated_http2 = other.negotiated_http2;
    other.unlock_internal(other_lock_acquired);
    this->_mutex = other._mutex;
    this->_thread_safe_enabled = other._thread_safe_enabled;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.tls_session = ft_nullptr;
    other.tls_context = ft_nullptr;
    other.has_socket = false;
    other.from_pool = false;
    other.should_store = false;
    other.negotiated_http2 = false;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

api_connection_pool_handle::~api_connection_pool_handle()
{
    this->teardown_thread_safety();
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
    this->has_socket = false;
    this->from_pool = false;
    this->should_store = false;
    this->negotiated_http2 = false;
    this->set_error(ER_SUCCESS);
    return ;
}

api_connection_pool_handle &api_connection_pool_handle::operator=(
        api_connection_pool_handle &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;

    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    this->key = ft_move(other.key);
    this->socket = ft_move(other.socket);
    this->tls_session = other.tls_session;
    this->tls_context = other.tls_context;
    this->security_mode = other.security_mode;
    this->has_socket = other.has_socket;
    this->from_pool = other.from_pool;
    this->should_store = other.should_store;
    this->negotiated_http2 = other.negotiated_http2;
    this->unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    if (this->_thread_safe_enabled)
        this->teardown_thread_safety();
    this->_mutex = other._mutex;
    this->_thread_safe_enabled = other._thread_safe_enabled;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.tls_session = ft_nullptr;
    other.tls_context = ft_nullptr;
    other.has_socket = false;
    other.from_pool = false;
    other.should_store = false;
    other.negotiated_http2 = false;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

int api_connection_pool_handle::enable_thread_safety()
{
    void *allocated_memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    allocated_memory = cma_malloc(sizeof(pt_mutex));
    if (allocated_memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(allocated_memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error_code;

        mutex_error_code = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(allocated_memory);
        this->set_error(mutex_error_code);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void api_connection_pool_handle::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool api_connection_pool_handle::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<api_connection_pool_handle *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int api_connection_pool_handle::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<api_connection_pool_handle *>(this)->set_error(ft_errno);
    else
        const_cast<api_connection_pool_handle *>(this)->set_error(ER_SUCCESS);
    return (result);
}

void api_connection_pool_handle::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<api_connection_pool_handle *>(this)->set_error(this->_mutex->get_error());
    else
        const_cast<api_connection_pool_handle *>(this)->set_error(entry_errno);
    return ;
}

int api_connection_pool_handle::get_error() const
{
    return (this->_error_code);
}

const char *api_connection_pool_handle::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

void api_connection_pool_handle::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int api_connection_pool_handle::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void api_connection_pool_handle::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void api_connection_pool_handle::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

api_connection_pool_handle_lock_guard::api_connection_pool_handle_lock_guard(
        api_connection_pool_handle &handle)
        : _handle(handle), _lock_acquired(false), _lock_result(-1)
{
    this->_lock_result = this->_handle.lock(&this->_lock_acquired);
    return ;
}

api_connection_pool_handle_lock_guard::~api_connection_pool_handle_lock_guard()
{
    this->_handle.unlock(this->_lock_acquired);
    return ;
}

bool api_connection_pool_handle_lock_guard::is_locked() const
{
    if (this->_lock_result != 0)
        return (false);
    return (this->_lock_acquired);
}

