#include "api_internal.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>

api_connection_pool_handle::api_connection_pool_handle()
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr), key(), socket(),
      security_mode(api_connection_security_mode::PLAIN),
      has_socket(FT_FALSE), from_pool(FT_FALSE), should_store(FT_FALSE),
      negotiated_http2(FT_FALSE), plain_socket_timed_out(FT_FALSE),
      plain_socket_validated(FT_FALSE)
{
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    return ;
}

api_connection_pool_handle::api_connection_pool_handle(
    const api_connection_pool_handle &other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr), key(), socket(),
      security_mode(api_connection_security_mode::PLAIN),
      has_socket(FT_FALSE), from_pool(FT_FALSE), should_store(FT_FALSE),
      negotiated_http2(FT_FALSE), plain_socket_timed_out(FT_FALSE),
      plain_socket_validated(FT_FALSE)
{
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "api_connection_pool_handle::api_connection_pool_handle(copy)",
            "source is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

api_connection_pool_handle::api_connection_pool_handle(
    api_connection_pool_handle &&other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _mutex(ft_nullptr), key(), socket(),
      security_mode(api_connection_security_mode::PLAIN),
      has_socket(FT_FALSE), from_pool(FT_FALSE), should_store(FT_FALSE),
      negotiated_http2(FT_FALSE), plain_socket_timed_out(FT_FALSE),
      plain_socket_validated(FT_FALSE)
{
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "api_connection_pool_handle::api_connection_pool_handle(move)",
            "source is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return ;
    }
    if (this->initialize(ft_move(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

api_connection_pool_handle::~api_connection_pool_handle()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

void api_connection_pool_handle::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void api_connection_pool_handle::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t api_connection_pool_handle::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;

    this->abort_if_not_initialised("api_connection_pool_handle::enable_thread_safety");
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

int32_t api_connection_pool_handle::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;

    this->abort_if_not_initialised("api_connection_pool_handle::disable_thread_safety");
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

ft_bool api_connection_pool_handle::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t api_connection_pool_handle::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        this->abort_lifecycle_error("api_connection_pool_handle::initialize",
            "initialize called on initialised instance");
    int32_t key_init_result = this->key.initialize();
    if (key_init_result != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (key_init_result);
    }
    this->key.clear();
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    this->security_mode = api_connection_security_mode::PLAIN;
    this->has_socket = FT_FALSE;
    this->from_pool = FT_FALSE;
    this->should_store = FT_FALSE;
    this->negotiated_http2 = FT_FALSE;
    this->plain_socket_timed_out = FT_FALSE;
    this->plain_socket_validated = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_connection_pool_handle::initialize(
    const api_connection_pool_handle &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_connection_pool_handle::initialize(copy)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (destroy_result);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->key = other.key;
#if NETWORKING_HAS_OPENSSL
    this->tls_session = other.tls_session;
    this->tls_context = other.tls_context;
#endif
    this->security_mode = other.security_mode;
    this->has_socket = other.has_socket;
    this->from_pool = other.from_pool;
    this->should_store = other.should_store;
    this->negotiated_http2 = other.negotiated_http2;
    this->plain_socket_timed_out = other.plain_socket_timed_out;
    this->plain_socket_validated = other.plain_socket_validated;
    return (FT_ERR_SUCCESS);
}

int32_t api_connection_pool_handle::initialize(
    api_connection_pool_handle &&other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_connection_pool_handle::initialize(move)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (destroy_result);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize(other) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    other.key.clear();
#if NETWORKING_HAS_OPENSSL
    other.tls_session = ft_nullptr;
    other.tls_context = ft_nullptr;
#endif
    other.security_mode = api_connection_security_mode::PLAIN;
    other.has_socket = FT_FALSE;
    other.from_pool = FT_FALSE;
    other.should_store = FT_FALSE;
    other.negotiated_http2 = FT_FALSE;
    other.plain_socket_timed_out = FT_FALSE;
    other.plain_socket_validated = FT_FALSE;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t api_connection_pool_handle::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    (void)this->disable_thread_safety();
    this->key.clear();
#if NETWORKING_HAS_OPENSSL
    this->tls_session = ft_nullptr;
    this->tls_context = ft_nullptr;
#endif
    this->security_mode = api_connection_security_mode::PLAIN;
    this->has_socket = FT_FALSE;
    this->from_pool = FT_FALSE;
    this->should_store = FT_FALSE;
    this->negotiated_http2 = FT_FALSE;
    this->plain_socket_timed_out = FT_FALSE;
    this->plain_socket_validated = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t api_connection_pool_handle::move(
    api_connection_pool_handle &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
}

int32_t api_connection_pool_handle::lock(ft_bool *lock_acquired) const
{
    api_connection_pool_handle *mutable_handle;
    int32_t lock_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    mutable_handle = const_cast<api_connection_pool_handle *>(this);
    mutable_handle->abort_if_not_initialised("api_connection_pool_handle::lock");
    lock_result = pt_recursive_mutex_lock_if_not_null(mutable_handle->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void api_connection_pool_handle::unlock(ft_bool lock_acquired) const
{
    api_connection_pool_handle *mutable_handle;

    if (lock_acquired == FT_FALSE)
        return ;
    mutable_handle = const_cast<api_connection_pool_handle *>(this);
    (void)pt_recursive_mutex_unlock_if_not_null(mutable_handle->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *api_connection_pool_handle::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialised("api_connection_pool_handle::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
