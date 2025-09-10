#ifndef API_TLS_CLIENT_HPP
#define API_TLS_CLIENT_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <openssl/ssl.h>
#include <cstdint>

typedef void (*api_callback)(char *body, int status, void *user_data);

class api_tls_client
{
    private:
        SSL_CTX *_ctx;
        SSL *_ssl;
        int _sock;
        ft_string _host;
        int _timeout;

    public:
        api_tls_client(const char *host, uint16_t port, int timeout = 60000);
        ~api_tls_client();
        bool is_valid() const;

        char *request(const char *method, const char *path, json_group *payload = ft_nullptr,
                      const char *headers = ft_nullptr, int *status = ft_nullptr);

        json_group *request_json(const char *method, const char *path,
                                 json_group *payload = ft_nullptr,
                                 const char *headers = ft_nullptr, int *status = ft_nullptr);

        bool request_async(const char *method, const char *path,
                           json_group *payload = ft_nullptr,
                           const char *headers = ft_nullptr,
                           api_callback callback = ft_nullptr,
                           void *user_data = ft_nullptr);
};

#endif
