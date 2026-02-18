#include "api.hpp"
#include "api_internal.hpp"
#include "api_http_internal.hpp"
#include "api_request_metrics.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../Time/time.hpp"
#include <errno.h>
#include <utility>
#include "../Template/move.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#endif

static int api_request_capture_network_error()
{
    int system_error;

#ifdef _WIN32
    system_error = WSAGetLastError();
#else
    system_error = errno;
#endif
    if (system_error == 0)
        return (FT_ERR_SUCCESS);
    return (cmp_map_system_error_to_ft(system_error));
}

static int api_request_assign_resolve_error(int mapped_error)
{
    if (mapped_error != FT_ERR_SUCCESS)
        return (mapped_error);
    return (FT_ERR_SOCKET_RESOLVE_FAILED);
}

static void api_request_copy_ip(char destination[46], const char *source)
{
    destination[0] = '\0';
    if (source == ft_nullptr)
        return ;
    ft_strncpy(destination, source, 45);
    destination[45] = '\0';
    return ;
}

static api_request_wait_until_ready_hook g_api_request_wait_hook = ft_nullptr;

void api_request_set_downgrade_wait_hook(
    api_request_wait_until_ready_hook hook)
{
    g_api_request_wait_hook = hook;
    return ;
}

bool api_request(const char *ip, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout, bool enable_http2,
    bool *used_http2, const api_retry_policy *retry_policy)
{
    bool http2_used_local;
    bool request_success;
    int error_code = FT_ERR_SUCCESS;

    if (used_http2)
        *used_http2 = false;
    if (!streaming_handler)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    http2_used_local = false;
    if (enable_http2)
    {
        request_success = api_request_stream_http2(ip, port, method, path,
                streaming_handler, payload, headers, timeout,
                &http2_used_local, retry_policy);
        if (!request_success)
        {
            error_code = FT_ERR_IO;
            if (error_code == FT_ERR_SUCCESS)
                error_code = FT_ERR_IO;
            return (false);
        }
        if (used_http2)
            *used_http2 = http2_used_local;
        return (true);
    }
    request_success = api_request_stream(ip, port, method, path,
            streaming_handler, payload, headers, timeout, retry_policy);
    if (!request_success)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (false);
    }
    if (used_http2)
        *used_http2 = false;
    return (true);
}

bool api_request_stream(const char *ip, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout,
    const api_retry_policy *retry_policy)
{
    if (ft_log_get_api_logging())
    {
        const char *log_ip = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";

        if (ip)
            log_ip = ip;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_stream %s:%u %s %s", log_ip, port,
            log_method, log_path);
    }
    int error_code = FT_ERR_SUCCESS;
    if (!streaming_handler)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_stream)
    {
        bool hook_result = hooks->request_stream(ip, port, method, path,
                streaming_handler, payload, headers, timeout, retry_policy,
                hooks->user_data);
        if (!hook_result)
            error_code = FT_ERR_IO;
        return (hook_result);
    }

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;

    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (!pooled_connection)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (false);
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            if (api_is_configuration_socket_error(socket_setup_error))
                error_code = socket_setup_error;
            else
                error_code = FT_ERR_SOCKET_CONNECT_FAILED;
            api_retry_circuit_record_failure(connection_handle, retry_policy);
            return (false);
        }
        connection_handle.has_socket = true;
    }
    struct api_connection_return_guard
    {
        api_connection_pool_handle *handle;
        bool success;
        api_connection_return_guard(api_connection_pool_handle &value)
        {
            handle = &value;
            success = false;
            return ;
        }
        ~api_connection_return_guard()
        {
            if (!handle)
                return ;
            if (success)
            {
                api_connection_pool_mark_idle(*handle);
                return ;
            }
            api_connection_pool_evict(*handle);
            return ;
        }
        void set_success(void)
        {
            success = true;
            return ;
        }
    } connection_guard(connection_handle);
    if (!connection_handle.has_socket)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    bool result;

    result = api_http_execute_plain_streaming(connection_handle, method, path,
            ip, payload, headers, timeout, ip, port, retry_policy,
            streaming_handler, error_code);
    if (!result)
        return (false);
    connection_guard.set_success();
    return (true);
}

