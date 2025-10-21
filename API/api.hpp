#ifndef API_WRAPPER_HPP
#define API_WRAPPER_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>
#include <cstddef>

typedef void (*api_callback)(char *body, int status, void *user_data);
typedef void (*api_json_callback)(json_group *body, int status, void *user_data);

typedef void (*api_stream_headers_callback)(int status_code,
        const char *headers, void *user_data);
typedef bool (*api_stream_body_callback)(const char *chunk_data,
        size_t chunk_size, bool is_final_chunk, void *user_data);

struct api_streaming_handler
{
    api_stream_headers_callback headers_callback;
    api_stream_body_callback body_callback;
    void *user_data;
};

struct api_retry_policy
{
    int max_attempts;
    int initial_delay_ms;
    int max_delay_ms;
    int backoff_multiplier;
};

bool    api_request_string_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);
bool    api_request_string_http2_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000,
        bool *used_http2 = ft_nullptr);

bool    api_request_string_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);
bool    api_request_string_tls_http2_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000,
        bool *used_http2 = ft_nullptr);

bool    api_request_json_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);
bool    api_request_json_http2_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000,
        bool *used_http2 = ft_nullptr);

bool    api_request_json_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);
bool    api_request_json_tls_http2_async(const char *host, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000,
        bool *used_http2 = ft_nullptr);

bool    api_request_stream(const char *ip, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
bool    api_request_stream_http2(const char *ip, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);
bool    api_request_stream_host(const char *host, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
bool    api_request_stream_tls(const char *host, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000, const char *ca_certificate = ft_nullptr,
        bool verify_peer = true,
        const api_retry_policy *retry_policy = ft_nullptr);
bool    api_request_stream_tls_http2(const char *host, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000, const char *ca_certificate = ft_nullptr,
        bool verify_peer = true, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
char    *api_request_string_http2(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_bearer(const char *ip, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_basic(const char *ip, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_https(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, const char *ca_certificate = ft_nullptr,
        bool verify_peer = true,
        const api_retry_policy *retry_policy = ft_nullptr);
char    *api_request_https_http2(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, const char *ca_certificate = ft_nullptr,
        bool verify_peer = true, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
char    *api_request_string_tls_http2(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_host(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_host_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_host_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
json_group *api_request_json_http2(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_bearer(const char *ip, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_basic(const char *ip, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);
json_group *api_request_json_tls_http2(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000, bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_host(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_host_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_host_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_tls_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_tls_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_tls_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_tls_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

char    *api_request_string_url(const char *url, const char *method,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

json_group *api_request_json_url(const char *url, const char *method,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000,
        const api_retry_policy *retry_policy = ft_nullptr);

void    api_connection_pool_flush(void);
void    api_connection_pool_set_max_idle(size_t max_idle);
size_t  api_connection_pool_get_max_idle(void);
void    api_connection_pool_set_idle_timeout(long long idle_timeout_ms);
long long   api_connection_pool_get_idle_timeout(void);
void    api_debug_reset_connection_pool_counters(void);
size_t  api_debug_get_connection_pool_acquires(void);
size_t  api_debug_get_connection_pool_reuses(void);
size_t  api_debug_get_connection_pool_misses(void);

size_t  api_debug_get_last_async_request_size(void);
size_t  api_debug_get_last_async_bytes_sent(void);
int     api_debug_get_last_async_send_state(void);
int     api_debug_get_last_async_send_timeout(void);
size_t  api_debug_get_last_async_bytes_received(void);
int     api_debug_get_last_async_receive_state(void);
int     api_debug_get_last_async_receive_timeout(void);

#endif
