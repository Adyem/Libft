#ifndef API_REQUEST_METRICS_HPP
#define API_REQUEST_METRICS_HPP

#include "../CPP_class/class_string.hpp"
#include "../Observability/observability_networking_metrics.hpp"
#include "../Time/time.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"

struct api_request_metrics_guard
{
    t_monotonic_time_point _start_time;
    ft_string _endpoint;
    const char *_method;
    const char *_resource;
    size_t _request_bytes;
    char **_result_body;
    int *_status_pointer;
    int *_error_pointer;
    bool _enabled;

    api_request_metrics_guard(const char *host, uint16_t port,
        const char *method, const char *resource, size_t request_bytes,
        char **result_body, int *status_pointer, int *error_pointer) noexcept
    {
        _start_time = time_monotonic_point_now();
        _endpoint.clear();
        _method = method;
        _resource = resource;
        _request_bytes = request_bytes;
        _result_body = result_body;
        _status_pointer = status_pointer;
        _error_pointer = error_pointer;
        _enabled = true;
        if (host)
            _endpoint = host;
        else
            _endpoint = "(null)";
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            _enabled = false;
            return ;
        }
        if (port != 0)
        {
            char *port_string;

            port_string = cma_itoa(static_cast<int>(port));
            if (!port_string)
            {
                _enabled = false;
                return ;
            }
            _endpoint.append(':');
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                _enabled = false;
                cma_free(port_string);
                return ;
            }
            _endpoint.append(port_string);
            cma_free(port_string);
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                _enabled = false;
                return ;
            }
        }
        return ;
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

        if (!_enabled)
            return ;
        finish_time = time_monotonic_point_now();
        duration_ms = time_monotonic_point_diff_ms(_start_time, finish_time);
        if (duration_ms < 0)
            duration_ms = 0;
        response_bytes = 0;
        if (_result_body && *_result_body)
            response_bytes = ft_strlen(*_result_body);
        status_value = -1;
        if (_status_pointer)
            status_value = *_status_pointer;
        error_value = FT_ERR_SUCCESSS;
        if (_error_pointer)
            error_value = *_error_pointer;
        sample.labels.component = "api";
        operation_label = _method;
        if (!operation_label)
            operation_label = "UNKNOWN";
        sample.labels.operation = operation_label;
        sample.labels.target = _endpoint.c_str();
        sample.labels.resource = _resource;
        if (!sample.labels.resource)
            sample.labels.resource = "/";
        sample.duration_ms = duration_ms;
        sample.request_bytes = _request_bytes;
        sample.response_bytes = response_bytes;
        sample.status_code = status_value;
        sample.error_code = error_value;
        sample.error_tag = ft_nullptr;
        if (error_value == FT_ERR_SUCCESSS)
            sample.success = true;
        else
            sample.success = false;
        observability_networking_metrics_record(sample);
        return ;
    }
};

#endif
