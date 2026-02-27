#include "api.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

api_streaming_handler::api_streaming_handler() noexcept
    : _initialized_state(api_streaming_handler::_state_uninitialized),
      _headers_callback(ft_nullptr), _body_callback(ft_nullptr),
      _user_data(ft_nullptr), _mutex(ft_nullptr)
{
    return ;
}

api_streaming_handler::~api_streaming_handler()
{
    if (this->_initialized_state == api_streaming_handler::_state_initialized)
        (void)this->destroy();
    return ;
}

void api_streaming_handler::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "api_streaming_handler lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void api_streaming_handler::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == api_streaming_handler::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int api_streaming_handler::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int initialize_result;

    this->abort_if_not_initialized("api_streaming_handler::enable_thread_safety");
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

int api_streaming_handler::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int destroy_result;

    this->abort_if_not_initialized("api_streaming_handler::disable_thread_safety");
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

bool api_streaming_handler::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int api_streaming_handler::initialize() noexcept
{
    if (this->_initialized_state == api_streaming_handler::_state_initialized)
        this->abort_lifecycle_error("api_streaming_handler::initialize",
            "initialize called on initialized instance");
    this->_headers_callback = ft_nullptr;
    this->_body_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_initialized_state = api_streaming_handler::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int api_streaming_handler::destroy() noexcept
{
    if (this->_initialized_state != api_streaming_handler::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    this->_headers_callback = ft_nullptr;
    this->_body_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    (void)this->disable_thread_safety();
    this->_initialized_state = api_streaming_handler::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

void api_streaming_handler::reset() noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_headers_callback = ft_nullptr;
    this->_body_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_streaming_handler::set_headers_callback(
    api_stream_headers_callback callback) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_headers_callback = callback;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_streaming_handler::set_body_callback(
    api_stream_body_callback callback) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_body_callback = callback;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

void api_streaming_handler::set_user_data(void *user_data) noexcept
{
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return ;
    this->_user_data = user_data;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

bool api_streaming_handler::invoke_headers_callback(int status_code,
    const char *headers) const noexcept
{
    api_stream_headers_callback callback;
    void *user_data;

    callback = ft_nullptr;
    user_data = ft_nullptr;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (false);
    callback = this->_headers_callback;
    user_data = this->_user_data;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (callback == ft_nullptr)
        return (true);
    callback(status_code, headers, user_data);
    return (true);
}

bool api_streaming_handler::invoke_body_callback(const char *chunk_data,
    size_t chunk_size, bool is_final_chunk, bool &should_continue) const noexcept
{
    api_stream_body_callback callback;
    void *user_data;

    callback = ft_nullptr;
    user_data = ft_nullptr;
    should_continue = true;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (false);
    callback = this->_body_callback;
    user_data = this->_user_data;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (callback == ft_nullptr)
        return (true);
    should_continue = callback(chunk_data, chunk_size, is_final_chunk, user_data);
    return (true);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *api_streaming_handler::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("api_streaming_handler::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
