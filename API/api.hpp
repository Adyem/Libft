#ifndef API_WRAPPER_HPP
#define API_WRAPPER_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdint>
#include <cstddef>

typedef void (*api_callback)(char *body, int status, void *user_data);

bool    api_request_string_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);

bool    api_request_string_tls_async(const char *host, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int timeout = 60000);

char    *api_request_string(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

char    *api_request_string_bearer(const char *ip, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_basic(const char *ip, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

char    *api_request_string_host(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

char    *api_request_string_host_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_host_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

json_group *api_request_json_bearer(const char *ip, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_basic(const char *ip, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

json_group *api_request_json_host(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = ft_nullptr,
        const char *headers = ft_nullptr, int *status = ft_nullptr,
        int timeout = 60000);

json_group *api_request_json_host_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_host_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_tls_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_tls_bearer(const char *host, uint16_t port,
        const char *method, const char *path, const char *token,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_tls_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_tls_basic(const char *host, uint16_t port,
        const char *method, const char *path, const char *credentials,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

char    *api_request_string_url(const char *url, const char *method,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

json_group *api_request_json_url(const char *url, const char *method,
        json_group *payload = ft_nullptr, const char *headers = ft_nullptr,
        int *status = ft_nullptr, int timeout = 60000);

#endif
