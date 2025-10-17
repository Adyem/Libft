#ifndef API_HTTP_INTERNAL_HPP
#define API_HTTP_INTERNAL_HPP

#include "api_internal.hpp"
#include "api.hpp"
#include "../JSon/json.hpp"
#include <limits.h>

class ft_string;

char *api_http_execute_plain(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, int &error_code);
char *api_https_execute(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
    const api_retry_policy *retry_policy, int &error_code);
char *api_http_execute_plain_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, bool &used_http2, int &error_code);
bool api_http_execute_plain_streaming(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, int &error_code);
bool api_http_prepare_plain_socket(api_connection_pool_handle &connection_handle,
    const char *host, uint16_t port, int timeout, int &error_code);
bool api_http_execute_plain_http2_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *host, uint16_t port,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &used_http2,
    int &error_code);
char *api_https_execute_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
    const api_retry_policy *retry_policy, bool &used_http2,
    int &error_code);
bool api_https_execute_http2_streaming(
    api_connection_pool_handle &connection_handle, const char *method,
    const char *path, const char *host_header, json_group *payload,
    const char *headers, int timeout, const char *ca_certificate,
    bool verify_peer, const char *host, uint16_t port,
    const char *security_identity, const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, bool &used_http2,
    int &error_code);
bool api_https_execute_streaming(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int timeout,
    const char *ca_certificate, bool verify_peer, const char *host,
    uint16_t port, const char *security_identity,
    const api_retry_policy *retry_policy,
    const api_streaming_handler *streaming_handler, int &error_code);
bool api_http_stream_invoke_body(const api_streaming_handler *streaming_handler,
    const char *chunk_data, size_t chunk_size, bool is_final_chunk,
    int &error_code);
void api_http_stream_invoke_headers(
    const api_streaming_handler *streaming_handler, int status_code,
    const char *headers);
bool api_http_stream_process_chunked_buffer(ft_string &buffer,
    long long &chunk_remaining, bool &trailers_pending,
    bool &final_chunk_sent,
    const api_streaming_handler *streaming_handler, int &error_code);
void api_request_set_resolve_error(int resolver_status);
void api_request_set_ssl_error(SSL *ssl_session, int operation_result);

inline int api_retry_get_max_attempts(const api_retry_policy *retry_policy)
{
    if (!retry_policy)
        return (1);
    if (retry_policy->max_attempts <= 0)
        return (1);
    return (retry_policy->max_attempts);
}

inline int api_retry_get_initial_delay(const api_retry_policy *retry_policy)
{
    if (!retry_policy)
        return (0);
    if (retry_policy->initial_delay_ms <= 0)
        return (0);
    return (retry_policy->initial_delay_ms);
}

inline int api_retry_get_max_delay(const api_retry_policy *retry_policy)
{
    if (!retry_policy)
        return (0);
    if (retry_policy->max_delay_ms <= 0)
        return (0);
    return (retry_policy->max_delay_ms);
}

inline int api_retry_get_multiplier(const api_retry_policy *retry_policy)
{
    if (!retry_policy)
        return (2);
    if (retry_policy->backoff_multiplier <= 0)
        return (2);
    return (retry_policy->backoff_multiplier);
}

inline int api_retry_prepare_delay(int delay, int max_delay)
{
    long long prepared_delay;

    if (delay <= 0)
        return (0);
    prepared_delay = static_cast<long long>(delay);
    if (max_delay > 0 && prepared_delay > max_delay)
        prepared_delay = max_delay;
    if (prepared_delay > INT_MAX)
        prepared_delay = INT_MAX;
    return (static_cast<int>(prepared_delay));
}

inline int api_retry_next_delay(int current_delay, int max_delay,
    int multiplier)
{
    long long next_delay;

    if (current_delay <= 0)
        return (current_delay);
    next_delay = static_cast<long long>(current_delay);
    if (multiplier > 1)
        next_delay *= static_cast<long long>(multiplier);
    if (max_delay > 0 && next_delay > max_delay)
        next_delay = max_delay;
    if (next_delay > INT_MAX)
        next_delay = INT_MAX;
    return (static_cast<int>(next_delay));
}

#endif
