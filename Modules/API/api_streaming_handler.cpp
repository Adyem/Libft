#include "api.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Basic/basic.hpp"

api_streaming_handler::api_streaming_handler() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _headers_callback(ft_nullptr), _body_callback(ft_nullptr),
      _user_data(ft_nullptr), _mutex(ft_nullptr)
{
    return ;
}

api_streaming_handler::~api_streaming_handler()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

void api_streaming_handler::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

void api_streaming_handler::abort_if_not_initialised(const char *method_name) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, method_name);
    return ;
}

int32_t api_streaming_handler::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;

    this->abort_if_not_initialised("api_streaming_handler::enable_thread_safety");
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

int32_t api_streaming_handler::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;

    this->abort_if_not_initialised("api_streaming_handler::disable_thread_safety");
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

ft_bool api_streaming_handler::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t api_streaming_handler::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        this->abort_lifecycle_error("api_streaming_handler::initialize",
            "initialize called on initialised instance");
    this->_headers_callback = ft_nullptr;
    this->_body_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_streaming_handler::initialize(
    const api_streaming_handler &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_streaming_handler::initialize(copy)",
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
    this->_headers_callback = other._headers_callback;
    this->_body_callback = other._body_callback;
    this->_user_data = other._user_data;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_streaming_handler::initialize(
    api_streaming_handler &&other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_streaming_handler::initialize(move)",
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
    this->_headers_callback = other._headers_callback;
    this->_body_callback = other._body_callback;
    this->_user_data = other._user_data;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._headers_callback = ft_nullptr;
    other._body_callback = ft_nullptr;
    other._user_data = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t api_streaming_handler::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    (void)this->disable_thread_safety();
    this->_headers_callback = ft_nullptr;
    this->_body_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

uint32_t api_streaming_handler::move(api_streaming_handler &other) noexcept
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
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

ft_bool api_streaming_handler::invoke_headers_callback(int32_t status_code,
    const char *headers) const noexcept
{
    api_stream_headers_callback callback;
    void *user_data;

    callback = ft_nullptr;
    user_data = ft_nullptr;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    callback = this->_headers_callback;
    user_data = this->_user_data;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (callback == ft_nullptr)
        return (FT_TRUE);
    callback(status_code, headers, user_data);
    return (FT_TRUE);
}

ft_bool api_streaming_handler::invoke_body_callback(const char *chunk_data,
    ft_size_t chunk_size, ft_bool is_final_chunk, ft_bool &should_continue) const noexcept
{
    api_stream_body_callback callback;
    void *user_data;

    callback = ft_nullptr;
    user_data = ft_nullptr;
    should_continue = FT_TRUE;
    if (pt_recursive_mutex_lock_if_not_null(this->_mutex) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    callback = this->_body_callback;
    user_data = this->_user_data;
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (callback == ft_nullptr)
        return (FT_TRUE);
    should_continue = callback(chunk_data, chunk_size, is_final_chunk, user_data);
    return (FT_TRUE);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *api_streaming_handler::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialised("api_streaming_handler::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
