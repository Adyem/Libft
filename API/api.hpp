#ifndef API_WRAPPER_HPP
#define API_WRAPPER_HPP

#include "../JSon/json.hpp"
#include <cstdint>
#include <cstddef>

char    *api_request_string(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = NULL,
        const char *headers = NULL, int *status = NULL,
        int timeout = 60000);

char    *api_request_string_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = NULL,
        const char *headers = NULL, int *status = NULL,
        int timeout = 60000);

json_group *api_request_json(const char *ip, uint16_t port,
        const char *method, const char *path, json_group *payload = NULL,
        const char *headers = NULL, int *status = NULL,
        int timeout = 60000);

json_group *api_request_json_tls(const char *host, uint16_t port,
        const char *method, const char *path, json_group *payload = NULL,
        const char *headers = NULL, int *status = NULL,
        int timeout = 60000);

#endif
