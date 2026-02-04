#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../CPP_class/class_string.hpp"
#include "ssl_wrapper.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
typedef void (*http_response_handler)(int status_code, const ft_string &headers,
    const char *body_chunk, size_t chunk_size, bool finished);

int http_get_stream(const char *host, const char *path, http_response_handler handler,
    bool use_ssl = false, const char *custom_port = NULL);
int http_get(const char *host, const char *path, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl = false, const char *custom_port = NULL);
int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length);
int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length);
void http_client_pool_flush(void);
void http_client_pool_set_max_idle(size_t max_idle);
size_t http_client_pool_get_idle_count(void);
void http_client_pool_debug_reset_counters(void);
size_t http_client_pool_debug_get_reuse_count(void);
size_t http_client_pool_debug_get_miss_count(void);
#endif

#endif
