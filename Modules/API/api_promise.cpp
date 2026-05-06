#include "promise.hpp"
#include "../Networking/openssl_support.hpp"
#include "../Template/move.hpp"

api_promise::api_promise() noexcept
{
    return ;
}

api_promise::api_promise(const api_promise &other) noexcept
    : ft_promise<json_group*>(other)
{
    return ;
}

api_promise::api_promise(api_promise &&other) noexcept
    : ft_promise<json_group*>(ft_move(other))
{
    return ;
}

api_promise::~api_promise() noexcept
{
    return ;
}

ft_bool api_promise::request(const char *ip_address, uint16_t port,
                          const char *method, const char *path,
                          json_group *payload,
                          const char *headers, int32_t *status,
                          int32_t timeout)
{
    json_group *response_body = api_request_json(ip_address, port, method, path, payload,
                                        headers, status, timeout);
    if (!response_body)
        return (FT_FALSE);
    set_value(response_body);
    return (FT_TRUE);
}

api_string_promise::api_string_promise() noexcept
{
    return ;
}

api_string_promise::api_string_promise(const api_string_promise &other) noexcept
    : ft_promise<char*>(other)
{
    return ;
}

api_string_promise::api_string_promise(api_string_promise &&other) noexcept
    : ft_promise<char*>(ft_move(other))
{
    return ;
}

api_string_promise::~api_string_promise() noexcept
{
    return ;
}

ft_bool api_string_promise::request(const char *ip_address, uint16_t port,
                                 const char *method, const char *path,
                                 json_group *payload,
                                 const char *headers, int32_t *status,
                                 int32_t timeout)
{
    char *response_body = api_request_string(ip_address, port, method, path, payload,
                                    headers, status, timeout);
    if (!response_body)
        return (FT_FALSE);
    set_value(response_body);
    return (FT_TRUE);
}

api_tls_promise::api_tls_promise() noexcept
{
    return ;
}

api_tls_promise::api_tls_promise(const api_tls_promise &other) noexcept
    : ft_promise<json_group*>(other)
{
    return ;
}

api_tls_promise::api_tls_promise(api_tls_promise &&other) noexcept
    : ft_promise<json_group*>(ft_move(other))
{
    return ;
}

api_tls_promise::~api_tls_promise() noexcept
{
    return ;
}

ft_bool api_tls_promise::request(const char *host, uint16_t port,
                              const char *method, const char *path,
                              json_group *payload,
                              const char *headers, int32_t *status,
                              int32_t timeout)
{
#if NETWORKING_HAS_OPENSSL
    json_group *response_body = api_request_json_tls(host, port, method, path, payload,
                                            headers, status, timeout);
    if (!response_body)
        return (FT_FALSE);
    set_value(response_body);
    return (FT_TRUE);
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)payload;
    (void)headers;
    (void)status;
    (void)timeout;
    return (FT_FALSE);
#endif
}

api_tls_string_promise::api_tls_string_promise() noexcept
{
    return ;
}

api_tls_string_promise::api_tls_string_promise(const api_tls_string_promise &other) noexcept
    : ft_promise<char*>(other)
{
    return ;
}

api_tls_string_promise::api_tls_string_promise(api_tls_string_promise &&other) noexcept
    : ft_promise<char*>(ft_move(other))
{
    return ;
}

api_tls_string_promise::~api_tls_string_promise() noexcept
{
    return ;
}

ft_bool api_tls_string_promise::request(const char *host, uint16_t port,
                                     const char *method, const char *path,
                                     json_group *payload,
                                     const char *headers, int32_t *status,
                                     int32_t timeout)
{
#if NETWORKING_HAS_OPENSSL
    char *response_body = api_request_string_tls(host, port, method, path, payload,
                                        headers, status, timeout);
    if (!response_body)
        return (FT_FALSE);
    set_value(response_body);
    return (FT_TRUE);
#else
    (void)host;
    (void)port;
    (void)method;
    (void)path;
    (void)payload;
    (void)headers;
    (void)status;
    (void)timeout;
    return (FT_FALSE);
#endif
}
