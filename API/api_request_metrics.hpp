#ifndef API_REQUEST_METRICS_HPP
#define API_REQUEST_METRICS_HPP

#include "../CPP_class/class_string.hpp"
#include "../Observability/observability_networking_metrics.hpp"
#include "../Time/time.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

struct api_request_metrics_guard
{
    uint8_t _initialized_state;
    static const uint8_t _state_uninitialized = 0;
    static const uint8_t _state_destroyed = 1;
    static const uint8_t _state_initialized = 2;
    t_monotonic_time_point _start_time;
    ft_string _endpoint;
    const char *_method;
    const char *_resource;
    size_t _request_bytes;
    char **_result_body;
    int *_status_pointer;
    int *_error_pointer;
    bool _enabled;

    void abort_lifecycle_error(const char *method_name,
        const char *reason) const noexcept
    {
        pf_printf_fd(2, "api_request_metrics_guard lifecycle error: %s: %s\n",
            method_name, reason);
        su_abort();
        return ;
    }

    api_request_metrics_guard() noexcept
        : _initialized_state(_state_uninitialized),
          _start_time(),
          _endpoint(),
          _method(ft_nullptr),
          _resource(ft_nullptr),
          _request_bytes(0),
          _result_body(ft_nullptr),
          _status_pointer(ft_nullptr),
          _error_pointer(ft_nullptr),
          _enabled(false)
    {
        return ;
    }

    api_request_metrics_guard(const api_request_metrics_guard &other) = delete;
    api_request_metrics_guard &operator=(const api_request_metrics_guard &other) = delete;
    api_request_metrics_guard(api_request_metrics_guard &&other) = delete;
    api_request_metrics_guard &operator=(api_request_metrics_guard &&other) = delete;

    int initialize(const char *host, uint16_t port,
        const char *method, const char *resource, size_t request_bytes,
        char **result_body, int *status_pointer, int *error_pointer) noexcept
    {
        if (this->_initialized_state == _state_initialized)
            this->abort_lifecycle_error("api_request_metrics_guard::initialize",
                "called while object is already initialized");
        _start_time = time_monotonic_point_now();
        this->_method = method;
        this->_resource = resource;
        this->_request_bytes = request_bytes;
        this->_result_body = result_body;
        this->_status_pointer = status_pointer;
        this->_error_pointer = error_pointer;
        this->_enabled = true;
        this->_initialized_state = _state_initialized;
        if (this->_endpoint.initialize() != FT_ERR_SUCCESS)
        {
            this->_enabled = false;
            this->_initialized_state = _state_destroyed;
            return (ft_string::last_operation_error());
        }
        if (this->_endpoint.clear() != FT_ERR_SUCCESS)
        {
            this->_enabled = false;
            this->_initialized_state = _state_destroyed;
            return (ft_string::last_operation_error());
        }
        if (host)
            this->_endpoint = host;
        else
            this->_endpoint = "(null)";
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            this->_enabled = false;
            this->_initialized_state = _state_destroyed;
            return (ft_string::last_operation_error());
        }
        if (port != 0)
        {
            char port_string[32];
            int write_result;

            write_result = pf_snprintf(port_string, sizeof(port_string), "%u",
                    static_cast<unsigned int>(port));
            if (write_result < 0)
            {
                this->_enabled = false;
                this->_initialized_state = _state_destroyed;
                return (FT_ERR_IO);
            }
            this->_endpoint.append(':');
            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            {
                this->_enabled = false;
                this->_initialized_state = _state_destroyed;
                return (ft_string::last_operation_error());
            }
            this->_endpoint.append(port_string);
            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            {
                this->_enabled = false;
                this->_initialized_state = _state_destroyed;
                return (ft_string::last_operation_error());
            }
        }
        return (FT_ERR_SUCCESS);
    }

    int destroy() noexcept
    {
        if (this->_initialized_state != _state_initialized)
            return (FT_ERR_INVALID_STATE);
        (void)this->_endpoint.destroy();
        this->_enabled = false;
        this->_initialized_state = _state_destroyed;
        return (FT_ERR_SUCCESS);
    }

    ~api_request_metrics_guard() noexcept
    {
        t_monotonic_time_point finish_time;
        long long duration_ms;
        size_t response_bytes;
        int status_value;
        int error_value;
        ft_networking_observability_sample sample;
        const char *operation_label;

        if (this->_initialized_state != _state_initialized)
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
            sample.success = true;
        else
            sample.success = false;
        observability_networking_metrics_record(sample);
        (void)this->destroy();
        return ;
    }
};

#endif
