#ifndef API_PROMISE_HPP
#define API_PROMISE_HPP

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/promise.hpp"
#include "api.hpp"

class api_promise : public ft_promise<json_group*>
{
    public:
        api_promise() noexcept;
        api_promise(const api_promise &other) noexcept = delete;
        api_promise(api_promise &&other) noexcept = delete;
        ~api_promise() noexcept;
        api_promise &operator=(const api_promise &other) noexcept = delete;
        api_promise &operator=(api_promise &&other) noexcept = delete;

        ft_bool request(const char *ip_address, uint16_t port,
                     const char *method, const char *path,
                     json_group *payload = ft_nullptr,
                     const char *headers = ft_nullptr, int32_t *status = ft_nullptr,
                     int32_t timeout = 60000);
};

class api_string_promise : public ft_promise<char*>
{
    public:
        api_string_promise() noexcept;
        api_string_promise(const api_string_promise &other) noexcept = delete;
        api_string_promise(api_string_promise &&other) noexcept = delete;
        ~api_string_promise() noexcept;
        api_string_promise &operator=(const api_string_promise &other) noexcept = delete;
        api_string_promise &operator=(api_string_promise &&other) noexcept = delete;

        ft_bool request(const char *ip_address, uint16_t port,
                     const char *method, const char *path,
                     json_group *payload = ft_nullptr,
                     const char *headers = ft_nullptr, int32_t *status = ft_nullptr,
                     int32_t timeout = 60000);
};

class api_tls_promise : public ft_promise<json_group*>
{
    public:
        api_tls_promise() noexcept;
        api_tls_promise(const api_tls_promise &other) noexcept = delete;
        api_tls_promise(api_tls_promise &&other) noexcept = delete;
        ~api_tls_promise() noexcept;
        api_tls_promise &operator=(const api_tls_promise &other) noexcept = delete;
        api_tls_promise &operator=(api_tls_promise &&other) noexcept = delete;

        ft_bool request(const char *host, uint16_t port,
                     const char *method, const char *path,
                     json_group *payload = ft_nullptr,
                     const char *headers = ft_nullptr, int32_t *status = ft_nullptr,
                     int32_t timeout = 60000);
};

class api_tls_string_promise : public ft_promise<char*>
{
    public:
        api_tls_string_promise() noexcept;
        api_tls_string_promise(const api_tls_string_promise &other) noexcept = delete;
        api_tls_string_promise(api_tls_string_promise &&other) noexcept = delete;
        ~api_tls_string_promise() noexcept;
        api_tls_string_promise &operator=(const api_tls_string_promise &other) noexcept = delete;
        api_tls_string_promise &operator=(api_tls_string_promise &&other) noexcept = delete;

        ft_bool request(const char *host, uint16_t port,
                     const char *method, const char *path,
                     json_group *payload = ft_nullptr,
                     const char *headers = ft_nullptr, int32_t *status = ft_nullptr,
                     int32_t timeout = 60000);
};

#endif
