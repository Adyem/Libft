#ifndef API_HTTP_INTERNAL_HPP
#define API_HTTP_INTERNAL_HPP

#include "api_internal.hpp"
#include "../JSon/json.hpp"

char *api_http_execute_plain(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    int &error_code);
char *api_https_execute(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, int &error_code);
char *api_http_execute_plain_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    bool &used_http2, int &error_code);
char *api_https_execute_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *ca_certificate, bool verify_peer, bool &used_http2,
    int &error_code);
void api_request_set_resolve_error(int resolver_status);
void api_request_set_ssl_error(SSL *ssl_session, int operation_result);

#endif
