#include "tls_client.hpp"
#include "api_internal.hpp"
#include "../Printf/printf.hpp"
#include "../Errno/errno.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Networking/networking.hpp"
#include "../Networking/openssl_support.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../Template/move.hpp"
#include "../PThread/unique_lock.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#endif
#if NETWORKING_HAS_OPENSSL
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <cstdint>
#include <utility>
#endif

#if NETWORKING_HAS_OPENSSL

static const size_t TLS_STRING_NPOS = static_cast<size_t>(-1);

thread_local ft_operation_error_stack api_tls_client::_operation_errors = {{}, {}, 0};

void api_tls_client::record_operation_error_unlocked(int error_code) noexcept
{
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(api_tls_client::_operation_errors, error_code, operation_id);
    return ;
}

static char tls_string_char_at(const ft_string &value, size_t index)
{
    const char *data;

    if (index >= value.size())
        return ('\0');
    data = value.c_str();
    return (data[index]);
}

static ft_string tls_string_substr(const ft_string &value, size_t start_index, size_t length)
{
    ft_string result;
    size_t total_length;
    size_t copy_length;
    const char *data;
    size_t index;

    total_length = value.size();
    if (start_index >= total_length)
        return (result);
    data = value.c_str();
    copy_length = length;
    if (start_index + copy_length > total_length)
        copy_length = total_length - start_index;
    index = 0;
    while (index < copy_length)
    {
        result.append(data[start_index + index]);
        index++;
    }
    return (result);
}

static size_t tls_string_find_substring(const ft_string &value, const char *needle, size_t start_index)
{
    const char *haystack;
    const char *search_start;
    const char *found;

    if (needle == ft_nullptr)
        return (TLS_STRING_NPOS);
    if (start_index > value.size())
        return (TLS_STRING_NPOS);
    haystack = value.c_str();
    search_start = haystack + start_index;
    found = ft_strstr(search_start, needle);
    if (found == ft_nullptr)
        return (TLS_STRING_NPOS);
    return (static_cast<size_t>(found - haystack));
}

static size_t tls_string_find_char(const ft_string &value, char needle, size_t start_index)
{
    const char *haystack;
    const char *search_start;
    const char *found;

    if (start_index > value.size())
        return (TLS_STRING_NPOS);
    haystack = value.c_str();
    search_start = haystack + start_index;
    found = ft_strchr(search_start, needle);
    if (found == ft_nullptr)
        return (TLS_STRING_NPOS);
    return (static_cast<size_t>(found - haystack));
}

static char tls_ascii_to_lower(char character)
{
    if (character >= 'A' && character <= 'Z')
        character = static_cast<char>(character + 32);
    return (character);
}

static void tls_trim_whitespace(ft_string &value)
{
    size_t start_index;

    start_index = 0;
    while (start_index < value.size())
    {
        char current_char;

        current_char = tls_string_char_at(value, start_index);
        if (current_char != ' ' && current_char != '\t')
            break;
        value.erase(start_index, 1);
    }
    size_t end_index;

    end_index = value.size();
    while (end_index > 0)
    {
        char current_char;

        current_char = tls_string_char_at(value, end_index - 1);
        if (current_char != ' ' && current_char != '\t')
            break;
        value.erase(end_index - 1, 1);
        end_index--;
    }
    return ;
}

static void tls_string_to_lower(ft_string &value)
{
    ft_string lowered_value;
    size_t index;
    size_t length;

    length = value.size();
    index = 0;
    while (index < length)
    {
        char current_char;

        current_char = tls_string_char_at(value, index);
        lowered_value.append(tls_ascii_to_lower(current_char));
        index++;
    }
    value = lowered_value;
    return ;
}

static bool tls_header_equals(const ft_string &header_name, const char *target_name)
{
    size_t index;

    if (target_name == ft_nullptr)
        return (false);
    index = 0;
    while (index < header_name.size() && target_name[index] != '\0')
    {
        char left_character;
        char right_character;

        left_character = tls_ascii_to_lower(tls_string_char_at(header_name, index));
        right_character = tls_ascii_to_lower(target_name[index]);
        if (left_character != right_character)
            return (false);
        index++;
    }
    if (index != header_name.size() || target_name[index] != '\0')
        return (false);
    return (true);
}

