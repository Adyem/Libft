#ifndef API_WRAPPER_HPP
#define API_WRAPPER_HPP

#include "../JSon/json.hpp"
#include <cstdint>

json_group *api_request_json(const char *ip, uint16_t port,
                             const char *method, const char *path,
                             json_group *payload);
json_group *api_get_json(const char *ip, uint16_t port, const char *path);

#endif