bool api_request_stream_http2(const char *ip, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy)
{
    if (used_http2)
        *used_http2 = false;
    if (ft_log_get_api_logging())
    {
        const char *log_ip = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";

        if (ip)
            log_ip = ip;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_stream_http2 %s:%u %s %s", log_ip, port,
            log_method, log_path);
    }
    int error_code = FT_ERR_SUCCESS;
    if (!streaming_handler)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_stream_http2)
    {
        bool hook_result = hooks->request_stream_http2(ip, port, method, path,
                streaming_handler, payload, headers, timeout, used_http2,
                retry_policy, hooks->user_data);
        if (!hook_result)
            error_code = FT_ERR_IO;
        return (hook_result);
    }

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;

    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (!pooled_connection)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (false);
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            if (api_is_configuration_socket_error(socket_setup_error))
                error_code = socket_setup_error;
            else
                error_code = FT_ERR_SOCKET_CONNECT_FAILED;
            api_retry_circuit_record_failure(connection_handle, retry_policy);
            return (false);
        }
        connection_handle.has_socket = true;
    }
    struct api_connection_return_guard
    {
        api_connection_pool_handle *handle;
        bool success;
        api_connection_return_guard(api_connection_pool_handle &value)
        {
            handle = &value;
            success = false;
            return ;
        }
        ~api_connection_return_guard()
        {
            if (!handle)
                return ;
            if (success)
            {
                api_connection_pool_mark_idle(*handle);
                return ;
            }
            api_connection_pool_evict(*handle);
            return ;
        }
        void set_success(void)
        {
            success = true;
            return ;
        }
    } connection_guard(connection_handle);
    if (!connection_handle.has_socket)
    {
        error_code = FT_ERR_IO;
        return (false);
    }
    bool http2_used_local;
    bool result;

    http2_used_local = false;
    result = api_http_execute_plain_http2_streaming(connection_handle, method,
            path, ip, payload, headers, timeout, ip, port, retry_policy,
            streaming_handler, http2_used_local, error_code);
    if (!result)
    {
        error_code = FT_ERR_SUCCESS;
        result = api_http_execute_plain_streaming(connection_handle, method,
                path, ip, payload, headers, timeout, ip, port, retry_policy,
                streaming_handler, error_code);
        if (!result)
            return (false);
        http2_used_local = false;
    }
    connection_guard.set_success();
    if (used_http2)
        *used_http2 = http2_used_local;
    return (true);
}

char *api_request_string(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (ft_log_get_api_logging())
    {
        const char *log_ip = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (ip)
            log_ip = ip;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_string %s:%u %s %s",
            log_ip, port, log_method, log_path);
    }
    const api_transport_hooks *hooks;
    int error_code = FT_ERR_SUCCESS;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_string)
    {
        char *hook_result = hooks->request_string(ip, port, method, path, payload,
                headers, status, timeout, retry_policy, hooks->user_data);
        if (!hook_result)
            error_code = FT_ERR_IO;
        return (hook_result);
    }
    size_t metrics_request_bytes;
    char *metrics_payload_string;

    metrics_request_bytes = 0;
    if (method)
        metrics_request_bytes += ft_strlen(method);
    if (path)
        metrics_request_bytes += ft_strlen(path);
    if (headers && headers[0])
        metrics_request_bytes += ft_strlen(headers);
    if (ip)
        metrics_request_bytes += ft_strlen(ip);
    metrics_payload_string = ft_nullptr;
    if (payload)
        metrics_payload_string = json_write_to_string(payload);
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard(ip, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;

    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (!pooled_connection)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (ft_nullptr);
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            if (api_is_configuration_socket_error(socket_setup_error))
                error_code = socket_setup_error;
            else
                error_code = FT_ERR_SOCKET_CONNECT_FAILED;
            api_retry_circuit_record_failure(connection_handle, retry_policy);
            return (ft_nullptr);
        }
        connection_handle.has_socket = true;
    }
    struct api_connection_return_guard
    {
        api_connection_pool_handle *handle;
        bool success;
        api_connection_return_guard(api_connection_pool_handle &value)
        {
            handle = &value;
            success = false;
            return ;
        }
        ~api_connection_return_guard()
        {
            if (!handle)
                return ;
            if (success)
            {
                api_connection_pool_mark_idle(*handle);
                return ;
            }
            api_connection_pool_evict(*handle);
            return ;
        }
        void set_success(void)
        {
            success = true;
            return ;
        }
    } connection_guard(connection_handle);
    if (!connection_handle.has_socket)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    metrics_result_body = api_http_execute_plain(connection_handle, method,
            path, ip, payload, headers, status, timeout, ip, port,
            retry_policy, error_code);
    if (!metrics_result_body)
        return (ft_nullptr);
    connection_guard.set_success();
    return (metrics_result_body);
}