static bool ssl_pointer_supports_network_checks(SSL *ssl)
{
    uintptr_t ssl_address;
    static const uintptr_t minimum_valid_address = 0x1000;

    if (ssl == ft_nullptr)
        return (false);
    ssl_address = reinterpret_cast<uintptr_t>(ssl);
    if (ssl_address < minimum_valid_address)
        return (false);
    if ((ssl_address & (sizeof(void *) - 1)) != 0)
        return (false);
    return (true);
}

static ssize_t ssl_send_all(SSL *ssl, const void *data, size_t size)
{
    size_t total = 0;
    const char *ptr = static_cast<const char*>(data);
    while (total < size)
    {
        ssize_t sent = nw_ssl_write(ssl, ptr + total, size - total);
        if (sent > 0)
        {
            total += sent;
            continue ;
        }
        if (sent < 0)
            return (-1);
        int last_error = ft_global_error_stack_drop_last_error();
        if (last_error == FT_ERR_SSL_WANT_READ || last_error == FT_ERR_SSL_WANT_WRITE)
        {
            if (ssl_pointer_supports_network_checks(ssl))
            {
                if (networking_check_ssl_after_send(ssl) != 0)
                    return (-1);
            }
            continue ;
        }
        return (-1);
    }
    return (static_cast<ssize_t>(total));
}

