#include "api_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

api_connection_pool_handle::api_connection_pool_handle()
    : _initialized_state(api_connection_pool_handle::_state_uninitialized),
      _mutex(ft_nullptr), key(), socket(),
      security_mode(api_connection_security_mode::PLAIN),
      has_socket(false), from_pool(false), should_store(false),
      negotiated_http2(false), plain_socket_timed_out(false),
      plain_socket_validated(false)
{
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    return ;
}

api_connection_pool_handle::~api_connection_pool_handle()
{
    if (this->_initialized_state == api_connection_pool_handle::_state_uninitialized)
    {
        pf_printf_fd(2, "api_connection_pool_handle lifecycle error: %s\n",
            "destructor called on uninitialized instance");
        su_abort();
    }
    if (this->_initialized_state == api_connection_pool_handle::_state_initialized)
        (void)this->destroy();
    return ;
}

void api_connection_pool_handle::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "api_connection_pool_handle lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void api_connection_pool_handle::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == api_connection_pool_handle::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int api_connection_pool_handle::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int initialize_result;

    this->abort_if_not_initialized("api_connection_pool_handle::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_result = new_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (initialize_result);
    }
    this->_mutex = new_mutex;
    return (FT_ERR_SUCCESS);
}

int api_connection_pool_handle::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int destroy_result;

    this->abort_if_not_initialized("api_connection_pool_handle::disable_thread_safety");
    mutex_pointer = this->_mutex;
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    this->_mutex = ft_nullptr;
    destroy_result = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_result != FT_ERR_SUCCESS)
        return (destroy_result);
    return (FT_ERR_SUCCESS);
}

bool api_connection_pool_handle::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("api_connection_pool_handle::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int api_connection_pool_handle::initialize() noexcept
{
    int thread_safety_result;

    if (this->_initialized_state == api_connection_pool_handle::_state_initialized)
        this->abort_lifecycle_error("api_connection_pool_handle::initialize",
            "initialize called on initialized instance");
    this->key.clear();
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    this->security_mode = api_connection_security_mode::PLAIN;
    this->has_socket = false;
    this->from_pool = false;
    this->should_store = false;
    this->negotiated_http2 = false;
    this->plain_socket_timed_out = false;
    this->plain_socket_validated = false;
    this->_initialized_state = api_connection_pool_handle::_state_initialized;
    thread_safety_result = this->enable_thread_safety();
    if (thread_safety_result != FT_ERR_SUCCESS)
    {
        this->_initialized_state = api_connection_pool_handle::_state_destroyed;
        return (thread_safety_result);
    }
    return (FT_ERR_SUCCESS);
}

int api_connection_pool_handle::destroy() noexcept
{
    if (this->_initialized_state != api_connection_pool_handle::_state_initialized)
        this->abort_lifecycle_error("api_connection_pool_handle::destroy",
            "destroy called on non-initialized instance");
    this->key.clear();
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    this->security_mode = api_connection_security_mode::PLAIN;
    this->has_socket = false;
    this->from_pool = false;
    this->should_store = false;
    this->negotiated_http2 = false;
    this->plain_socket_timed_out = false;
    this->plain_socket_validated = false;
    (void)this->disable_thread_safety();
    this->_initialized_state = api_connection_pool_handle::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int api_connection_pool_handle::lock(bool *lock_acquired) const
{
    api_connection_pool_handle *mutable_handle;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    mutable_handle = const_cast<api_connection_pool_handle *>(this);
    mutable_handle->abort_if_not_initialized("api_connection_pool_handle::lock");
    if (mutable_handle->_mutex == ft_nullptr)
        return (0);
    if (mutable_handle->_mutex->lock() != FT_ERR_SUCCESS)
        return (-1);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (0);
}

void api_connection_pool_handle::unlock(bool lock_acquired) const
{
    api_connection_pool_handle *mutable_handle;

    if (lock_acquired == false)
        return ;
    mutable_handle = const_cast<api_connection_pool_handle *>(this);
    if (mutable_handle->_mutex == ft_nullptr)
        return ;
    (void)mutable_handle->_mutex->unlock();
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *api_connection_pool_handle::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("api_connection_pool_handle::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
