#include "api.hpp"
#include "api_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/openssl_support.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"
#include "../Threading/thread.hpp"
#include "../Time/time.hpp"
#include <atomic>
#include <errno.h>
#include <limits.h>

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
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

static std::atomic<ft_size_t> g_api_async_last_request_size(0);
static std::atomic<ft_size_t> g_api_async_last_bytes_sent(0);
static std::atomic<ft_size_t> g_api_async_last_bytes_received(0);
static std::atomic<int32_t> g_api_async_last_send_state(0);
static std::atomic<int32_t> g_api_async_last_send_timeout(0);
static std::atomic<int32_t> g_api_async_last_receive_state(0);
static std::atomic<int32_t> g_api_async_last_receive_timeout(0);

static ft_bool api_request_string_ends_with_crlf(const ft_string &value)
{
    ft_size_t length;
    const char *character_pointer;

    length = value.size();
    if (length < 2)
        return (FT_FALSE);
    character_pointer = value.at(length - 2);
    if (character_pointer == ft_nullptr)
        return (FT_FALSE);
    if (*character_pointer != '\r')
        return (FT_FALSE);
    character_pointer = value.at(length - 1);
    if (character_pointer == ft_nullptr)
        return (FT_FALSE);
    if (*character_pointer != '\n')
        return (FT_FALSE);
    return (FT_TRUE);
}

static void api_request_trim_header_block(ft_string &headers)
{
    ft_size_t length;
    const char *character_pointer;

    length = headers.size();
    while (length > 0)
    {
        character_pointer = headers.at(length - 1);
        if (character_pointer == ft_nullptr)
            break ;
        if (*character_pointer == '\r' || *character_pointer == '\n')
        {
            headers.erase(length - 1, 1);
            length -= 1;
            continue;
        }
        break ;
    }
    length = headers.size();
    while (length > 0)
    {
        character_pointer = headers.at(0);
        if (character_pointer == ft_nullptr)
            break ;
        if (*character_pointer == '\r' || *character_pointer == '\n')
        {
            headers.erase(0, 1);
            length -= 1;
            continue;
        }
        break ;
    }
    return ;
}

static void api_request_append_header_block(ft_string &request, const ft_string &block)
{
    if (block.empty())
        return ;
    if (!api_request_string_ends_with_crlf(request))
        request += "\r\n";
    request += block;
    return ;
}

static void api_request_reset_async_debug_counters(void)
{
    g_api_async_last_request_size.store(0);
    g_api_async_last_bytes_sent.store(0);
    g_api_async_last_bytes_received.store(0);
    g_api_async_last_send_state.store(0);
    g_api_async_last_send_timeout.store(0);
    g_api_async_last_receive_state.store(0);
    g_api_async_last_receive_timeout.store(0);
    return ;
}

struct api_async_request
{
    char *ip_address;
    uint16_t port;
    char *method;
    char *path;
    json_group *payload;
    char *headers;
    int32_t timeout;
    api_callback callback;
    void *user_data;
};