char *api_request_string_http2(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy)
{
    if (used_http2)
        *used_http2 = false;
    if (ft_log_get_api_logging())
    {
        const char *log_ip = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (ip)
            log_ip = ip;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_string_http2 %s:%u %s %s",
            log_ip, port, log_method, log_path);
    }
    const api_transport_hooks *hooks;
    int error_code = FT_ERR_SUCCESS;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_string_http2)
    {
        char *hook_result = hooks->request_string_http2(ip, port, method, path,
                payload, headers, status, timeout, used_http2, retry_policy,
                hooks->user_data);
        if (!hook_result)
            error_code = FT_ERR_IO;
        return (hook_result);
    }
    size_t metrics_request_bytes;
    char *metrics_payload_string;

    metrics_request_bytes = 0;
    if (method)
        metrics_request_bytes += ft_strlen(method);
    if (path)
        metrics_request_bytes += ft_strlen(path);
    if (headers && headers[0])
        metrics_request_bytes += ft_strlen(headers);
    if (ip)
        metrics_request_bytes += ft_strlen(ip);
    metrics_payload_string = ft_nullptr;
    if (payload)
        metrics_payload_string = json_write_to_string(payload);
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard(ip, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    bool downgrade_due_to_connect_failure;

    downgrade_due_to_connect_failure = false;
    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (!pooled_connection)
    {
        if (!api_retry_circuit_allow(connection_handle, retry_policy,
                error_code))
            return (ft_nullptr);
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            if (api_is_configuration_socket_error(socket_setup_error))
                error_code = socket_setup_error;
            else
            {
                error_code = FT_ERR_SOCKET_CONNECT_FAILED;
                downgrade_due_to_connect_failure = true;
            }
            api_retry_circuit_record_failure(connection_handle, retry_policy);
            if (!downgrade_due_to_connect_failure)
                return (ft_nullptr);
        }
        if (!downgrade_due_to_connect_failure)
            connection_handle.has_socket = true;
    }
    struct api_connection_return_guard
    {
        api_connection_pool_handle *handle;
        bool success;
        api_connection_return_guard(api_connection_pool_handle &value)
        {
            handle = &value;
            success = false;
            return ;
        }
        ~api_connection_return_guard()
        {
            if (!handle)
                return ;
            if (success)
            {
                api_connection_pool_mark_idle(*handle);
                return ;
            }
            api_connection_pool_evict(*handle);
            return ;
        }
        void set_success(void)
        {
            success = true;
            return ;
        }
    } connection_guard(connection_handle);
    if (downgrade_due_to_connect_failure)
    {
        int max_attempts;
        int attempt_index;
        int initial_delay;
        int current_delay;
        int max_delay;
        int multiplier;

        if (g_api_request_wait_hook)
        {
            bool wait_ready;

            wait_ready = g_api_request_wait_hook();
            if (!wait_ready)
                return (ft_nullptr);
        }
        max_attempts = api_retry_get_max_attempts(retry_policy);
        initial_delay = api_retry_get_initial_delay(retry_policy);
        max_delay = api_retry_get_max_delay(retry_policy);
        multiplier = api_retry_get_multiplier(retry_policy);
        current_delay = api_retry_prepare_delay(initial_delay, max_delay);
        attempt_index = 0;
        while (attempt_index < max_attempts)
        {
            bool socket_ready;
            bool should_retry;

            if (!api_retry_circuit_allow(connection_handle, retry_policy,
                    error_code))
                return (ft_nullptr);
            error_code = FT_ERR_SUCCESS;
            socket_ready = api_http_prepare_plain_socket(connection_handle, ip,
                    port, timeout, error_code);
            if (socket_ready)
            {
                metrics_result_body = api_http_execute_plain(connection_handle,
                        method, path, ip, payload, headers, status, timeout,
                        ip, port, retry_policy, error_code);
                if (metrics_result_body)
                {
                    connection_guard.set_success();
                    if (used_http2)
                        *used_http2 = false;
                    return (metrics_result_body);
                }
            }
            should_retry = api_http_should_retry_plain(error_code);
            if (!should_retry)
                return (ft_nullptr);
            api_retry_circuit_record_failure(connection_handle, retry_policy);
            api_connection_pool_evict(connection_handle);
            attempt_index = attempt_index + 1;
            if (attempt_index >= max_attempts)
                break;
            if (current_delay > 0)
            {
                int sleep_delay;

                sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
                if (sleep_delay > 0)
                    time_sleep_ms(static_cast<unsigned int>(sleep_delay));
            }
            current_delay = api_retry_next_delay(current_delay, max_delay,
                    multiplier);
            if (current_delay <= 0)
                current_delay = api_retry_prepare_delay(initial_delay,
                        max_delay);
        }
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        if (used_http2)
            *used_http2 = false;
        return (ft_nullptr);
    }
    if (!connection_handle.has_socket)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    bool http2_used_local;

    http2_used_local = false;
    metrics_result_body = api_http_execute_plain_http2(connection_handle,
            method, path, ip, payload, headers, status, timeout, ip, port,
            retry_policy, http2_used_local, error_code);
    if (!metrics_result_body)
    {
        bool fallback_socket_alive;
        int previous_error_code;

        fallback_socket_alive = false;
        if (connection_handle.has_socket)
            fallback_socket_alive = api_http_plain_socket_is_alive(connection_handle);
        if (!fallback_socket_alive)
        {
            connection_handle.socket.close_socket();
            connection_handle.has_socket = false;
            connection_handle.from_pool = false;
        }
        connection_handle.should_store = pooled_connection;
        connection_handle.negotiated_http2 = false;
        connection_handle.tls_session = ft_nullptr;
        connection_handle.tls_context = ft_nullptr;
        previous_error_code = error_code;
        error_code = FT_ERR_SUCCESS;
        metrics_result_body = api_http_execute_plain(connection_handle, method,
                path, ip, payload, headers, status, timeout, ip, port,
                retry_policy, error_code);
        if (!metrics_result_body)
        {
            if (previous_error_code != FT_ERR_SUCCESS
                && previous_error_code != FT_ERR_SOCKET_CONNECT_FAILED
                && error_code == FT_ERR_SOCKET_CONNECT_FAILED)
            {
                error_code = previous_error_code;
            }
            return (ft_nullptr);
        }
        http2_used_local = false;
    }
    connection_guard.set_success();
    if (used_http2)
        *used_http2 = http2_used_local;
    return (metrics_result_body);
}

