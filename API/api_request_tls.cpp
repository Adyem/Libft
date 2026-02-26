#include "api.hpp"
#include "api_internal.hpp"
#include "api_http_internal.hpp"
#include "api_request_metrics.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Networking/networking.hpp"
#include "../Networking/openssl_support.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/thread.hpp"
#include <errno.h>
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
# include <fcntl.h>
# include <sys/select.h>
#endif

#
#if NETWORKING_HAS_OPENSSL
#include <openssl/err.h>
#include <cstdint>
#include <climits>
#include <utility>
#endif

#if NETWORKING_HAS_OPENSSL

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

static int api_request_tls_prepare_connection_handle(api_connection_pool_handle &handle,
    int &error_code) noexcept
{
    int initialize_result = handle.initialize();
    if (initialize_result != FT_ERR_SUCCESS)
        error_code = initialize_result;
    return initialize_result;
}

static void api_request_assign_network_error(int mapped_error, int fallback)
{
    (void)mapped_error;
    (void)fallback;
    return ;
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

char *api_request_https(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer,
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
        ft_log_debug("api_request_https %s:%u %s %s",
            log_ip, port, log_method, log_path);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_https)
    {
        return (hooks->request_https(ip, port, method, path, payload, headers,
                status, timeout, ca_certificate, verify_peer, retry_policy,
                hooks->user_data));
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
    if (payload)
    {
        metrics_payload_string = json_write_to_string(payload);
    }
    else
    {
        metrics_payload_string = ft_nullptr;
    }
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    int error_code = FT_ERR_SUCCESS;
    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard;
    (void)metrics_guard.initialize(ip, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_string security_identity;
    const char *security_identity_pointer;

    security_identity_pointer = ft_nullptr;
    if (verify_peer)
    {
        security_identity = "verify:1";
        if (ca_certificate && ca_certificate[0] != '\0')
        {
            security_identity += ":";
            security_identity += ca_certificate;
        }
    }
    else
        security_identity = "verify:0";
    if (!security_identity.empty())
        security_identity_pointer = security_identity.c_str();

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (ft_nullptr);

    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::TLS, security_identity_pointer);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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

    bool http2_used_local;
    http2_used_local = false;
    metrics_result_body = api_https_execute_http2(connection_handle, method,
            path, ip, payload, headers, status, timeout, ca_certificate,
            verify_peer, ip, port, security_identity_pointer, retry_policy,
            http2_used_local, error_code);
    if (!metrics_result_body)
    {
        error_code = FT_ERR_SUCCESS;
        metrics_result_body = api_https_execute(connection_handle, method,
                path, ip, payload, headers, status, timeout, ca_certificate,
                verify_peer, ip, port, security_identity_pointer, retry_policy,
                error_code);
        if (!metrics_result_body)
            return (ft_nullptr);
    }
    connection_guard.set_success();
    return (metrics_result_body);
}

char *api_request_https_http2(const char *ip, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, bool *used_http2,
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
        ft_log_debug("api_request_https_http2 %s:%u %s %s",
            log_ip, port, log_method, log_path);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_https_http2)
    {
        return (hooks->request_https_http2(ip, port, method, path, payload,
                headers, status, timeout, ca_certificate, verify_peer,
                used_http2, retry_policy, hooks->user_data));
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
    if (payload)
    {
        metrics_payload_string = json_write_to_string(payload);
    }
    else
    {
        metrics_payload_string = ft_nullptr;
    }
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    int error_code = FT_ERR_SUCCESS;
    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard;
    (void)metrics_guard.initialize(ip, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, ip);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_string security_identity;
    const char *security_identity_pointer;

    security_identity_pointer = ft_nullptr;
    if (verify_peer)
    {
        security_identity = "verify:1";
        if (ca_certificate && ca_certificate[0] != '\0')
        {
            security_identity += ":";
            security_identity += ca_certificate;
        }
    }
    else
        security_identity = "verify:0";
    if (!security_identity.empty())
        security_identity_pointer = security_identity.c_str();

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (ft_nullptr);

    pooled_connection = api_connection_pool_acquire(connection_handle, ip, port,
            api_connection_security_mode::TLS, security_identity_pointer);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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

    bool http2_used_local;

    http2_used_local = false;
    metrics_result_body = api_https_execute_http2(connection_handle, method,
            path, ip, payload, headers, status, timeout, ca_certificate,
            verify_peer, ip, port, security_identity_pointer, retry_policy,
            http2_used_local, error_code);
    if (!metrics_result_body)
    {
        error_code = FT_ERR_SUCCESS;
        metrics_result_body = api_https_execute(connection_handle, method,
                path, ip, payload, headers, status, timeout, ca_certificate,
                verify_peer, ip, port, security_identity_pointer, retry_policy,
                error_code);
        if (!metrics_result_body)
            return (ft_nullptr);
        http2_used_local = false;
    }
    connection_guard.set_success();
    if (used_http2)
        *used_http2 = http2_used_local;
    return (metrics_result_body);
}

bool api_request_stream_tls(const char *host, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout, const char *ca_certificate,
    bool verify_peer, const api_retry_policy *retry_policy)
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
        ft_log_debug("api_request_stream_tls %s:%u %s %s", log_host, port,
            log_method, log_path);
    }
    if (!streaming_handler || !host || !method || !path)
        return (false);
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_stream_tls)
    {
        return (hooks->request_stream_tls(host, port, method, path,
                streaming_handler, payload, headers, timeout, ca_certificate,
                verify_peer, retry_policy, hooks->user_data));
    }
    int error_code = FT_ERR_SUCCESS;

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, host);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_string security_identity;
    const char *security_identity_pointer;

    security_identity_pointer = ft_nullptr;
    if (verify_peer)
    {
        security_identity = "verify:1";
        if (ca_certificate && ca_certificate[0] != '\0')
        {
            security_identity += ":";
            security_identity += ca_certificate;
        }
    }
    else
        security_identity = "verify:0";
    if (!security_identity.empty())
        security_identity_pointer = security_identity.c_str();

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (false);

    pooled_connection = api_connection_pool_acquire(connection_handle, host,
            port, api_connection_security_mode::TLS, security_identity_pointer);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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
    result = api_https_execute_http2_streaming(connection_handle, method, path,
            host, payload, headers, timeout, ca_certificate, verify_peer,
            host, port, security_identity_pointer, retry_policy,
            streaming_handler, http2_used_local, error_code);
    if (!result)
    {
        error_code = FT_ERR_SUCCESS;
        result = api_https_execute_streaming(connection_handle, method, path,
                host, payload, headers, timeout, ca_certificate, verify_peer,
                host, port, security_identity_pointer, retry_policy,
                streaming_handler, error_code);
        if (!result)
            return (false);
    }
    connection_guard.set_success();
    return (true);
}

