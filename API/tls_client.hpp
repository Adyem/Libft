#ifndef API_TLS_CLIENT_HPP
#define API_TLS_CLIENT_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/thread.hpp"
#include "../PThread/mutex.hpp"
#include <atomic>
#include <openssl/ssl.h>
#include <cstdint>
#include "../Template/vector.hpp"

typedef void (*api_callback)(char *body, int status, void *user_data);

struct api_tls_certificate_diagnostics
{
    ft_string subject;
    ft_string issuer;
    ft_string serial_number;
    ft_string not_before;
    ft_string not_after;
    ft_string fingerprint_sha256;
};

struct api_tls_handshake_diagnostics
{
    ft_string protocol;
    ft_string cipher;
    ft_vector<api_tls_certificate_diagnostics> certificates;
};

class api_tls_client
{
    private:
        SSL_CTX *_ctx;
        SSL *_ssl;
        int _sock;
        ft_string _host;
        int _timeout;
        mutable std::atomic<int> _error_code;
        mutable pt_mutex _mutex;
        bool _is_shutting_down;
        ft_vector<ft_thread> _async_workers;
        api_tls_handshake_diagnostics _handshake_diagnostics;

        void set_error(int error_code) const noexcept;
        bool populate_handshake_diagnostics();

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

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        bool refresh_handshake_diagnostics();
        const api_tls_handshake_diagnostics &get_handshake_diagnostics() const noexcept;
};

#endif
