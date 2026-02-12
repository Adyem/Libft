#include "api.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Template/move.hpp"
#include "../PThread/pthread.hpp"

thread_local ft_operation_error_stack api_streaming_handler::_operation_errors = {{}, {}, 0};

static void api_streaming_handler_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

void api_streaming_handler::record_operation_error_unlocked(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(api_streaming_handler::_operation_errors,
            error_code, operation_id);
    return ;
}

void api_streaming_handler::set_error(int error) const noexcept
{

    this->_error_code = error;
    api_streaming_handler::record_operation_error_unlocked(error);
    return ;
}

int api_streaming_handler::lock_pair(const api_streaming_handler &first,
    const api_streaming_handler &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const api_streaming_handler *ordered_first;
    const api_streaming_handler *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESS)
        {
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const api_streaming_handler *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
        {
            ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

            if (lower_guard.get_error() != FT_ERR_SUCCESS)
            {
                return (lower_guard.get_error());
            }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ERR_SUCCESS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            return (FT_ERR_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        api_streaming_handler_sleep_backoff();
    }
}

api_streaming_handler::api_streaming_handler() noexcept
    : _headers_callback(ft_nullptr), _body_callback(ft_nullptr),
      _user_data(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

api_streaming_handler::api_streaming_handler(
    const api_streaming_handler &other) noexcept
    : _headers_callback(ft_nullptr), _body_callback(ft_nullptr),
      _user_data(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    {
        ft_unique_lock<pt_mutex> other_guard(other._mutex);

        if (other_guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(other_guard.get_error());
            return ;
        }
        this->_headers_callback = other._headers_callback;
        this->_body_callback = other._body_callback;
        this->_user_data = other._user_data;
        this->_error_code = other._error_code;
        this->set_error(other._error_code);
    }
    return ;
}

api_streaming_handler &api_streaming_handler::operator=(
    const api_streaming_handler &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = api_streaming_handler::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_headers_callback = other._headers_callback;
    this->_body_callback = other._body_callback;
    this->_user_data = other._user_data;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    return (*this);
}

api_streaming_handler::api_streaming_handler(
    api_streaming_handler &&other) noexcept
    : _headers_callback(ft_nullptr), _body_callback(ft_nullptr),
      _user_data(ft_nullptr), _error_code(FT_ERR_SUCCESS), _mutex()
{
    {
        ft_unique_lock<pt_mutex> other_guard(other._mutex);

        if (other_guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(other_guard.get_error());
            return ;
        }
        this->_headers_callback = other._headers_callback;
        this->_body_callback = other._body_callback;
        this->_user_data = other._user_data;
        this->_error_code = other._error_code;
        other._headers_callback = ft_nullptr;
        other._body_callback = ft_nullptr;
        other._user_data = ft_nullptr;
        other._error_code = FT_ERR_SUCCESS;
        this->set_error(this->_error_code);
        other.set_error(FT_ERR_SUCCESS);
    }
    return ;
}

api_streaming_handler &api_streaming_handler::operator=(
    api_streaming_handler &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = api_streaming_handler::lock_pair(*this, other,
            this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_headers_callback = other._headers_callback;
    this->_body_callback = other._body_callback;
    this->_user_data = other._user_data;
    this->_error_code = other._error_code;
    other._headers_callback = ft_nullptr;
    other._body_callback = ft_nullptr;
    other._user_data = ft_nullptr;
    other._error_code = FT_ERR_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(FT_ERR_SUCCESS);
    return (*this);
}

api_streaming_handler::~api_streaming_handler()
{
    return ;
}

void api_streaming_handler::reset() noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_headers_callback = ft_nullptr;
        this->_body_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_error_code = FT_ERR_SUCCESS;
        this->set_error(FT_ERR_SUCCESS);
    }
    return ;
}

void api_streaming_handler::set_headers_callback(
    api_stream_headers_callback callback) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_headers_callback = callback;
        this->set_error(FT_ERR_SUCCESS);
    }
    return ;
}

void api_streaming_handler::set_body_callback(
    api_stream_body_callback callback) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_body_callback = callback;
        this->set_error(FT_ERR_SUCCESS);
    }
    return ;
}

void api_streaming_handler::set_user_data(void *user_data) noexcept
{
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        this->_user_data = user_data;
        this->set_error(FT_ERR_SUCCESS);
    }
    return ;
}

bool api_streaming_handler::invoke_headers_callback(int status_code,
    const char *headers) const noexcept
{
    api_stream_headers_callback callback;
    void *user_data;
    callback = ft_nullptr;
    user_data = ft_nullptr;
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (false);
        }
        callback = this->_headers_callback;
        user_data = this->_user_data;
        this->set_error(FT_ERR_SUCCESS);
    }
    if (!callback)
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
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);

        if (guard.get_error() != FT_ERR_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (false);
        }
        callback = this->_body_callback;
        user_data = this->_user_data;
        this->set_error(FT_ERR_SUCCESS);
    }
    if (!callback)
        return (true);
    should_continue = callback(chunk_data, chunk_size, is_final_chunk,
            user_data);
    return (true);
}

int api_streaming_handler::get_error() const noexcept
{
    return (this->_error_code);
}

const char *api_streaming_handler::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