static ft_bool api_async_build_request(const api_async_request &data,
        ft_string &request)
{
    ft_string body_string;
    char *temporary_string;
    if (request.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    request.clear();
    if (body_string.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    body_string.clear();
    if (!data.method || !data.path || !data.ip_address)
    {
        return (FT_FALSE);
    }
    request += data.method;
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    request += " ";
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    request += data.path;
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    request += " HTTP/1.1\r\nHost: ";
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    request += data.ip_address;
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    if (data.headers && data.headers[0])
    {
        ft_string headers_string;

        headers_string = data.headers;
        if (headers_string.get_error() != FT_ERR_SUCCESS)
        {
            request.clear();
            return (FT_FALSE);
        }
        api_request_trim_header_block(headers_string);
        if (!headers_string.empty())
        {
            api_request_append_header_block(request, headers_string);
            if (request.get_error() != FT_ERR_SUCCESS)
            {
                request.clear();
                return (FT_FALSE);
            }
        }
    }
    if (data.payload)
    {
        temporary_string = json_write_to_string(data.payload);
        if (!temporary_string)
        {
            request.clear();
            return (FT_FALSE);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        if (body_string.get_error() != FT_ERR_SUCCESS)
        {
            request.clear();
            return (FT_FALSE);
        }
        request += "\r\nContent-Type: application/json";
        if (request.get_error() != FT_ERR_SUCCESS)
        {
            request.clear();
            return (FT_FALSE);
        }
        if (!api_append_content_length_header(request, body_string.size()))
        {
            request.clear();
            return (FT_FALSE);
        }
    }
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (request.get_error() != FT_ERR_SUCCESS)
    {
        request.clear();
        return (FT_FALSE);
    }
    if (data.payload)
    {
        request += body_string.c_str();
        if (request.get_error() != FT_ERR_SUCCESS)
        {
            request.clear();
            return (FT_FALSE);
        }
    }
    return (FT_TRUE);
}

static int32_t api_async_calculate_send_timeout(const ft_string &request,
        int32_t timeout_ms)
{
    int32_t computed_timeout;
    ft_size_t request_size;
    ft_size_t request_kib;

    computed_timeout = timeout_ms;
    request_size = request.size();
    if (computed_timeout > 0)
    {
        request_kib = request_size / 1024;
        if ((request_size % 1024) != 0)
            request_kib += 1;
        if (request_kib > 0)
        {
            if (request_kib > static_cast<ft_size_t>(INT_MAX))
                computed_timeout = INT_MAX;
            else if (static_cast<int32_t>(request_kib) > computed_timeout)
                computed_timeout = static_cast<int32_t>(request_kib);
        }
    }
    if (computed_timeout < 10000)
        computed_timeout = 10000;
    return (computed_timeout);
}

static int32_t api_async_calculate_receive_timeout(
        const ft_string &request, int32_t timeout_ms, int32_t send_timeout_ms)
{
    int64_t computed_timeout;
    ft_size_t request_size;
    ft_size_t request_kib;
    int64_t extra_timeout;

    computed_timeout = timeout_ms;
    if (computed_timeout < 0)
        computed_timeout = 0;
    if (computed_timeout < send_timeout_ms)
        computed_timeout = send_timeout_ms;
    request_size = request.size();
    extra_timeout = 0;
    if (request_size > 0)
    {
        request_kib = request_size / 1024;
        if ((request_size % 1024) != 0)
            request_kib += 1;
        if (request_kib > 0)
        {
            if (request_kib > static_cast<ft_size_t>(INT_MAX))
                extra_timeout = INT_MAX;
            else
                extra_timeout = static_cast<int64_t>(request_kib);
        }
    }
    if (extra_timeout > 0)
    {
        if (computed_timeout > static_cast<int64_t>(INT_MAX) - extra_timeout)
            computed_timeout = INT_MAX;
        else
            computed_timeout += extra_timeout;
    }
    if (computed_timeout < 15000)
        computed_timeout = 15000;
    if (computed_timeout > INT_MAX)
        computed_timeout = INT_MAX;
    return (static_cast<int32_t>(computed_timeout));
}

static void api_async_worker(api_async_request *data)
{
    ft_string request_string;
    api_retry_policy retry_policy;
    int32_t status;
    char *result_body;
    ft_bool request_prepared;
    int32_t send_timeout_ms;
    int32_t receive_timeout_ms;
    ft_size_t body_length;

    if (!data)
        return ;
    status = -1;
    result_body = ft_nullptr;
    api_request_reset_async_debug_counters();
    request_prepared = api_async_build_request(*data, request_string);
    g_api_async_last_request_size.store(request_string.size());
    send_timeout_ms = api_async_calculate_send_timeout(request_string,
            data->timeout);
    g_api_async_last_send_timeout.store(send_timeout_ms);
    receive_timeout_ms = api_async_calculate_receive_timeout(
            request_string, data->timeout, send_timeout_ms);
    g_api_async_last_receive_timeout.store(receive_timeout_ms);
    if (retry_policy.initialize() == FT_ERR_SUCCESS)
    {
        retry_policy.set_max_attempts(10);
        retry_policy.set_initial_delay_ms(100);
        retry_policy.set_max_delay_ms(500);
        retry_policy.set_backoff_multiplier(2);
    }
    if (request_prepared)
    {
        result_body = api_request_string_host(data->ip_address, data->port,
                data->method, data->path, data->payload, data->headers,
                &status, receive_timeout_ms, &retry_policy);
    }
    (void)retry_policy.destroy();
    if (result_body)
    {
        body_length = ft_strlen(result_body);
        g_api_async_last_bytes_sent.store(request_string.size());
        g_api_async_last_send_state.store(5);
        g_api_async_last_bytes_received.store(body_length);
        g_api_async_last_receive_state.store(5);
    }
    else
    {
        g_api_async_last_bytes_sent.store(0);
        g_api_async_last_send_state.store(9);
        g_api_async_last_bytes_received.store(0);
        g_api_async_last_receive_state.store(9);
    }
    if (data->callback)
        data->callback(result_body, status, data->user_data);
    if (data->ip_address)
        cma_free(data->ip_address);
    if (data->method)
        cma_free(data->method);
    if (data->path)
        cma_free(data->path);
    if (data->headers)
        cma_free(data->headers);
    cma_free(data);
    return ;
}

ft_size_t  api_debug_get_last_async_request_size(void)
{
    return (g_api_async_last_request_size.load());
}

ft_size_t  api_debug_get_last_async_bytes_sent(void)
{
    return (g_api_async_last_bytes_sent.load());
}

int32_t     api_debug_get_last_async_send_state(void)
{
    return (g_api_async_last_send_state.load());
}

int32_t     api_debug_get_last_async_send_timeout(void)
{
    return (g_api_async_last_send_timeout.load());
}

ft_size_t  api_debug_get_last_async_bytes_received(void)
{
    return (g_api_async_last_bytes_received.load());
}

int32_t     api_debug_get_last_async_receive_state(void)
{
    return (g_api_async_last_receive_state.load());
}

int32_t     api_debug_get_last_async_receive_timeout(void)
{
    return (g_api_async_last_receive_timeout.load());
}

ft_bool    api_request_string_async(const char *ip_address, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout)
{
    if (!ip_address || !method || !path || !callback)
    {
        return (FT_FALSE);
    }
    const api_transport_hooks *hooks;

    hooks = api_get_transport_hooks();
    if (hooks && hooks->request_string_async)
    {
        return (hooks->request_string_async(ip_address, port, method, path, callback,
                user_data, payload, headers, timeout, hooks->user_data));
    }
    api_async_request *data = static_cast<api_async_request*>(cma_malloc(sizeof(api_async_request)));
    if (!data)
    {
        return (FT_FALSE);
    }
    ft_bzero(data, sizeof(api_async_request));
    data->ip_address = adv_strdup(ip_address);
    data->method = adv_strdup(method);
    data->path = adv_strdup(path);
    if (headers)
        data->headers = adv_strdup(headers);
    data->port = port;
    data->payload = payload;
    data->timeout = timeout;
    data->callback = callback;
    data->user_data = user_data;
    if (!data->ip_address || !data->method || !data->path || (headers && !data->headers))
    {
        if (data->ip_address)
            cma_free(data->ip_address);
        if (data->method)
            cma_free(data->method);
        if (data->path)
            cma_free(data->path);
        if (data->headers)
            cma_free(data->headers);
        cma_free(data);
        return (FT_FALSE);
    }
    ft_thread thread_worker(api_async_worker, data);
    thread_worker.detach();
    return (FT_TRUE);
}

struct api_json_async_data
{
    api_json_callback callback;
    void *user_data;
};

static void api_json_async_wrapper(char *body, int32_t status, void *user_data)
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

ft_bool    api_request_json_async(const char *ip_address, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout)
{
    api_json_async_data *data;

    if (!ip_address || !method || !path || !callback)
    {
        return (FT_FALSE);
    }
    data = static_cast<api_json_async_data*>(cma_malloc(sizeof(api_json_async_data)));
    if (!data)
    {
        return (FT_FALSE);
    }
    data->callback = callback;
    data->user_data = user_data;
    if (!api_request_string_async(ip_address, port, method, path, api_json_async_wrapper,
            data, payload, headers, timeout))
    {
        cma_free(data);
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_bool    api_request_string_http2_async(const char *ip_address, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout,
        ft_bool *used_http2)
{
    if (used_http2)
        *used_http2 = FT_FALSE;
    return (api_request_string_async(ip_address, port, method, path, callback,
            user_data, payload, headers, timeout));
}

ft_bool    api_request_string_tls_http2_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout,
        ft_bool *used_http2)
{
    if (used_http2)
        *used_http2 = FT_FALSE;
#if NETWORKING_HAS_OPENSSL
    return (api_request_string_tls_async(host, port, method, path, callback,
            user_data, payload, headers, timeout));
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)callback;
    (void)user_data;
    (void)payload;
    (void)headers;
    (void)timeout;
    return (FT_FALSE);
#endif
}

ft_bool    api_request_json_http2_async(const char *ip_address, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout,
        ft_bool *used_http2)
{
    if (used_http2)
        *used_http2 = FT_FALSE;
    return (api_request_json_async(ip_address, port, method, path, callback,
            user_data, payload, headers, timeout));
}

ft_bool    api_request_json_tls_http2_async(const char *host, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int32_t timeout,
        ft_bool *used_http2)
{
    if (used_http2)
        *used_http2 = FT_FALSE;
#if NETWORKING_HAS_OPENSSL
    return (api_request_json_tls_async(host, port, method, path, callback,
            user_data, payload, headers, timeout));
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)callback;
    (void)user_data;
    (void)payload;
    (void)headers;
    (void)timeout;
    return (FT_FALSE);
#endif
}
