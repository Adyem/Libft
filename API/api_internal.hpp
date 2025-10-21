#ifndef API_INTERNAL_HPP
#define API_INTERNAL_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include <cstddef>
#include <cstdint>

typedef bool (*api_request_wait_until_ready_hook)(void);

void api_request_set_downgrade_wait_hook(
    api_request_wait_until_ready_hook hook);

enum class api_connection_security_mode
{
    PLAIN,
    TLS
};

struct api_connection_pool_handle
{
    ft_string key;
    ft_socket socket;
    SSL *tls_session;
    SSL_CTX *tls_context;
    api_connection_security_mode security_mode;
    bool has_socket;
    bool from_pool;
    bool should_store;
    bool negotiated_http2;
};

bool api_connection_pool_acquire(api_connection_pool_handle &handle,
    const char *host, uint16_t port,
    api_connection_security_mode security_mode,
    const char *security_identity);
void api_connection_pool_mark_idle(api_connection_pool_handle &handle);
void api_connection_pool_evict(api_connection_pool_handle &handle);
void api_connection_pool_disable_store(api_connection_pool_handle &handle);
void api_connection_pool_flush(void);
void api_connection_pool_set_enabled(bool enabled);
bool api_connection_pool_is_enabled(void);
void api_connection_pool_set_max_idle(size_t max_idle);
size_t api_connection_pool_get_max_idle(void);
void api_connection_pool_set_idle_timeout(long long idle_timeout_ms);
long long api_connection_pool_get_idle_timeout(void);
bool api_connection_pool_track_tls_session(SSL *tls_session);
bool api_connection_pool_untrack_tls_session(SSL *tls_session);
void api_debug_reset_connection_pool_counters(void);
size_t api_debug_get_connection_pool_acquires(void);
size_t api_debug_get_connection_pool_reuses(void);
size_t api_debug_get_connection_pool_misses(void);

bool api_append_content_length_header(ft_string &request, size_t content_length);
size_t api_debug_get_last_async_request_size(void);
size_t api_debug_get_last_async_bytes_sent(void);
int api_debug_get_last_async_send_state(void);
int api_debug_get_last_async_send_timeout(void);
size_t api_debug_get_last_async_bytes_received(void);
int api_debug_get_last_async_receive_state(void);
int api_debug_get_last_async_receive_timeout(void);

bool api_is_configuration_socket_error(int error_code);

#endif