bool api_request_stream_tls_http2(const char *host, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout, const char *ca_certificate,
    bool verify_peer, bool *used_http2, const api_retry_policy *retry_policy)
{
    if (used_http2)
        *used_http2 = false;
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
        ft_log_debug("api_request_stream_tls_http2 %s:%u %s %s", log_host,
            port, log_method, log_path);
    }
    if (!streaming_handler || !host || !method || !path)
        return (false);
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_stream_tls_http2)
    {
        return (hooks->request_stream_tls_http2(host, port, method, path,
                streaming_handler, payload, headers, timeout, ca_certificate,
                verify_peer, used_http2, retry_policy, hooks->user_data));
    }
    int error_code = FT_ERR_SUCCESS;

    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, host);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    ft_string security_identity;
    const char *security_identity_pointer;

    security_identity_pointer = ft_nullptr;
    if (verify_peer)
    {
        security_identity = "verify:1";
        if (ca_certificate && ca_certificate[0] != '\0')
        {
            security_identity += ":";
            security_identity += ca_certificate;
        }
    }
    else
        security_identity = "verify:0";
    if (!security_identity.empty())
        security_identity_pointer = security_identity.c_str();

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (false);

    pooled_connection = api_connection_pool_acquire(connection_handle, host,
            port, api_connection_security_mode::TLS, security_identity_pointer);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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
    result = api_https_execute_http2_streaming(connection_handle, method, path,
            host, payload, headers, timeout, ca_certificate, verify_peer, host,
            port, security_identity_pointer, retry_policy, streaming_handler,
            http2_used_local, error_code);
    if (!result)
    {
        error_code = FT_ERR_SUCCESS;
        result = api_https_execute_streaming(connection_handle, method, path,
                host, payload, headers, timeout, ca_certificate, verify_peer,
                host, port, security_identity_pointer, retry_policy,
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

char *api_request_string_tls(const char *host, uint16_t port,
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
        ft_log_debug("api_request_string_tls %s:%u %s %s",
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
    if (hooks && hooks->request_string_tls)
    {
        return (hooks->request_string_tls(host, port, method, path, payload,
                headers, status, timeout, retry_policy, hooks->user_data));
    }

    size_t metrics_request_bytes;
    char *metrics_payload_string;

    metrics_request_bytes = 0;
    metrics_request_bytes += ft_strlen(host);
    if (method)
        metrics_request_bytes += ft_strlen(method);
    if (path)
        metrics_request_bytes += ft_strlen(path);
    if (headers && headers[0])
        metrics_request_bytes += ft_strlen(headers);
    if (payload)
    {
        metrics_payload_string = json_write_to_string(payload);
    }
    else
    {
        metrics_payload_string = ft_nullptr;
    }
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, host);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (ft_nullptr);

    pooled_connection = api_connection_pool_acquire(connection_handle, host,
            port, api_connection_security_mode::TLS, ft_nullptr);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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

    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard;
    (void)metrics_guard.initialize(host, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    metrics_result_body = api_https_execute(connection_handle, method, path, host,
            payload, headers, status, timeout, ft_nullptr, false, host, port,
            ft_nullptr, retry_policy, error_code);
    if (!metrics_result_body)
        return (ft_nullptr);
    connection_guard.set_success();
    return (metrics_result_body);
}

char *api_request_string_tls_http2(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy)
{
    if (used_http2)
        *used_http2 = false;
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
        ft_log_debug("api_request_string_tls_http2 %s:%u %s %s",
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
    if (hooks && hooks->request_string_tls_http2)
    {
        return (hooks->request_string_tls_http2(host, port, method, path,
                payload, headers, status, timeout, used_http2, retry_policy,
                hooks->user_data));
    }

    size_t metrics_request_bytes;
    char *metrics_payload_string;

    metrics_request_bytes = 0;
    metrics_request_bytes += ft_strlen(host);
    if (method)
        metrics_request_bytes += ft_strlen(method);
    if (path)
        metrics_request_bytes += ft_strlen(path);
    if (headers && headers[0])
        metrics_request_bytes += ft_strlen(headers);
    if (payload)
    {
        metrics_payload_string = json_write_to_string(payload);
    }
    else
    {
        metrics_payload_string = ft_nullptr;
    }
    if (metrics_payload_string)
    {
        metrics_request_bytes += ft_strlen(metrics_payload_string);
        cma_free(metrics_payload_string);
    }
    SocketConfig config;
    config._type = SocketType::CLIENT;
    api_request_copy_ip(config._ip, host);
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;

    api_connection_pool_handle connection_handle;
    bool pooled_connection;
    if (api_request_tls_prepare_connection_handle(connection_handle, error_code) != FT_ERR_SUCCESS)
        return (ft_nullptr);

    pooled_connection = api_connection_pool_acquire(connection_handle, host, port,
            api_connection_security_mode::TLS, host);
    if (!pooled_connection)
    {
        int socket_setup_error;

        socket_setup_error = connection_handle.socket.initialize(config);
        if (socket_setup_error != FT_ERR_SUCCESS)
        {
            error_code = socket_setup_error;
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
    bool http2_used_local;
    char *metrics_result_body;
    int metrics_status_storage;
    int *metrics_status_pointer;

    metrics_result_body = ft_nullptr;
    metrics_status_storage = -1;
    if (status)
        metrics_status_pointer = status;
    else
        metrics_status_pointer = &metrics_status_storage;
    api_request_metrics_guard metrics_guard;
    (void)metrics_guard.initialize(host, port, method, path,
        metrics_request_bytes, &metrics_result_body, metrics_status_pointer,
        &error_code);

    http2_used_local = false;
    metrics_result_body = api_https_execute_http2(connection_handle, method, path, host,
            payload, headers, status, timeout, ft_nullptr, true, host, port,
            ft_nullptr, retry_policy, http2_used_local, error_code);
    if (!metrics_result_body)
    {
        error_code = FT_ERR_SUCCESS;
        metrics_result_body = api_https_execute(connection_handle, method, path, host,
                payload, headers, status, timeout, ft_nullptr, true, host,
                port, ft_nullptr, retry_policy, error_code);
        if (!metrics_result_body)
            return (ft_nullptr);
        http2_used_local = false;
    }
    connection_guard.set_success();
    if (used_http2)
        *used_http2 = http2_used_local;
    return (metrics_result_body);
}

json_group *api_request_json_tls(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    char *body = api_request_string_tls(host, port, method, path, payload,
                                       headers, status, timeout, retry_policy);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

json_group *api_request_json_tls_http2(const char *host, uint16_t port,
    const char *method, const char *path, json_group *payload,
    const char *headers, int *status, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy)
{
    bool http2_used_local;
    char *body;

    http2_used_local = false;
    body = api_request_string_tls_http2(host, port, method, path, payload,
            headers, status, timeout, &http2_used_local, retry_policy);
    if (used_http2)
        *used_http2 = http2_used_local;
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

char *api_request_string_tls_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!token)
        return (api_request_string_tls(host, port, method, path, payload,
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
    return (api_request_string_tls(host, port, method, path, payload,
                                   header_string.c_str(), status, timeout,
                                   retry_policy));
}

json_group *api_request_json_tls_bearer(const char *host, uint16_t port,
    const char *method, const char *path, const char *token,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    char *body = api_request_string_tls_bearer(host, port, method, path, token,
                                               payload, headers, status, timeout,
                                               retry_policy);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

char *api_request_string_tls_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    if (!credentials)
        return (api_request_string_tls(host, port, method, path, payload,
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
    return (api_request_string_tls(host, port, method, path, payload,
                                   header_string.c_str(), status, timeout,
                                   retry_policy));
}

json_group *api_request_json_tls_basic(const char *host, uint16_t port,
    const char *method, const char *path, const char *credentials,
    json_group *payload, const char *headers, int *status, int timeout,
    const api_retry_policy *retry_policy)
{
    char *body = api_request_string_tls_basic(host, port, method, path,
                                              credentials, payload, headers,
                                              status, timeout, retry_policy);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    return (result);
}

struct api_tls_async_request
{
    char *host;
    uint16_t port;
    char *method;
    char *path;
    json_group *payload;
    char *headers;
    int timeout;
    api_callback callback;
    void *user_data;
};

static void api_tls_async_worker(api_tls_async_request *data)
{
    SSL_CTX *context = ft_nullptr;
    SSL *ssl_session = ft_nullptr;
    int socket_fd = -1;
    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_string request;
    ft_string body_string;
    ft_string response;
    char port_string[6];
    char *result_body = ft_nullptr;
    int status = -1;
    char buffer[1024];
    fd_set read_set;
    fd_set write_set;
    struct timeval tv;
    size_t total_sent;
    int ssl_ret;
    int async_resolver_status;

    if (!data || !data->host || !data->method || !data->path)
        goto cleanup;
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }
    context = SSL_CTX_new(TLS_client_method());
    if (!context)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }

    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", data->port);
    async_resolver_status = getaddrinfo(data->host, port_string, &hints, &address_results);
    if (async_resolver_status != 0)
    {
        api_request_set_resolve_error(async_resolver_status);
        goto cleanup;
    }
    address_info = address_results;
    while (address_info != ft_nullptr && socket_fd < 0)
    {
        socket_fd = nw_socket(address_info->ai_family,
                               address_info->ai_socktype,
                               address_info->ai_protocol);
        if (socket_fd < 0)
            address_info = address_info->ai_next;
    }
    if (socket_fd < 0)
        goto cleanup;

#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(socket_fd, FIONBIO, &mode);
#else
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
#endif

    if (nw_connect(socket_fd, address_info->ai_addr,
                    static_cast<socklen_t>(address_info->ai_addrlen)) < 0)
    {
        int connect_error;

        connect_error = api_request_capture_network_error();
#ifdef _WIN32
        if (connect_error != cmp_map_system_error_to_ft(WSAEINPROGRESS)
            && connect_error != cmp_map_system_error_to_ft(WSAEWOULDBLOCK))
#else
        if (connect_error != cmp_map_system_error_to_ft(EINPROGRESS))
#endif
        {
            api_request_assign_network_error(connect_error,
                FT_ERR_SOCKET_CONNECT_FAILED);
            goto cleanup;
        }
    }

    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    tv.tv_sec = data->timeout / 1000;
    tv.tv_usec = (data->timeout % 1000) * 1000;
    int wait_result;

    wait_result = select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv);
    if (wait_result <= 0)
    {
        int wait_error;

        wait_error = api_request_capture_network_error();
        api_request_assign_network_error(wait_error,
            FT_ERR_SOCKET_CONNECT_FAILED);
        goto cleanup;
    }

    ssl_session = SSL_new(context);
    if (!ssl_session)
    {
        api_request_set_ssl_error(ft_nullptr, 0);
        goto cleanup;
    }
    if (SSL_set_fd(ssl_session, socket_fd) != 1)
    {
        api_request_set_ssl_error(ssl_session, 0);
        goto cleanup;
    }

    ssl_ret = SSL_connect(ssl_session);
    while (ssl_ret <= 0)
    {
        int ssl_err = SSL_get_error(ssl_session, ssl_ret);
        if (ssl_err == SSL_ERROR_WANT_READ)
        {
            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            int select_result;

            select_result = select(socket_fd + 1, &read_set, ft_nullptr,
                ft_nullptr, &tv);
            if (select_result <= 0)
            {
                int select_error;

                select_error = api_request_capture_network_error();
                api_request_assign_network_error(select_error,
                    FT_ERR_SOCKET_CONNECT_FAILED);
                goto cleanup;
            }
        }
        else if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            int select_result;

            select_result = select(socket_fd + 1, ft_nullptr, &write_set,
                ft_nullptr, &tv);
            if (select_result <= 0)
            {
                int select_error;

                select_error = api_request_capture_network_error();
                api_request_assign_network_error(select_error,
                    FT_ERR_SOCKET_CONNECT_FAILED);
                goto cleanup;
            }
        }
        else
            goto cleanup;
        ssl_ret = SSL_connect(ssl_session);
    }

    request = data->method;
    request += " ";
    request += data->path;
    request += " HTTP/1.1\r\nHost: ";
    request += data->host;
    if (data->headers && data->headers[0])
    {
        request += "\r\n";
        request += data->headers;
    }
    if (data->payload)
    {
        char *temporary_string = json_write_to_string(data->payload);
        if (!temporary_string)
            goto cleanup;
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        if (!api_append_content_length_header(request, body_string.size()))
        {
            goto cleanup;
        }
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (data->payload)
        request += body_string.c_str();

    total_sent = 0;
    while (total_sent < request.size())
    {
        int write_ret = SSL_write(ssl_session, request.c_str() + total_sent,
                                  static_cast<int>(request.size() - total_sent));
        if (write_ret <= 0)
        {
            int ssl_err = SSL_get_error(ssl_session, write_ret);
            if (ssl_err == SSL_ERROR_WANT_READ)
            {
                FD_ZERO(&read_set);
                FD_SET(socket_fd, &read_set);
                int select_result;

                select_result = select(socket_fd + 1, &read_set, ft_nullptr,
                    ft_nullptr, &tv);
                if (select_result <= 0)
                {
                    int select_error;

                    select_error = api_request_capture_network_error();
                    api_request_assign_network_error(select_error,
                        FT_ERR_SOCKET_SEND_FAILED);
                    goto cleanup;
                }
                continue;
            }
            else if (ssl_err == SSL_ERROR_WANT_WRITE)
            {
                FD_ZERO(&write_set);
                FD_SET(socket_fd, &write_set);
                int select_result;

                select_result = select(socket_fd + 1, ft_nullptr, &write_set,
                    ft_nullptr, &tv);
                if (select_result <= 0)
                {
                    int select_error;

                    select_error = api_request_capture_network_error();
                    api_request_assign_network_error(select_error,
                        FT_ERR_SOCKET_SEND_FAILED);
                    goto cleanup;
                }
                continue;
            }
            goto cleanup;
        }
        total_sent += static_cast<size_t>(write_ret);
    }

    while (true)
    {
        int read_ret = SSL_read(ssl_session, buffer, sizeof(buffer) - 1);
        if (read_ret > 0)
        {
            buffer[read_ret] = '\0';
            response += buffer;
            continue;
        }
        int ssl_err = SSL_get_error(ssl_session, read_ret);
        if (ssl_err == SSL_ERROR_WANT_READ)
        {
            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            int select_result;

            select_result = select(socket_fd + 1, &read_set, ft_nullptr,
                ft_nullptr, &tv);
            if (select_result <= 0)
            {
                int select_error;

                select_error = api_request_capture_network_error();
                api_request_assign_network_error(select_error,
                    FT_ERR_SOCKET_RECEIVE_FAILED);
                break;
            }
            continue;
        }
        if (ssl_err == SSL_ERROR_WANT_WRITE)
        {
            FD_ZERO(&write_set);
            FD_SET(socket_fd, &write_set);
            int select_result;

            select_result = select(socket_fd + 1, ft_nullptr, &write_set,
                ft_nullptr, &tv);
            if (select_result <= 0)
            {
                int select_error;

                select_error = api_request_capture_network_error();
                api_request_assign_network_error(select_error,
                    FT_ERR_SOCKET_RECEIVE_FAILED);
                break;
            }
            continue;
        }
        break;
    }

    if (response.size() > 0)
    {
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            status = ft_atoi(space + 1);
        const char *body = ft_strstr(response.c_str(), "\r\n\r\n");
        if (body)
        {
            body += 4;
            result_body = adv_strdup(body);
        }
    }

cleanup:
    if (data->callback)
        data->callback(result_body, status, data->user_data);
    if (ssl_session)
    {
        SSL_shutdown(ssl_session);
        SSL_free(ssl_session);
    }
    if (socket_fd >= 0)
        nw_close(socket_fd);
    if (context)
        SSL_CTX_free(context);
    if (address_results)
        freeaddrinfo(address_results);
    if (data->host)
        cma_free(data->host);
    if (data->method)
        cma_free(data->method);
    if (data->path)
        cma_free(data->path);
    if (data->headers)
        cma_free(data->headers);
    cma_free(data);
    return ;
}

bool    api_request_string_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    if (!host || !method || !path || !callback)
        return (false);
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_string_tls_async)
    {
        return (hooks->request_string_tls_async(host, port, method, path,
                callback, user_data, payload, headers, timeout,
                hooks->user_data));
    }
    api_tls_async_request *data = static_cast<api_tls_async_request*>(cma_malloc(sizeof(api_tls_async_request)));
    if (!data)
        return (false);
    ft_bzero(data, sizeof(api_tls_async_request));
    data->host = adv_strdup(host);
    data->method = adv_strdup(method);
    data->path = adv_strdup(path);
    if (headers)
        data->headers = adv_strdup(headers);
    data->port = port;
    data->payload = payload;
    data->timeout = timeout;
    data->callback = callback;
    data->user_data = user_data;
    if (!data->host || !data->method || !data->path || (headers && !data->headers))
    {
        if (data->host)
            cma_free(data->host);
        if (data->method)
            cma_free(data->method);
        if (data->path)
            cma_free(data->path);
        if (data->headers)
            cma_free(data->headers);
        cma_free(data);
        return (false);
    }
    ft_thread thread_worker(api_tls_async_worker, data);
    thread_worker.detach();
    return (true);
}

struct api_json_async_data
{
    api_json_callback callback;
    void *user_data;
};

static void api_json_async_wrapper(char *body, int status, void *user_data)
{
    api_json_async_data *data;
    json_group *json_body;

    data = static_cast<api_json_async_data*>(user_data);
    json_body = ft_nullptr;
    if (body)
    {
        json_body = json_read_from_string(body);
        cma_free(body);
    }
    if (data && data->callback)
        data->callback(json_body, status, data->user_data);
    if (data)
        cma_free(data);
    return ;
}

bool    api_request_json_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    api_json_async_data *data;

    if (!host || !method || !path || !callback)
        return (false);
    data = static_cast<api_json_async_data*>(cma_malloc(sizeof(api_json_async_data)));
    if (!data)
        return (false);
    data->callback = callback;
    data->user_data = user_data;
    if (!api_request_string_tls_async(host, port, method, path, api_json_async_wrapper,
            data, payload, headers, timeout))
    {
        cma_free(data);
        return (false);
    }
    return (true);
}

#endif
