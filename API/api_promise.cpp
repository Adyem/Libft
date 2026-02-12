#include "promise.hpp"
#include "../Errno/errno.hpp"

bool api_promise::request(const char *ip, uint16_t port,
                          const char *method, const char *path,
                          json_group *payload,
                          const char *headers, int *status,
                          int timeout)
{
    json_group *resp = api_request_json(ip, port, method, path, payload,
                                        headers, status, timeout);
    int request_error;

    request_error = ft_global_error_stack_drop_last_error();
    if (!resp)
    {
        if (request_error == FT_ERR_SUCCESS)
            request_error = FT_ERR_SOCKET_CONNECT_FAILED;
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    if (request_error != FT_ERR_SUCCESS)
    {
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    set_value(resp);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
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
    int request_error;

    request_error = ft_global_error_stack_drop_last_error();
    if (!resp)
    {
        if (request_error == FT_ERR_SUCCESS)
            request_error = FT_ERR_SOCKET_CONNECT_FAILED;
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    if (request_error != FT_ERR_SUCCESS)
    {
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    set_value(resp);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (true);
}

bool api_tls_promise::request(const char *host, uint16_t port,
                              const char *method, const char *path,
                              json_group *payload,
                              const char *headers, int *status,
                              int timeout)
{
    json_group *resp = api_request_json_tls(host, port, method, path, payload,
                                            headers, status, timeout);
    int request_error;

    request_error = ft_global_error_stack_drop_last_error();
    if (!resp)
    {
        if (request_error == FT_ERR_SUCCESS)
            request_error = FT_ERR_SOCKET_CONNECT_FAILED;
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    if (request_error != FT_ERR_SUCCESS)
    {
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    set_value(resp);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (true);
}

bool api_tls_string_promise::request(const char *host, uint16_t port,
                                     const char *method, const char *path,
                                     json_group *payload,
                                     const char *headers, int *status,
                                     int timeout)
{
    char *resp = api_request_string_tls(host, port, method, path, payload,
                                        headers, status, timeout);
    int request_error;

    request_error = ft_global_error_stack_drop_last_error();
    if (!resp)
    {
        if (request_error == FT_ERR_SUCCESS)
            request_error = FT_ERR_SOCKET_CONNECT_FAILED;
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    if (request_error != FT_ERR_SUCCESS)
    {
        this->set_error(request_error);
        ft_global_error_stack_push(request_error);
        return (false);
    }
    set_value(resp);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (true);
}
