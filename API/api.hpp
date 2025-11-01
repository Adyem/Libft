#ifndef API_WRAPPER_HPP
#define API_WRAPPER_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "api_request_signing.hpp"
#include <cstdint>
#include <cstddef>

typedef void (*api_callback)(char *body, int status, void *user_data);
typedef void (*api_json_callback)(json_group *body, int status, void *user_data);

typedef void (*api_stream_headers_callback)(int status_code,
        const char *headers, void *user_data);
typedef bool (*api_stream_body_callback)(const char *chunk_data,
        size_t chunk_size, bool is_final_chunk, void *user_data);

class api_streaming_handler
{
    private:
        api_stream_headers_callback _headers_callback;
        api_stream_body_callback _body_callback;
        void *_user_data;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const api_streaming_handler &first,
            const api_streaming_handler &second,
            ft_unique_lock<pt_mutex> &first_guard,
            ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
        api_streaming_handler() noexcept;
        api_streaming_handler(const api_streaming_handler &other) noexcept;
        api_streaming_handler &operator=(const api_streaming_handler &other) noexcept;
        api_streaming_handler(api_streaming_handler &&other) noexcept;
        api_streaming_handler &operator=(api_streaming_handler &&other) noexcept;
        ~api_streaming_handler();

        void reset() noexcept;

        void set_headers_callback(api_stream_headers_callback callback) noexcept;
        void set_body_callback(api_stream_body_callback callback) noexcept;
        void set_user_data(void *user_data) noexcept;

        bool invoke_headers_callback(int status_code,
            const char *headers) const noexcept;
        bool invoke_body_callback(const char *chunk_data, size_t chunk_size,
            bool is_final_chunk, bool &should_continue) const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

class api_retry_policy
{
    private:
        int _max_attempts;
        int _initial_delay_ms;
        int _max_delay_ms;
        int _backoff_multiplier;
        int _circuit_breaker_threshold;
        int _circuit_breaker_cooldown_ms;
        int _circuit_breaker_half_open_successes;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const noexcept;
        static int lock_pair(const api_retry_policy &first,
            const api_retry_policy &second,
            ft_unique_lock<pt_mutex> &first_guard,
            ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
        api_retry_policy() noexcept;
        api_retry_policy(const api_retry_policy &other) noexcept;
        api_retry_policy &operator=(const api_retry_policy &other) noexcept;
        api_retry_policy(api_retry_policy &&other) noexcept;
        api_retry_policy &operator=(api_retry_policy &&other) noexcept;
        ~api_retry_policy();

        void reset() noexcept;

        void set_max_attempts(int value) noexcept;
        void set_initial_delay_ms(int value) noexcept;
        void set_max_delay_ms(int value) noexcept;
        void set_backoff_multiplier(int value) noexcept;
        void set_circuit_breaker_threshold(int value) noexcept;
        void set_circuit_breaker_cooldown_ms(int value) noexcept;
        void set_circuit_breaker_half_open_successes(int value) noexcept;

        int get_max_attempts() const noexcept;
        int get_initial_delay_ms() const noexcept;
        int get_max_delay_ms() const noexcept;
        int get_backoff_multiplier() const noexcept;
        int get_circuit_breaker_threshold() const noexcept;
        int get_circuit_breaker_cooldown_ms() const noexcept;
        int get_circuit_breaker_half_open_successes() const noexcept;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

struct api_transport_hooks
{
    bool (*request_stream)(const char *ip, uint16_t port,
            const char *method, const char *path,
            const api_streaming_handler *streaming_handler,
            json_group *payload, const char *headers, int timeout,
            const api_retry_policy *retry_policy, void *user_data);
    bool (*request_stream_http2)(const char *ip, uint16_t port,
            const char *method, const char *path,
            const api_streaming_handler *streaming_handler,
            json_group *payload, const char *headers, int timeout,
            bool *used_http2, const api_retry_policy *retry_policy,
            void *user_data);
    bool (*request_stream_host)(const char *host, uint16_t port,
            const char *method, const char *path,
            const api_streaming_handler *streaming_handler,
            json_group *payload, const char *headers, int timeout,
            const api_retry_policy *retry_policy, void *user_data);
    bool (*request_stream_tls)(const char *host, uint16_t port,
            const char *method, const char *path,
            const api_streaming_handler *streaming_handler,
            json_group *payload, const char *headers, int timeout,
            const char *ca_certificate, bool verify_peer,
            const api_retry_policy *retry_policy, void *user_data);
    bool (*request_stream_tls_http2)(const char *host, uint16_t port,
            const char *method, const char *path,
            const api_streaming_handler *streaming_handler,
            json_group *payload, const char *headers, int timeout,
            const char *ca_certificate, bool verify_peer, bool *used_http2,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_string)(const char *ip, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_string_http2)(const char *ip, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout, bool *used_http2,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_string_host)(const char *host, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_string_tls)(const char *host, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_string_tls_http2)(const char *host, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout, bool *used_http2,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_https)(const char *ip, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout,
            const char *ca_certificate, bool verify_peer,
            const api_retry_policy *retry_policy, void *user_data);
    char *(*request_https_http2)(const char *ip, uint16_t port,
            const char *method, const char *path, json_group *payload,
            const char *headers, int *status, int timeout,
            const char *ca_certificate, bool verify_peer, bool *used_http2,
            const api_retry_policy *retry_policy, void *user_data);
    bool (*request_string_async)(const char *ip, uint16_t port,
            const char *method, const char *path, api_callback callback,
            void *user_data, json_group *payload, const char *headers,
            int timeout, void *transport_user_data);
    bool (*request_string_tls_async)(const char *host, uint16_t port,
            const char *method, const char *path, api_callback callback,
            void *user_data, json_group *payload, const char *headers,
            int timeout, void *transport_user_data);
    void *user_data;
};

void    api_set_transport_hooks(const api_transport_hooks *hooks);
void    api_clear_transport_hooks(void);
const api_transport_hooks    *api_get_transport_hooks(void);

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

bool    api_request(const char *ip, uint16_t port,
        const char *method, const char *path,
        const api_streaming_handler *streaming_handler,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int timeout = 60000, bool enable_http2 = true,
        bool *used_http2 = ft_nullptr,
        const api_retry_policy *retry_policy = ft_nullptr);
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
