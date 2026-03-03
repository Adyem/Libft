#include "promise.hpp"
#include "../Networking/openssl_support.hpp"

bool api_promise::request(const char *ip, uint16_t port,
                          const char *method, const char *path,
                          json_group *payload,
                          const char *headers, int *status,
                          int timeout)
{
    json_group *resp = api_request_json(ip, port, method, path, payload,
                                        headers, status, timeout);
    if (!resp)
        return (false);
    set_value(resp);
    return (true);
}

bool api_string_promise::request(const char *ip, uint16_t port,
                                 const char *method, const char *path,
                                 json_group *payload,
                                 const char *headers, int *status,
                                 int timeout)
{
    char *resp = api_request_string(ip, port, method, path, payload,
                                    headers, status, timeout);
    if (!resp)
        return (false);
    set_value(resp);
    return (true);
}

bool api_tls_promise::request(const char *host, uint16_t port,
                              const char *method, const char *path,
                              json_group *payload,
                              const char *headers, int *status,
                              int timeout)
{
#if NETWORKING_HAS_OPENSSL
    json_group *resp = api_request_json_tls(host, port, method, path, payload,
                                            headers, status, timeout);
    if (!resp)
        return (false);
    set_value(resp);
    return (true);
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)payload;
    (void)headers;
    (void)status;
    (void)timeout;
    return (false);
#endif
}

bool api_tls_string_promise::request(const char *host, uint16_t port,
                                     const char *method, const char *path,
                                     json_group *payload,
                                     const char *headers, int *status,
                                     int timeout)
{
#if NETWORKING_HAS_OPENSSL
    char *resp = api_request_string_tls(host, port, method, path, payload,
                                        headers, status, timeout);
    if (!resp)
        return (false);
    set_value(resp);
    return (true);
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)payload;
    (void)headers;
    (void)status;
    (void)timeout;
    return (false);
#endif
}
