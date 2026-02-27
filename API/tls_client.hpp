#ifndef API_TLS_CLIENT_HPP
#define API_TLS_CLIENT_HPP

#include "../JSon/json.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/thread.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/vector.hpp"
#include "../Networking/openssl_support.hpp"
#include <cstdint>

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

#if NETWORKING_HAS_OPENSSL
class api_tls_client
{
    private:
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        SSL_CTX *_ctx;
        SSL *_ssl;
        int _sock;
        ft_string _host;
        uint16_t _port;
        int _timeout;
        mutable pt_recursive_mutex *_mutex;
        bool _is_shutting_down;
        ft_vector<ft_thread> _async_workers;
        api_tls_handshake_diagnostics _handshake_diagnostics;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
        bool populate_handshake_diagnostics();

    public:
        api_tls_client(const char *host, uint16_t port, int timeout = 60000);
        api_tls_client(const api_tls_client &other) = delete;
        api_tls_client &operator=(const api_tls_client &other) = delete;
        api_tls_client(api_tls_client &&other) = delete;
        api_tls_client &operator=(api_tls_client &&other) = delete;
        ~api_tls_client();
        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        bool is_valid() const;

        char *request(const char *method, const char *path,
                json_group *payload = ft_nullptr,
                      const char *headers = ft_nullptr, int *status = ft_nullptr);

        json_group *request_json(const char *method, const char *path,
                                 json_group *payload = ft_nullptr,
                                 const char *headers = ft_nullptr,
                                 int *status = ft_nullptr);

        bool request_async(const char *method, const char *path,
                           json_group *payload = ft_nullptr,
                           const char *headers = ft_nullptr,
                           api_callback callback = ft_nullptr,
                           void *user_data = ft_nullptr);
        bool refresh_handshake_diagnostics();
        const api_tls_handshake_diagnostics &get_handshake_diagnostics() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};
#endif

#endif