static bool tls_copy_bio_to_string(BIO *memory, ft_string &output)
{
    char *memory_data;
    long memory_length;

    output.clear();
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (false);
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    memory_length = BIO_get_mem_data(memory, &memory_data);
    if (memory_length < 0)
    {
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    if (memory_length == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (true);
    }
    output.append(memory_data, static_cast<size_t>(memory_length));
    int string_error = ft_string::last_operation_error();
    if (string_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(string_error);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_extract_x509_name(X509_NAME *name, ft_string &output)
{
    BIO *memory;
    bool copy_result;

    if (name == ft_nullptr)
    {
        output.clear();
        int string_error = ft_string::last_operation_error();
        ft_global_error_stack_push(string_error);
        return (string_error == FT_ERR_SUCCESSS);
    }
    memory = BIO_new(BIO_s_mem());
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (X509_NAME_print_ex(memory, name, 0, XN_FLAG_RFC2253) < 0)
    {
        BIO_free(memory);
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    copy_result = tls_copy_bio_to_string(memory, output);
    BIO_free(memory);
    if (!copy_result)
    {
        int last_error = ft_global_error_stack_drop_last_error();
        if (last_error == FT_ERR_SUCCESSS)
            ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_extract_asn1_time(const ASN1_TIME *time_value, ft_string &output)
{
    BIO *memory;
    bool copy_result;

    if (time_value == ft_nullptr)
    {
        output.clear();
        int string_error = ft_string::last_operation_error();
        ft_global_error_stack_push(string_error);
        return (string_error == FT_ERR_SUCCESSS);
    }
    memory = BIO_new(BIO_s_mem());
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (ASN1_TIME_print(memory, time_value) != 1)
    {
        BIO_free(memory);
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    copy_result = tls_copy_bio_to_string(memory, output);
    BIO_free(memory);
    if (!copy_result)
    {
        int last_error = ft_global_error_stack_drop_last_error();
        if (last_error == FT_ERR_SUCCESSS)
            ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_extract_serial_number(const ASN1_INTEGER *serial_value,
    ft_string &output)
{
    BIO *memory;
    bool copy_result;

    if (serial_value == ft_nullptr)
    {
        output.clear();
        int string_error = ft_string::last_operation_error();
        ft_global_error_stack_push(string_error);
        return (string_error == FT_ERR_SUCCESSS);
    }
    memory = BIO_new(BIO_s_mem());
    if (memory == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    if (i2a_ASN1_INTEGER(memory, const_cast<ASN1_INTEGER*>(serial_value)) <= 0)
    {
        BIO_free(memory);
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    copy_result = tls_copy_bio_to_string(memory, output);
    BIO_free(memory);
    if (!copy_result)
    {
        int last_error = ft_global_error_stack_drop_last_error();
        if (last_error == FT_ERR_SUCCESSS)
            ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_compute_certificate_fingerprint(X509 *certificate,
    ft_string &output)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length;
    size_t index;
    char byte_buffer[3];

    output.clear();
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        int string_error = ft_string::last_operation_error();
        ft_global_error_stack_push(string_error);
        return (false);
    }
    if (certificate == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (X509_digest(certificate, EVP_sha256(), digest, &digest_length) != 1)
    {
        ft_global_error_stack_push(FT_ERR_CONFIGURATION);
        return (false);
    }
    index = 0;
    while (index < digest_length)
    {
        if (pf_snprintf(byte_buffer, sizeof(byte_buffer), "%02X",
                digest[index]) < 0)
        {
            ft_global_error_stack_push(FT_ERR_CONFIGURATION);
            return (false);
        }
        output.append(byte_buffer, 2);
        int string_error = ft_string::last_operation_error();
        if (string_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(string_error);
            return (false);
        }
        if (index + 1 < digest_length)
        {
            output.append(':');
            int separator_error = ft_string::last_operation_error();
            if (separator_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(separator_error);
                return (false);
            }
        }
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_fill_certificate_diagnostic(X509 *certificate,
    api_tls_certificate_diagnostics &diagnostic)
{
    const ASN1_TIME *not_before;
    const ASN1_TIME *not_after;
    const ASN1_INTEGER *serial_number;

    if (certificate == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!tls_extract_x509_name(X509_get_subject_name(certificate),
            diagnostic.subject))
        return (false);
    if (!tls_extract_x509_name(X509_get_issuer_name(certificate),
            diagnostic.issuer))
        return (false);
    serial_number = X509_get_serialNumber(certificate);
    if (!tls_extract_serial_number(serial_number, diagnostic.serial_number))
        return (false);
    not_before = X509_get0_notBefore(certificate);
    if (!tls_extract_asn1_time(not_before, diagnostic.not_before))
        return (false);
    not_after = X509_get0_notAfter(certificate);
    if (!tls_extract_asn1_time(not_after, diagnostic.not_after))
        return (false);
    if (!tls_compute_certificate_fingerprint(certificate,
            diagnostic.fingerprint_sha256))
        return (false);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static bool tls_append_certificate_diagnostic(
    api_tls_handshake_diagnostics &diagnostics, X509 *certificate)
{
    api_tls_certificate_diagnostics entry;
    size_t size_before;
    size_t size_after;

    if (!tls_fill_certificate_diagnostic(certificate, entry))
        return (false);
    size_before = diagnostics.certificates.size();
    diagnostics.certificates.push_back(ft_move(entry));
    size_after = diagnostics.certificates.size();
    if (size_after < size_before + 1)
    {
        int vector_error = diagnostics.certificates.get_error();
        if (vector_error == FT_ERR_SUCCESSS)
            vector_error = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(vector_error);
        return (false);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

static void tls_log_handshake_diagnostics(
    const api_tls_handshake_diagnostics &diagnostics, const ft_string &host)
{
    size_t certificate_index;
    size_t certificate_count;

    if (!ft_log_get_api_logging())
        return ;
    ft_log_info("api_tls_client::handshake host=%s protocol=%s cipher=%s "
                "certificate_count=%zu",
        host.c_str(), diagnostics.protocol.c_str(),
        diagnostics.cipher.c_str(), diagnostics.certificates.size());
    certificate_index = 0;
    certificate_count = diagnostics.certificates.size();
    while (certificate_index < certificate_count)
    {
        const api_tls_certificate_diagnostics &entry =
            diagnostics.certificates[certificate_index];
        ft_log_info("api_tls_client::certificate[%zu] subject=%s issuer=%s "
                    "serial=%s not_before=%s not_after=%s fingerprint_sha256=%s",
            certificate_index, entry.subject.c_str(), entry.issuer.c_str(),
            entry.serial_number.c_str(), entry.not_before.c_str(),
            entry.not_after.c_str(), entry.fingerprint_sha256.c_str());
        certificate_index++;
    }
    return ;
}

api_tls_client::api_tls_client(const char *host_c, uint16_t port, int timeout_ms)
: _ctx(ft_nullptr), _ssl(ft_nullptr), _sock(-1), _host(""), _timeout(timeout_ms), _error_code(FT_ERR_SUCCESSS)
{
    this->_is_shutting_down = false;
    this->set_error(FT_ERR_SUCCESSS);
    if (!host_c)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_host = host_c;
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        this->set_error(FT_ERR_TERMINATED);
        return ;
    }

    this->_ctx = SSL_CTX_new(TLS_client_method());
    if (!this->_ctx)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (SSL_CTX_set_default_verify_paths(this->_ctx) != 1)
    {
        SSL_CTX_free(this->_ctx);
        this->_ctx = ft_nullptr;
        this->set_error(FT_ERR_CONFIGURATION);
        return ;
    }
    SSL_CTX_set_verify(this->_ctx, SSL_VERIFY_PEER, ft_nullptr);

    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_string[6];
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host_c, port_string, &hints, &address_results) != 0)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return ;
    }

    address_info = address_results;
    while (address_info != ft_nullptr)
    {
        this->_sock = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (this->_sock >= 0)
        {
            if (timeout_ms > 0)
            {
                struct timeval time_value;
                time_value.tv_sec = timeout_ms / 1000;
                time_value.tv_usec = (timeout_ms % 1000) * 1000;
                setsockopt(this->_sock, SOL_SOCKET, SO_RCVTIMEO, &time_value, sizeof(time_value));
                setsockopt(this->_sock, SOL_SOCKET, SO_SNDTIMEO, &time_value, sizeof(time_value));
            }
            if (nw_connect(this->_sock, address_info->ai_addr, static_cast<socklen_t>(address_info->ai_addrlen)) == 0)
                break;
            nw_close(this->_sock);
            this->_sock = -1;
        }
        address_info = address_info->ai_next;
    }
    if (address_results)
        freeaddrinfo(address_results);
    if (this->_sock < 0)
    {
        this->set_error(FT_ERR_SOCKET_CONNECT_FAILED);
        return ;
    }

    this->_ssl = SSL_new(this->_ctx);
    if (!this->_ssl)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    if (SSL_set1_host(this->_ssl, this->_host.c_str()) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        nw_close(this->_sock);
        this->_sock = -1;
        this->set_error(FT_ERR_CONFIGURATION);
        return ;
    }
    SSL_set_hostflags(this->_ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    void *host_name_argument;

    host_name_argument = static_cast<void*>(const_cast<char*>(this->_host.c_str()));
    if (SSL_ctrl(this->_ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, host_name_argument) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        nw_close(this->_sock);
        this->_sock = -1;
        this->set_error(FT_ERR_CONFIGURATION);
        return ;
    }
    if (SSL_set_fd(this->_ssl, this->_sock) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        nw_close(this->_sock);
        this->_sock = -1;
        this->set_error(FT_ERR_CONFIGURATION);
        return ;
    }
    if (SSL_connect(this->_ssl) <= 0)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        nw_close(this->_sock);
        this->_sock = -1;
        this->set_error(FT_ERR_SOCKET_CONNECT_FAILED);
        return ;
    }
    if (!this->populate_handshake_diagnostics())
    {
        int diagnostics_error;
        const char *error_message;

        diagnostics_error = this->_error_code;
        if (diagnostics_error == FT_ERR_SUCCESSS)
            diagnostics_error = ft_global_error_stack_drop_last_error();
        error_message = ft_strerror(diagnostics_error);
        if (ft_log_get_api_logging())
        {
            ft_log_warn("api_tls_client::handshake diagnostics unavailable host=%s "
                        "error=%d message=%s",
                this->_host.c_str(), diagnostics_error, error_message);
        }
    }
    this->set_error(FT_ERR_SUCCESSS);
}

api_tls_client::~api_tls_client()
{
    ft_vector<ft_thread> workers_local;
    SSL *ssl_pointer;
    SSL_CTX *ctx_pointer;
    int socket_fd;
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (state_lock.get_error() != FT_ERR_SUCCESSS)
        return ;
    this->_is_shutting_down = true;
    workers_local = ft_move(this->_async_workers);
    ssl_pointer = this->_ssl;
    ctx_pointer = this->_ctx;
    socket_fd = this->_sock;
    this->_ssl = ft_nullptr;
    this->_ctx = ft_nullptr;
    this->_sock = -1;
    state_lock.unlock();
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        if (ssl_pointer != ft_nullptr)
        {
            SSL_shutdown(ssl_pointer);
            SSL_free(ssl_pointer);
        }
        if (socket_fd >= 0)
            nw_close(socket_fd);
        if (ctx_pointer != ft_nullptr)
            SSL_CTX_free(ctx_pointer);
        return ;
    }
    size_t worker_index;

    worker_index = 0;
    while (worker_index < workers_local.size())
    {
        if (workers_local[worker_index].joinable())
            workers_local[worker_index].join();
        worker_index += 1;
    }
    workers_local.clear();
    if (ssl_pointer != ft_nullptr)
    {
        SSL_shutdown(ssl_pointer);
        SSL_free(ssl_pointer);
    }
    if (socket_fd >= 0)
        nw_close(socket_fd);
    if (ctx_pointer != ft_nullptr)
        SSL_CTX_free(ctx_pointer);
    return ;
}

bool api_tls_client::is_valid() const
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (this->_ssl != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (true);
    }
    this->set_error(FT_ERR_CONFIGURATION);
    return (false);
}

char *api_tls_client::request(const char *method, const char *path, json_group *payload,
                              const char *headers, int *status)
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (method == ft_nullptr || path == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (ft_nullptr);
    }
    if (this->_is_shutting_down)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (ft_nullptr);
    }
    if (this->_ssl == ft_nullptr)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    if (ft_log_get_api_logging())
    {
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_tls_client::request %s %s",
            log_method, log_path);
    }

    ft_string request(method);
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += this->_host.c_str();
    if (headers && headers[0])
    {
        request += "\r\n";
        request += headers;
    }

    ft_string body_string;
    if (payload)
    {
        char *temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            int json_error = ft_global_error_stack_drop_last_error();
            if (json_error == FT_ERR_SUCCESSS)
                json_error = FT_ERR_NO_MEMORY;
            this->set_error(json_error);
            return (ft_nullptr);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        if (!api_append_content_length_header(request, body_string.size()))
        {
            this->set_error(FT_ERR_IO);
            return (ft_nullptr);
        }
    }
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(this->_ssl, request.c_str(), request.size()) < 0)
    {
        this->set_error(FT_ERR_SOCKET_SEND_FAILED);
        return (ft_nullptr);
    }

    ft_string response;
    char buffer[1024];
    ssize_t bytes_received;
    const char *header_end_ptr = ft_nullptr;

    while (!header_end_ptr)
    {
        bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0)
        {
            this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
            return (ft_nullptr);
        }
        buffer[bytes_received] = '\0';
        response += buffer;
        header_end_ptr = ft_strstr(response.c_str(), "\r\n\r\n");
    }

    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1, ft_nullptr);
    }

    size_t header_len = static_cast<size_t>(header_end_ptr - response.c_str()) + 4;
    size_t content_length = 0;
    bool has_content_length = false;
    bool is_chunked = false;
    ft_string headers_section;
    headers_section = tls_string_substr(response, 0, header_len);
    size_t line_offset = 0;
    while (line_offset < headers_section.size())
    {
        size_t line_end = tls_string_find_substring(headers_section, "\r\n", line_offset);
        if (line_end == TLS_STRING_NPOS)
            break;
        ft_string header_line;
        header_line = tls_string_substr(headers_section, line_offset, line_end - line_offset);
        line_offset = line_end + 2;
        if (header_line.empty())
            continue;
        size_t colon_index = tls_string_find_char(header_line, ':', 0);
        if (colon_index == TLS_STRING_NPOS)
            continue;
        ft_string header_name;
        header_name = tls_string_substr(header_line, 0, colon_index);
        tls_trim_whitespace(header_name);
        if (header_name.empty())
            continue;
        ft_string header_value;
        header_value = tls_string_substr(header_line, colon_index + 1, header_line.size());
        tls_trim_whitespace(header_value);
        if (tls_header_equals(header_name, "Content-Length") && !has_content_length)
        {
            unsigned long parsed_length;
            int parse_error;
            unsigned long long parsed_length_ull;
            const char *header_value_cstr;
            char *header_parse_end;

            header_value_cstr = header_value.c_str();
            header_parse_end = ft_nullptr;
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            parsed_length = ft_strtoul(header_value_cstr, &header_parse_end, 10);
            parse_error = ft_global_error_stack_drop_last_error();
            if (parse_error != FT_ERR_SUCCESSS)
            {
                this->set_error(parse_error);
                return (ft_nullptr);
            }
            if (!header_parse_end || header_parse_end == header_value_cstr)
            {
                this->set_error(FT_ERR_OUT_OF_RANGE);
                return (ft_nullptr);
            }
            while (*header_parse_end != '\0')
            {
                if (*header_parse_end != ' ' && *header_parse_end != '\t')
                {
                    this->set_error(FT_ERR_OUT_OF_RANGE);
                    return (ft_nullptr);
                }
                header_parse_end += 1;
            }
            parsed_length_ull = static_cast<unsigned long long>(parsed_length);
            if (parsed_length_ull > FT_SYSTEM_SIZE_MAX)
            {
                this->set_error(FT_ERR_OUT_OF_RANGE);
                return (ft_nullptr);
            }
            content_length = static_cast<size_t>(parsed_length);
            has_content_length = true;
        }
        if (tls_header_equals(header_name, "Transfer-Encoding"))
        {
            ft_string lowered_value;
            lowered_value = header_value;
            tls_string_to_lower(lowered_value);
            size_t token_start = 0;
            while (token_start <= lowered_value.size())
            {
                size_t token_end = token_start;
                while (token_end < lowered_value.size()
                    && tls_string_char_at(lowered_value, token_end) != ',')
                    token_end++;
                ft_string token_value;
                token_value = tls_string_substr(lowered_value, token_start, token_end - token_start);
                tls_trim_whitespace(token_value);
                if (token_value == "chunked")
                    is_chunked = true;
                if (token_end == lowered_value.size())
                    break;
                token_start = token_end + 1;
            }
        }
    }

    ft_string body;
    body += response.c_str() + header_len;

    if (is_chunked)
    {
        ft_string chunk_buffer;
        chunk_buffer += body.c_str();
        body.clear();
        size_t parse_offset = 0;
        while (true)
        {
            const char *chunk_start = chunk_buffer.c_str() + parse_offset;
            const char *line_end = ft_strstr(chunk_start, "\r\n");
            while (!line_end)
            {
                bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                if (bytes_received <= 0)
                {
                    this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                    return (ft_nullptr);
                }
                buffer[bytes_received] = '\0';
                chunk_buffer += buffer;
                chunk_start = chunk_buffer.c_str() + parse_offset;
                line_end = ft_strstr(chunk_start, "\r\n");
            }
            size_t line_length = static_cast<size_t>(line_end - chunk_start);
            size_t index = 0;
            while (index < line_length && (chunk_start[index] == ' ' || chunk_start[index] == '\t'))
            {
                index++;
            }
            ft_string size_string;
            while (index < line_length && chunk_start[index] != ';')
            {
                size_string.append(chunk_start[index]);
                index++;
            }
            size_t size_trim = size_string.size();
            while (size_trim > 0)
            {
                const char *size_cstr = size_string.c_str();
                if (size_cstr[size_trim - 1] != ' ' && size_cstr[size_trim - 1] != '\t')
                    break;
                size_string.erase(size_trim - 1, 1);
                size_trim--;
            }
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            unsigned long chunk_length = ft_strtoul(size_string.c_str(), ft_nullptr, 16);
            int chunk_error = ft_global_error_stack_drop_last_error();
            if (chunk_error != FT_ERR_SUCCESSS)
            {
                this->set_error(chunk_error);
                return (ft_nullptr);
            }
            parse_offset += line_length + 2;
            while (chunk_buffer.size() < parse_offset + chunk_length + 2)
            {
                bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                if (bytes_received <= 0)
                {
                    this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                    return (ft_nullptr);
                }
                buffer[bytes_received] = '\0';
                chunk_buffer += buffer;
            }
            const char *data_ptr = chunk_buffer.c_str() + parse_offset;
            size_t copy_index = 0;
            while (copy_index < chunk_length)
            {
                body.append(data_ptr[copy_index]);
                copy_index++;
            }
            parse_offset += chunk_length;
            if (chunk_buffer.size() < parse_offset + 2)
            {
                while (chunk_buffer.size() < parse_offset + 2)
                {
                    bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                    if (bytes_received <= 0)
                    {
                        this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                        return (ft_nullptr);
                    }
                    buffer[bytes_received] = '\0';
                    chunk_buffer += buffer;
                }
            }
            if (chunk_length == 0)
            {
                bool trailers_complete = false;
                while (!trailers_complete)
                {
                    const char *trailer_ptr = chunk_buffer.c_str() + parse_offset;
                    const char *trailer_end = ft_strstr(trailer_ptr, "\r\n");
                    while (!trailer_end)
                    {
                        bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                        if (bytes_received <= 0)
                        {
                            this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                            return (ft_nullptr);
                        }
                        buffer[bytes_received] = '\0';
                        chunk_buffer += buffer;
                        trailer_ptr = chunk_buffer.c_str() + parse_offset;
                        trailer_end = ft_strstr(trailer_ptr, "\r\n");
                    }
                    size_t trailer_length = static_cast<size_t>(trailer_end - trailer_ptr);
                    parse_offset += trailer_length + 2;
                    if (trailer_length == 0)
                        trailers_complete = true;
                }
                break;
            }
            parse_offset += 2;
        }
    }
    else if (has_content_length)
    {
        while (body.size() < content_length)
        {
            bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0)
            {
                this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                return (ft_nullptr);
            }
            buffer[bytes_received] = '\0';
            body += buffer;
        }
        if (body.size() > content_length)
            body.erase(content_length, body.size() - content_length);
    }
    else
    {
        while ((bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes_received] = '\0';
            body += buffer;
        }
        if (bytes_received < 0)
        {
            this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
            return (ft_nullptr);
        }
    }

    char *result_body = cma_strdup(body.c_str());
    if (!result_body)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result_body);
}

json_group *api_tls_client::request_json(const char *method, const char *path,
                                         json_group *payload,
                                         const char *headers, int *status)
{
    char *body = this->request(method, path, payload, headers, status);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        int json_error = ft_global_error_stack_drop_last_error();
        if (json_error == FT_ERR_SUCCESSS)
            json_error = FT_ERR_NO_MEMORY;
        this->set_error(json_error);
        return (ft_nullptr);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (result);
}

bool api_tls_client::request_async(const char *method, const char *path,
                                   json_group *payload,
                                   const char *headers,
                                   api_callback callback,
                                   void *user_data)
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (!callback)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (this->_is_shutting_down)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    state_lock.unlock();
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    ft_thread worker([this, method, path, payload, headers, callback, user_data]()
    {
        int status_local = -1;
        char *result_body = this->request(method, path, payload, headers, &status_local);
        callback(result_body, status_local, user_data);
    });
    if (worker.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(worker.get_error());
        return (false);
    }
    state_lock.lock();
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        if (worker.joinable())
            worker.join();
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (this->_is_shutting_down)
    {
        state_lock.unlock();
        if (state_lock.get_error() != FT_ERR_SUCCESSS)
        {
            if (worker.joinable())
                worker.join();
            this->set_error(state_lock.get_error());
            return (false);
        }
        if (worker.joinable())
            worker.join();
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    size_t worker_count_before;
    size_t worker_count_after;
    int vector_error;

    worker_count_before = this->_async_workers.size();
    this->_async_workers.push_back(ft_move(worker));
    worker_count_after = this->_async_workers.size();
    if (worker_count_after < worker_count_before + 1)
    {
        vector_error = this->_async_workers.get_error();
        if (vector_error == FT_ERR_SUCCESSS)
            vector_error = FT_ERR_NO_MEMORY;
        state_lock.unlock();
        if (state_lock.get_error() != FT_ERR_SUCCESSS)
        {
            if (worker.joinable())
                worker.join();
            this->set_error(state_lock.get_error());
            return (false);
        }
        if (worker.joinable())
            worker.join();
        this->set_error(vector_error);
        return (false);
    }
    state_lock.unlock();
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

int api_tls_client::get_error() const noexcept
{
    int value;

    value = this->_error_code.load(std::memory_order_relaxed);
    return (value);
}

const char *api_tls_client::get_error_str() const noexcept
{
    int value;

    value = this->_error_code.load(std::memory_order_relaxed);
    return (ft_strerror(value));
}

void api_tls_client::set_error(int error_code) const noexcept
{

    this->_error_code.store(error_code, std::memory_order_relaxed);
    api_tls_client::record_operation_error_unlocked(error_code);
    return ;
}

bool api_tls_client::populate_handshake_diagnostics()
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);
    const char *protocol_name;
    const SSL_CIPHER *cipher;
    const char *cipher_name;
    STACK_OF(X509) *certificate_chain;
    int certificate_count;
    int certificate_index;
    X509 *certificate;
    X509 *leaf_certificate;
    int vector_error;

    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (this->_is_shutting_down)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    if (this->_ssl == ft_nullptr)
    {
        this->set_error(FT_ERR_CONFIGURATION);
        return (false);
    }
    this->_handshake_diagnostics.protocol.clear();
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_string::last_operation_error());
        return (false);
    }
    this->_handshake_diagnostics.cipher.clear();
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_string::last_operation_error());
        return (false);
    }
    this->_handshake_diagnostics.certificates.clear();
    if (this->_handshake_diagnostics.certificates.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_handshake_diagnostics.certificates.get_error());
        return (false);
    }
    protocol_name = SSL_get_version(this->_ssl);
    if (protocol_name != ft_nullptr)
    {
        this->_handshake_diagnostics.protocol.append(protocol_name);
        if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(ft_string::last_operation_error());
            return (false);
        }
    }
    cipher = SSL_get_current_cipher(this->_ssl);
    if (cipher != ft_nullptr)
    {
        cipher_name = SSL_CIPHER_get_name(cipher);
        if (cipher_name != ft_nullptr)
        {
            this->_handshake_diagnostics.cipher.append(cipher_name);
            if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(ft_string::last_operation_error());
                return (false);
            }
        }
    }
    certificate_chain = SSL_get_peer_cert_chain(this->_ssl);
    if (certificate_chain != ft_nullptr)
    {
        certificate_count = sk_X509_num(certificate_chain);
        certificate_index = 0;
        while (certificate_index < certificate_count)
        {
            certificate = sk_X509_value(certificate_chain, certificate_index);
            if (certificate != ft_nullptr)
            {
                if (!tls_append_certificate_diagnostic(this->_handshake_diagnostics,
                        certificate))
                {
                    vector_error = this->_handshake_diagnostics.certificates.get_error();
                    if (vector_error == FT_ERR_SUCCESSS)
                        vector_error = ft_global_error_stack_drop_last_error();
                    if (vector_error == FT_ERR_SUCCESSS)
                        vector_error = FT_ERR_CONFIGURATION;
                    this->set_error(vector_error);
                    return (false);
                }
            }
            certificate_index++;
        }
    }
    else
    {
        leaf_certificate = SSL_get1_peer_certificate(this->_ssl);
        if (leaf_certificate == ft_nullptr)
        {
            this->set_error(FT_ERR_CONFIGURATION);
            return (false);
        }
        if (!tls_append_certificate_diagnostic(this->_handshake_diagnostics,
                leaf_certificate))
        {
            vector_error = this->_handshake_diagnostics.certificates.get_error();
            if (vector_error == FT_ERR_SUCCESSS)
                vector_error = ft_global_error_stack_drop_last_error();
            if (vector_error == FT_ERR_SUCCESSS)
                vector_error = FT_ERR_CONFIGURATION;
            this->set_error(vector_error);
            X509_free(leaf_certificate);
            return (false);
        }
        X509_free(leaf_certificate);
    }
    this->set_error(FT_ERR_SUCCESSS);
    tls_log_handshake_diagnostics(this->_handshake_diagnostics, this->_host);
    return (true);
}

bool api_tls_client::refresh_handshake_diagnostics()
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (this->_is_shutting_down)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    state_lock.unlock();
    if (state_lock.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(state_lock.get_error());
        return (false);
    }
    if (!this->populate_handshake_diagnostics())
        return (false);
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

const api_tls_handshake_diagnostics &api_tls_client::get_handshake_diagnostics() const noexcept
{
    ft_unique_lock<pt_mutex> state_lock(this->_mutex);

    if (state_lock.get_error() != FT_ERR_SUCCESSS)
        this->set_error(state_lock.get_error());
    else
        this->set_error(FT_ERR_SUCCESSS);
    return (this->_handshake_diagnostics);
}

#endif