json_group *api_request_json(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string(ip, port, method, path, payload,
            headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

json_group *api_request_json_http2(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy)
{
    bool http2_used_local;
    char *body;

    http2_used_local = false;
    int error_code = FT_ERR_SUCCESS;
    body = api_request_string_http2(ip, port, method, path, payload,
            headers, status, timeout, &http2_used_local, retry_policy);
    if (used_http2)
        *used_http2 = http2_used_local;
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

bool api_request_stream_host(const char *host, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout,
    const api_retry_policy *retry_policy)
{
    if (ft_log_get_api_logging())
    {
        const char *log_host = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";

        if (host)
            log_host = host;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_stream_host %s:%u %s %s", log_host, port,
            log_method, log_path);
    }
    if (!host || !method || !path || !streaming_handler)
    {
                return (false);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_stream_host)
    {
        return (hooks->request_stream_host(host, port, method, path,
                streaming_handler, payload, headers, timeout, retry_policy,
                hooks->user_data));
    }
    char port_string[6];
    struct addrinfo hints;
    struct addrinfo *address_results;
    struct addrinfo *address_info;
    int resolver_status;

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    resolver_status = getaddrinfo(host, port_string, &hints, &address_results);
    if (resolver_status != 0)
    {
        api_request_set_resolve_error(resolver_status);
        return (false);
    }
    address_info = address_results;
    while (address_info && address_info->ai_socktype != SOCK_STREAM)
        address_info = address_info->ai_next;
    if (!address_info)
    {
        freeaddrinfo(address_results);
                return (false);
    }
    char ip_buffer[INET6_ADDRSTRLEN];
    void *source_address;
    int family;

    family = address_info->ai_family;
    if (family == AF_INET)
    {
        source_address = &reinterpret_cast<sockaddr_in*>(address_info->ai_addr)->sin_addr;
        if (!inet_ntop(family, source_address, ip_buffer, sizeof(ip_buffer)))
        {
            freeaddrinfo(address_results);
            return (false);
        }
    }
    else if (family == AF_INET6)
    {
        source_address = &reinterpret_cast<sockaddr_in6*>(address_info->ai_addr)->sin6_addr;
        if (!inet_ntop(family, source_address, ip_buffer, sizeof(ip_buffer)))
        {
            freeaddrinfo(address_results);
            return (false);
        }
    }
    else
    {
        freeaddrinfo(address_results);
                return (false);
    }
    freeaddrinfo(address_results);
    return (api_request_stream(ip_buffer, port, method, path,
        streaming_handler, payload, headers, timeout, retry_policy));
}

char *api_request_string_host(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (ft_log_get_api_logging())
    {
        const char *log_host = "(null)";
        const char *log_method = "(null)";
        const char *log_path = "(null)";

        if (host)
            log_host = host;
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_request_string_host %s:%u %s %s",
            log_host, port, log_method, log_path);
    }
    int error_code = FT_ERR_SUCCESS;
    if (!host || !method || !path)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_string_host)
    {
        char *hook_result = hooks->request_string_host(host, port, method,
                path, payload, headers, status, timeout, retry_policy,
                hooks->user_data);
        if (!hook_result)
        {
            error_code = FT_ERR_IO;
            if (error_code == FT_ERR_SUCCESS)
                error_code = FT_ERR_IO;
        }
        return (hook_result);
    }
    char port_string[6];
    struct addrinfo hints;
    struct addrinfo *address_results;
    struct addrinfo *address_info;
    int resolver_status;

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    resolver_status = getaddrinfo(host, port_string, &hints, &address_results);
    if (resolver_status != 0)
    {
        error_code = api_request_set_resolve_error(resolver_status);
        return (ft_nullptr);
    }
    address_info = address_results;
    while (address_info && address_info->ai_socktype != SOCK_STREAM)
        address_info = address_info->ai_next;
    if (!address_info)
    {
        freeaddrinfo(address_results);
        error_code = FT_ERR_SOCKET_RESOLVE_FAILED;
        return (ft_nullptr);
    }
    char ip_buffer[INET6_ADDRSTRLEN];
    void *source_address;
    int family;

    family = address_info->ai_family;
    if (family == AF_INET)
    {
        source_address = &reinterpret_cast<sockaddr_in*>(address_info->ai_addr)->sin_addr;
        if (!inet_ntop(family, source_address, ip_buffer, sizeof(ip_buffer)))
        {
            int resolve_error;

            resolve_error = api_request_capture_network_error();
            freeaddrinfo(address_results);
            error_code = api_request_assign_resolve_error(resolve_error);
            return (ft_nullptr);
        }
    }
    else if (family == AF_INET6)
    {
        source_address = &reinterpret_cast<sockaddr_in6*>(address_info->ai_addr)->sin6_addr;
        if (!inet_ntop(family, source_address, ip_buffer, sizeof(ip_buffer)))
        {
            int resolve_error;

            resolve_error = api_request_capture_network_error();
            freeaddrinfo(address_results);
            error_code = api_request_assign_resolve_error(resolve_error);
            return (ft_nullptr);
        }
    }
    else
    {
        freeaddrinfo(address_results);
        error_code = FT_ERR_SOCKET_RESOLVE_FAMILY;
        return (ft_nullptr);
    }
    freeaddrinfo(address_results);
    char *result = api_request_string(ip_buffer, port, method, path, payload,
            headers, status, timeout, retry_policy);
    if (!result)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

json_group *api_request_json_host(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_host(host, port, method, path, payload,
            headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

char *api_request_string_bearer(const char *ip, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!token)
        return (api_request_string(ip, port, method, path, payload,
                                   headers, status, timeout, retry_policy));
    ft_string header_string;

    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Bearer ";
    header_string += token;
    return (api_request_string(ip, port, method, path, payload,
                               header_string.c_str(), status, timeout,
                               retry_policy));
}

json_group *api_request_json_bearer(const char *ip, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_bearer(ip, port, method, path, token,
            payload, headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

char *api_request_string_basic(const char *ip, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!credentials)
        return (api_request_string(ip, port, method, path, payload,
                                   headers, status, timeout, retry_policy));
    ft_string header_string;

    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Basic ";
    header_string += credentials;
    return (api_request_string(ip, port, method, path, payload,
                               header_string.c_str(), status, timeout,
                               retry_policy));
}

json_group *api_request_json_basic(const char *ip, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_basic(ip, port, method, path, credentials,
            payload, headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

char *api_request_string_host_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!token)
        return (api_request_string_host(host, port, method, path, payload,
                                        headers, status, timeout,
                                        retry_policy));
    ft_string header_string;

    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Bearer ";
    header_string += token;
    return (api_request_string_host(host, port, method, path, payload,
                                    header_string.c_str(), status, timeout,
                                    retry_policy));
}

json_group *api_request_json_host_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_host_bearer(host, port, method, path, token,
            payload, headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

char *api_request_string_host_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!credentials)
        return (api_request_string_host(host, port, method, path, payload,
                                        headers, status, timeout,
                                        retry_policy));
    ft_string header_string;

    if (headers && headers[0])
    {
        header_string = headers;
        header_string += "\r\n";
    }
    header_string += "Authorization: Basic ";
    header_string += credentials;
    return (api_request_string_host(host, port, method, path, payload,
                                    header_string.c_str(), status, timeout,
                                    retry_policy));
}

json_group *api_request_json_host_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_host_basic(host, port, method, path,
            credentials, payload, headers, status, timeout,
            retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

static bool parse_url(const char *url, bool &tls, ft_string &host,
    uint16_t &port, ft_string &path, int &error_code)
{
    const char *scheme_end;
    const char *path_start;
    const char *walker;
    ft_string scheme;
    ft_string hostport;
    const char *colon;
    const char *slash;
    const char *host_start;

    if (!url)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    scheme_end = ft_strstr(url, "://");
    if (!scheme_end)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    walker = url;
    while (walker < scheme_end)
    {
        scheme.append(*walker);
        int scheme_error = ft_string::last_operation_error();
        if (scheme_error != FT_ERR_SUCCESS)
        {
            error_code = scheme_error;
            return (false);
        }
        walker++;
    }
    tls = (scheme == "https");
    host_start = scheme_end + 3;
    path_start = ft_strchr(host_start, '/');
    path.clear();
    if (path_start)
    {
        path.append(path_start);
        int path_error = ft_string::last_operation_error();
        if (path_error != FT_ERR_SUCCESS)
        {
            error_code = path_error;
            return (false);
        }
    }
    else
    {
        slash = "/";
        path = slash;
        int path_error = ft_string::last_operation_error();
        if (path_error != FT_ERR_SUCCESS)
        {
            error_code = path_error;
            return (false);
        }
    }
    if (path_start)
    {
        hostport.clear();
        walker = host_start;
        while (walker < path_start)
        {
            hostport.append(*walker);
            int hostport_error = ft_string::last_operation_error();
            if (hostport_error != FT_ERR_SUCCESS)
            {
                error_code = hostport_error;
                return (false);
            }
            walker++;
        }
    }
    else
    {
        hostport.clear();
        hostport.append(host_start);
        int hostport_error = ft_string::last_operation_error();
        if (hostport_error != FT_ERR_SUCCESS)
        {
            error_code = hostport_error;
            return (false);
        }
    }
    colon = ft_strchr(hostport.c_str(), ':');
    if (colon)
    {
        host.clear();
        walker = hostport.c_str();
        while (walker < colon)
        {
            host.append(*walker);
            int host_error = ft_string::last_operation_error();
            if (host_error != FT_ERR_SUCCESS)
            {
                error_code = host_error;
                return (false);
            }
            walker++;
        }
        port = static_cast<uint16_t>(ft_atoi(colon + 1));
    }
    else
    {
        host = hostport;
        int host_error = ft_string::last_operation_error();
        if (host_error != FT_ERR_SUCCESS)
        {
            error_code = host_error;
            return (false);
        }
        if (tls)
            port = 443;
        else
            port = 80;
    }
    error_code = FT_ERR_SUCCESS;
    return (true);
}

char *api_request_string_url(const char *url, const char *method,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (ft_log_get_api_logging())
    {
        const char *log_url = "(null)";
        const char *log_method = "(null)";
        if (url)
            log_url = url;
        if (method)
            log_method = method;
        ft_log_debug("api_request_string_url %s %s",
            log_url, log_method);
    }
    int error_code = FT_ERR_SUCCESS;
    if (!method)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    bool tls;
    ft_string host;
    ft_string path;
    uint16_t port;
    if (!parse_url(url, tls, host, port, path, error_code))
        return (ft_nullptr);
    char *result;
    if (tls)
        result = api_request_string_tls(host.c_str(), port, method,
                path.c_str(), payload, headers, status, timeout,
                retry_policy);
    else
        result = api_request_string_host(host.c_str(), port, method,
                path.c_str(), payload, headers, status, timeout,
                retry_policy);
    if (!result)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}

json_group *api_request_json_url(const char *url, const char *method,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    int error_code = FT_ERR_SUCCESS;
    char *body = api_request_string_url(url, method, payload,
            headers, status, timeout, retry_policy);
    if (!body)
    {
        error_code = FT_ERR_IO;
        if (error_code == FT_ERR_SUCCESS)
            error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        error_code = FT_ERR_IO;
        return (ft_nullptr);
    }
    error_code = FT_ERR_SUCCESS;
    return (result);
}
