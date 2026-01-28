#include "api_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread.hpp"
#include "../Template/move.hpp"

api_connection_pool_handle::api_connection_pool_handle()
        : _error_code(FT_ERR_SUCCESSS), _mutex(ft_nullptr), key(),
        socket(), tls_session(ft_nullptr), tls_context(ft_nullptr),
        security_mode(api_connection_security_mode::PLAIN), has_socket(false),
        from_pool(false), should_store(false), negotiated_http2(false),
        plain_socket_timed_out(false), plain_socket_validated(false)
{
    if (this->initialize_thread_safety() != 0)
    {
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

api_connection_pool_handle::api_connection_pool_handle(
        api_connection_pool_handle &&other)
        : _error_code(FT_ERR_SUCCESSS), _mutex(ft_nullptr), key(),
        socket(), tls_session(ft_nullptr), tls_context(ft_nullptr),
        security_mode(api_connection_security_mode::PLAIN), has_socket(false),
        from_pool(false), should_store(false), negotiated_http2(false),
        plain_socket_timed_out(false), plain_socket_validated(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(other.get_error());
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
    this->plain_socket_timed_out = other.plain_socket_timed_out;
    this->plain_socket_validated = other.plain_socket_validated;
    other.unlock_internal(other_lock_acquired);
    this->_mutex = other._mutex;
    other._mutex = ft_nullptr;
    other.tls_session = ft_nullptr;
    other.tls_context = ft_nullptr;
    other.has_socket = false;
    other.from_pool = false;
    other.should_store = false;
    other.negotiated_http2 = false;
    other.plain_socket_timed_out = false;
    other.plain_socket_validated = false;
    other.set_error(FT_ERR_SUCCESSS);
    this->set_error(FT_ERR_SUCCESSS);
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
    this->plain_socket_timed_out = false;
    this->plain_socket_validated = false;
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

api_connection_pool_handle &api_connection_pool_handle::operator=(
        api_connection_pool_handle &&other)
{
    bool this_lock_acquired;
    bool other_lock_acquired;

    if (this == &other)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(other.get_error());
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
    this->plain_socket_timed_out = other.plain_socket_timed_out;
    this->plain_socket_validated = other.plain_socket_validated;
    this->unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    if (this->_mutex != ft_nullptr)
        this->teardown_thread_safety();
    this->_mutex = other._mutex;
    other._mutex = ft_nullptr;
    other.tls_session = ft_nullptr;
    other.tls_context = ft_nullptr;
    other.has_socket = false;
    other.from_pool = false;
    other.should_store = false;
    other.negotiated_http2 = false;
    other.plain_socket_timed_out = false;
    other.plain_socket_validated = false;
    other.set_error(FT_ERR_SUCCESSS);
    this->set_error(FT_ERR_SUCCESSS);
    return (*this);
}

int api_connection_pool_handle::initialize_thread_safety()
{
    void *allocated_memory;
    pt_mutex *mutex_pointer;

    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    allocated_memory = cma_malloc(sizeof(pt_mutex));
    if (allocated_memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(allocated_memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error_code;

        mutex_error_code = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(allocated_memory);
        this->set_error(mutex_error_code);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

int api_connection_pool_handle::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    return (result);
}

void api_connection_pool_handle::unlock(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
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

thread_local ft_operation_error_stack api_connection_pool_handle::_operation_errors = {{}, {}, 0};

void api_connection_pool_handle::record_operation_error_unlocked(int error_code)
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(api_connection_pool_handle::_operation_errors,
            error_code, operation_id);
    return ;
}

void api_connection_pool_handle::set_error(int error) const
{

    this->_error_code = error;
    api_connection_pool_handle::record_operation_error_unlocked(error);
    return ;
}

int api_connection_pool_handle::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

void api_connection_pool_handle::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_mutex->get_error());
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
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
