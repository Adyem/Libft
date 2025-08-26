#include "api_promise.hpp"

bool api_promise::request(const char *ip, uint16_t port,
                          const char *method, const char *path,
                          json_group *payload,
                          const char *headers, int *status)
{
    json_group *resp = api_request_json(ip, port, method, path, payload,
                                        headers, status);
    if (!resp)
        return (false);
    set_value(resp);
    return (true);
}
