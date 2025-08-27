#ifndef API_TLS_CLIENT_HPP
#define API_TLS_CLIENT_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/string_class.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include <openssl/ssl.h>
#include <cstdint>

class api_tls_client
{
private:
    SSL_CTX *ctx;
    SSL *ssl;
    int sock;
    ft_string host;
    int timeout;

public:
    api_tls_client(const char *host, uint16_t port, int timeout = 60000);
    ~api_tls_client();
    bool is_valid() const;

    char *request(const char *method, const char *path, json_group *payload = NULL,
                  const char *headers = NULL, int *status = NULL);
};

#endif
