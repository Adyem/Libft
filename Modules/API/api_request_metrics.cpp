#include "api_request_metrics.hpp"
#include "../Template/move.hpp"

void api_request_metrics_guard::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    errno_abort_lifecycle(this->_initialised_state, method_name, reason);
    return ;
}

api_request_metrics_guard::api_request_metrics_guard() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _start_time(),
      _endpoint(),
      _method(ft_nullptr),
      _resource(ft_nullptr),
      _request_bytes(0),
      _result_body(ft_nullptr),
      _status_pointer(ft_nullptr),
      _error_pointer(ft_nullptr),
      _enabled(FT_FALSE)
{
    return ;
}

api_request_metrics_guard::api_request_metrics_guard(
    const api_request_metrics_guard &other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _start_time(),
      _endpoint(),
      _method(ft_nullptr),
      _resource(ft_nullptr),
      _request_bytes(0),
      _result_body(ft_nullptr),
      _status_pointer(ft_nullptr),
      _error_pointer(ft_nullptr),
      _enabled(FT_FALSE)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_request_metrics_guard::api_request_metrics_guard(copy)",
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

api_request_metrics_guard::api_request_metrics_guard(
    api_request_metrics_guard &&other) noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED),
      _start_time(),
      _endpoint(),
      _method(ft_nullptr),
      _resource(ft_nullptr),
      _request_bytes(0),
      _result_body(ft_nullptr),
      _status_pointer(ft_nullptr),
      _error_pointer(ft_nullptr),
      _enabled(FT_FALSE)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_request_metrics_guard::api_request_metrics_guard(move)",
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

int32_t api_request_metrics_guard::initialize(const char *host, uint16_t port,
    const char *method, const char *resource, ft_size_t request_bytes,
    char **result_body, int32_t *status_pointer, int32_t *error_pointer) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        this->abort_lifecycle_error("api_request_metrics_guard::initialize",
            "called while object is already initialised");
    this->_start_time = time_monotonic_point_now();
    this->_method = method;
    this->_resource = resource;
    this->_request_bytes = request_bytes;
    this->_result_body = result_body;
    this->_status_pointer = status_pointer;
    this->_error_pointer = error_pointer;
    this->_enabled = FT_TRUE;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (this->_endpoint.initialize() != FT_ERR_SUCCESS)
    {
        this->_enabled = FT_FALSE;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->_endpoint.get_error());
    }
    if (this->_endpoint.clear() != FT_ERR_SUCCESS)
    {
        this->_enabled = FT_FALSE;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->_endpoint.get_error());
    }
    if (host)
        this->_endpoint = host;
    else
        this->_endpoint = "(null)";
    if (this->_endpoint.get_error() != FT_ERR_SUCCESS)
    {
        this->_enabled = FT_FALSE;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->_endpoint.get_error());
    }
    if (port != 0)
    {
        char port_string[32];
        int32_t write_result;

        write_result = pf_snprintf(port_string, sizeof(port_string), "%u",
                static_cast<uint32_t>(port));
        if (write_result < 0)
        {
            this->_enabled = FT_FALSE;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (FT_ERR_IO);
        }
        this->_endpoint.append(':');
        if (this->_endpoint.get_error() != FT_ERR_SUCCESS)
        {
            this->_enabled = FT_FALSE;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_endpoint.get_error());
        }
        this->_endpoint.append(port_string);
        if (this->_endpoint.get_error() != FT_ERR_SUCCESS)
        {
            this->_enabled = FT_FALSE;
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (this->_endpoint.get_error());
        }
    }
    return (FT_ERR_SUCCESS);
}

int32_t api_request_metrics_guard::initialize(
    const api_request_metrics_guard &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_request_metrics_guard::initialize(copy)",
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
    this->_start_time = other._start_time;
    this->_method = other._method;
    this->_resource = other._resource;
    this->_request_bytes = other._request_bytes;
    this->_result_body = other._result_body;
    this->_status_pointer = other._status_pointer;
    this->_error_pointer = other._error_pointer;
    this->_enabled = other._enabled;
    if (this->_endpoint.initialize(other._endpoint.c_str()) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->_endpoint.get_error());
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t api_request_metrics_guard::initialize(
    api_request_metrics_guard &&other) noexcept
{
    int32_t initialize_result;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        this->abort_lifecycle_error("api_request_metrics_guard::initialize(move)",
            "source is uninitialised");
    initialize_result = this->initialize(static_cast<const api_request_metrics_guard &>(other));
    if (initialize_result != FT_ERR_SUCCESS)
        return (initialize_result);
    other._enabled = FT_FALSE;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

int32_t api_request_metrics_guard::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    (void)this->_endpoint.destroy();
    this->_enabled = FT_FALSE;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

api_request_metrics_guard::~api_request_metrics_guard() noexcept
{
    t_monotonic_time_point finish_time;
    int64_t duration_ms;
    ft_size_t response_bytes;
    int32_t status_value;
    int32_t error_value;
    ft_networking_observability_sample sample;
    const char *operation_label;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    if (!this->_enabled)
    {
        (void)this->destroy();
        return ;
    }
    finish_time = time_monotonic_point_now();
    duration_ms = time_monotonic_point_diff_ms(this->_start_time, finish_time);
    if (duration_ms < 0)
        duration_ms = 0;
    response_bytes = 0;
    if (this->_result_body && *this->_result_body)
        response_bytes = ft_strlen(*this->_result_body);
    status_value = -1;
    if (this->_status_pointer)
        status_value = *this->_status_pointer;
    error_value = FT_ERR_SUCCESS;
    if (this->_error_pointer)
        error_value = *this->_error_pointer;
    sample.labels.component = "api";
    operation_label = this->_method;
    if (!operation_label)
        operation_label = "UNKNOWN";
    sample.labels.operation = operation_label;
    sample.labels.target = this->_endpoint.c_str();
    sample.labels.resource = this->_resource;
    if (!sample.labels.resource)
        sample.labels.resource = "/";
    sample.duration_ms = duration_ms;
    sample.request_bytes = this->_request_bytes;
    sample.response_bytes = response_bytes;
    sample.status_code = status_value;
    sample.error_code = error_value;
    sample.error_tag = ft_nullptr;
    if (error_value == FT_ERR_SUCCESS)
        sample.success = FT_TRUE;
    else
        sample.success = FT_FALSE;
    observability_networking_metrics_record(sample);
    (void)this->destroy();
    return ;
}
