#ifndef API_PROMISE_HPP
#define API_PROMISE_HPP

#include "../Template/promise.hpp"
#include "api.hpp"

class api_promise : public ft_promise<json_group*>
{
public:
    bool request(const char *ip, uint16_t port,
                 const char *method, const char *path,
                 json_group *payload = NULL,
                 const char *headers = NULL, int *status = NULL);
};

#endif
