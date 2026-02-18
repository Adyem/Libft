#include "../test_internal.hpp"

#include <cerrno>

#ifndef LIBFT_TEST_BUILD
#endif

#if NETWORKING_HAS_OPENSSL
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/evp.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <windows.h>
# include <io.h>
# define TLS_TEST_CLOSE _close
# define TLS_TEST_WRITE _write
# define TLS_TEST_UNLINK _unlink
# define TLS_TEST_OPEN _open
# define TLS_TEST_FLAGS (_O_CREAT | _O_WRONLY | _O_BINARY | _O_TRUNC)
# define TLS_TEST_PERMISSIONS (_S_IREAD | _S_IWRITE)
#else
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# define TLS_TEST_CLOSE close
# define TLS_TEST_WRITE write
# define TLS_TEST_UNLINK unlink
# define TLS_TEST_OPEN open
# define TLS_TEST_FLAGS (O_CREAT | O_WRONLY | O_TRUNC)
# define TLS_TEST_PERMISSIONS (S_IRUSR | S_IWUSR)
#endif
#include <fcntl.h>
#include <cstdlib>
#include <sys/stat.h>

static const char g_tls_test_root_certificate[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDFTCCAf2gAwIBAgIUYGC5vRjg6dG3tyIaD79aIaxd82IwDQYJKoZIhvcNAQEL\n"
"BQAwGjEYMBYGA1UEAwwPTGliZnQgVGVzdCBSb290MB4XDTI1MTAyMTEwMDc1M1oX\n"
"DTM1MTAxOTEwMDc1M1owGjEYMBYGA1UEAwwPTGliZnQgVGVzdCBSb290MIIBIjAN\n"
"BgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApk6RoTb/hmn6RbLMEUNdJxDt+aEN\n"
"RYtlm6Ep0apeQhTbmRgpERnWDW5ybPd6kEreMQJb476uIRhjyi33W2667VJ13bS0\n"
"kZNHzFP9GsWsV+vhDVHrs/kzK1vAQErae61ZefrUEI5CAG7VGdQFDuP9jxBNr39V\n"
"6xZLMp4jO4UCPvccpJKk0VolyxOlbEZSNIPcmUp7V73vNZkWRr15ExUC9KDQKWNt\n"
"oPwoLneDCIB71ddas70yQGTyss/ZEf5kJ5S1CblVd+qq8kuK9FhYgZOJpuRMvWcR\n"
"PJEjBMNMqVwjzheMPLjlkx4fokZUt6XkKyOzkIiOZTUEtktseykuAdKlaQIDAQAB\n"
"o1MwUTAdBgNVHQ4EFgQUm/5iRuCleY4PKnfNGJslPe/6f70wHwYDVR0jBBgwFoAU\n"
"m/5iRuCleY4PKnfNGJslPe/6f70wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0B\n"
"AQsFAAOCAQEAervIHl8+2xspm9ss2aSBCM5yOdspELWL4Xqs3J2v2C0R9UaYapVf\n"
"kuKwsxEf1hbpNglacghQEEJFBGjJmmFPYayTEA4rLUtjkzSTT79eMRIGRRbLJHz5\n"
"3mgw6jdj3lieFV3tTVZQ3BiLc8TToXIMDSOrgT9kukdvaKdPeKRbO8vLQFVx8PWY\n"
"RFaBO1eJPN8BtkFLZ05pmN+ZpnW+4pMA/gpxAdXbw3Ps9momobCsjqsLyyVeF3Hy\n"
"hQt9m5yERRBEYuyZkIDG2a2cXF4EGxoDUBjbFoZWNwiI86VvfxzycKa1PynkHlYs\n"
"JzqT6hi89fZSK2wuAd8nPCkaFYyIU+lO7Q==\n"
"-----END CERTIFICATE-----\n";

static const char g_tls_test_server_certificate[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDCDCCAfCgAwIBAgICEAAwDQYJKoZIhvcNAQELBQAwGjEYMBYGA1UEAwwPTGli\n"
"ZnQgVGVzdCBSb290MB4XDTI1MTAyMTEwMDgxNFoXDTI4MDEyNDEwMDgxNFowFDES\n"
"MBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKC\n"
"AQEAtuaAY54vZfFe422yWlt9xRky2XBPFindiooNhY9gxAOaUdOlaAhoMgXrsuGl\n"
"tCqxxS0Y2WYcTAQ+yXq4DE0QUVuyDgTaoDFQqFdZQh/mSxAiK6nveHgdZMoy5pwa\n"
"TgR5La2qnAQohm9EA1SyG4aByQxkU0EkP/rVqTLFE3m5Bn5n71czGc+zTCb6RrBW\n"
"eViK6FmpUzTcmYnqrQ579T2D/vhhX9Iz1UumqQ+FbFDXgWCVUC8cvSQH/HV/XrEd\n"
"VulRKCaiTfztKFtezVGytiXDbjavTBZvVzY2v/atTykYwVfLVjHbIl7H6fbmVdsT\n"
"a0X4Q4UQKQIUqPcgYe1fkSBopQIDAQABo14wXDAaBgNVHREEEzARgglsb2NhbGhv\n"
"c3SHBH8AAAEwHQYDVR0OBBYEFPxDmhTN56MX5fMD+suMcwYBenSvMB8GA1UdIwQY\n"
"MBaAFJv+YkbgpXmODyp3zRibJT3v+n+9MA0GCSqGSIb3DQEBCwUAA4IBAQBoVq3z\n"
"zxbPcDDCrLDetjWBHo5G0aNlBZH3wpwk7plqzBwGk7yn15DgoHNTJN6gTTTmF5bP\n"
"Bxj4KU3E/WN1H+om1R6atUH5yWFzGAhP/QFpxUoqP1E+vztkE5cIhrUGrn1NOCBm\n"
"hBVeCkrf1ZmY4InMRhLCk3wK1vkiY1dFMeTVGduO2LufSnPm6fg1zUhd8IeYgZve\n"
"gBWvlmx7zSerCLlpY97bKIlSAKwGijCPpVWb3IeVJH+8qd6dIyrcOx1rYvhnF314\n"
"RQXNzrMmTyhM60Wnq+2Vsa/tEXl54tHBJDixYPMJX1ktoZz+CP21KnhuwSDcFs20\n"
"8BEYI1YHVisx6+Dt\n"
"-----END CERTIFICATE-----\n";

static const char g_tls_test_server_key[] =
"-----BEGIN PRIVATE KEY-----\n"
"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC25oBjni9l8V7j\n"
"bbJaW33FGTLZcE8WKd2Kig2Fj2DEA5pR06VoCGgyBeuy4aW0KrHFLRjZZhxMBD7J\n"
"ergMTRBRW7IOBNqgMVCoV1lCH+ZLECIrqe94eB1kyjLmnBpOBHktraqcBCiGb0QD\n"
"VLIbhoHJDGRTQSQ/+tWpMsUTebkGfmfvVzMZz7NMJvpGsFZ5WIroWalTNNyZieqt\n"
"Dnv1PYP++GFf0jPVS6apD4VsUNeBYJVQLxy9JAf8dX9esR1W6VEoJqJN/O0oW17N\n"
"UbK2JcNuNq9MFm9XNja/9q1PKRjBV8tWMdsiXsfp9uZV2xNrRfhDhRApAhSo9yBh\n"
"7V+RIGilAgMBAAECggEAU9otdn5pQJV+aUo7gI19hcVduK5ys7iR5xgPxvHK7VKx\n"
"h14+8QkZrAVm3+XvsD/6rGlWgST9SCSSHJD0nD8gggxF/4NeMB8r2WHifBxf2n4V\n"
"DGumbFa1MyY4W92peac3XyfFli2Z3rhYOCa2WUQ7cE8MF9c/pU18c0vhamJd34mB\n"
"5e9ECzjFG/dVQ96dmfwgGMfGLcayTUKKVHYZeCKT1OzVsIc3cDY66Gqm9XbpA5BX\n"
"NeWh9gyfyQnNRWUN3QSAEid/4P7UXzeOCc1ikO7k5xT/AizsXuaP6h1jSJZ69Y42\n"
"l+DKSoZ6uL8F6pmgikJEsyep4Njh/9EshjxpmZf2KQKBgQDePPI3Ig9nkzHJ69DR\n"
"BEop7IqAWaV1HYGB+tA3/2oRR7Rr8w3e/S6CEOo+BsNu8PUj10Cu/fBvH5BEZHSb\n"
"OmLYnSbDyV7oS6QW5qABT4w4Pfr5wWFg25ZZH1hZz8aIykENV/8AqXXfm3+dqumU\n"
"RaYUjcsjpfY5rbHgSSpNssmQ5wKBgQDSr6zTzJBBdTNpEprwCms3CgWqIQBmPayh\n"
"0xxa+uL46hMdgngGMKAOTL63m44Rp0Jv5xL/uAXCbXiLDs0wUPS2wJzYqz374aQq\n"
"8A3lbD6q/HtjjMCwy6frQQtFfZBNky/tQzwwr6voowpSLfjQe4CCe0HQOGKeXMQ3\n"
"bZlOCeGskwKBgCQomGQ+B3GbsmMfGieX4hU2JkbBzzjh5xtl2pDx3qkXGJTjw+L/\n"
"IJ0K8YHMCeo4DDqboCuYp5D8+rUV/x7uffyufQsn7WgwPm93cJsl/TN9aiNOwnBG\n"
"6gxwHCjotG23MqRgJ95MtCAApd/aWgzE1EPN2jpfm1NBHorWfKqNp1wLAoGBAJQU\n"
"88V9/1pytOrCL/JNcGQkAC0+utghuyeWiVSNCGDprPvNyuyW2riSSm4kwGs6A2FB\n"
"qcHgpvOsG9CpW6D9FbMpmFRVr+BNDSU8ypeZSEbH5TXoJTteOS0J75EtGvLbZBFA\n"
"c5rIrmx5w/RU6ZAnbgDH/08Jgo9KJoPK3bjTCzJPAoGARqIH0RezMX4qkb1xg/Cb\n"
"qz9lhSzAIN30IWDLZ2TqbSZ8dIOeCQcT/rbWRKrpx0ANbDELErvVZDMbTSVOIhQD\n"
"JFLd/j0tDlqZCwtPKNWdkzNHgALYWM8ByrBHCUzQOJ3jEnaDyOobaxnmUajhLbIy\n"
"I4tPGyBXqN2OMTNsjC0rkek=\n"
"-----END PRIVATE KEY-----\n";

struct tls_test_server_context
{
    ft_string certificate_path;
    ft_string key_path;
    std::atomic<bool> ready;
    std::atomic<bool> handshake_complete;
    std::atomic<bool> stop;
    int listen_fd;
    int client_fd;
    unsigned short port;
    int result;
};

static void tls_signal_server_stop(tls_test_server_context &context)
{
    context.stop.store(true);
    if (context.client_fd < 0 && context.listen_fd >= 0)
    {
        nw_close(context.listen_fd);
        context.listen_fd = -1;
    }
    return ;
}

static bool tls_write_temp_file(const char *prefix, const char *contents, ft_string &path)
{
    char template_path[256];
    int file_descriptor;
    size_t total_written;
    size_t content_length;
    ssize_t write_result;

    if (!prefix || !contents)
        return (false);
    ft_bzero(template_path, sizeof(template_path));
#ifdef _WIN32
    if (pf_snprintf(template_path, sizeof(template_path), "%sXXXXXX", prefix) < 0)
        return (false);
    if (_mktemp_s(template_path, sizeof(template_path)) != 0)
        return (false);
    file_descriptor = TLS_TEST_OPEN(template_path, TLS_TEST_FLAGS, TLS_TEST_PERMISSIONS);
    if (file_descriptor < 0)
        return (false);
#else
    if (pf_snprintf(template_path, sizeof(template_path), "%sXXXXXX", prefix) < 0)
        return (false);
    file_descriptor = mkstemp(template_path);
    if (file_descriptor < 0)
        return (false);
#endif
    content_length = ft_strlen(contents);
    total_written = 0;
    while (total_written < content_length)
    {
        write_result = TLS_TEST_WRITE(file_descriptor, contents + total_written,
            static_cast<unsigned int>(content_length - total_written));
        if (write_result <= 0)
        {
            TLS_TEST_CLOSE(file_descriptor);
            TLS_TEST_UNLINK(template_path);
            return (false);
        }
        total_written += static_cast<size_t>(write_result);
    }
    if (TLS_TEST_CLOSE(file_descriptor) != 0)
    {
        TLS_TEST_UNLINK(template_path);
        return (false);
    }
    path = template_path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        TLS_TEST_UNLINK(template_path);
        return (false);
    }
    return (true);
}

static bool tls_get_socket_port(int socket_fd, unsigned short &port_value)
{
    struct sockaddr_storage address;
    socklen_t address_length;

    if (socket_fd < 0)
        return (false);
    address_length = sizeof(address);
    if (getsockname(socket_fd, reinterpret_cast<struct sockaddr*>(&address), &address_length) != 0)
        return (false);
    if (address.ss_family == AF_INET)
    {
        const struct sockaddr_in *ipv4_address;

        ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&address);
        port_value = ntohs(ipv4_address->sin_port);
        return (true);
    }
    if (address.ss_family == AF_INET6)
    {
        const struct sockaddr_in6 *ipv6_address;

        ipv6_address = reinterpret_cast<const struct sockaddr_in6*>(&address);
        port_value = ntohs(ipv6_address->sin6_port);
        return (true);
    }
    return (false);
}

static void tls_test_server_run(tls_test_server_context *context)
{
    SSL_CTX *ssl_context;
    SSL *ssl_session;
    struct sockaddr_in server_address;
    struct sockaddr_storage client_address;
    socklen_t client_length;

    if (context == ft_nullptr)
        return ;
    context->result = -1;
    context->listen_fd = -1;
    context->client_fd = -1;
    context->ready.store(false);
    context->handshake_complete.store(false);
    context->stop.store(false);
    ssl_context = SSL_CTX_new(TLS_server_method());
    if (!ssl_context)
        return ;
    if (SSL_CTX_use_certificate_file(ssl_context, context->certificate_path.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (SSL_CTX_use_PrivateKey_file(ssl_context, context->key_path.c_str(), SSL_FILETYPE_PEM) != 1)
    {
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (SSL_CTX_check_private_key(ssl_context) != 1)
    {
        SSL_CTX_free(ssl_context);
        return ;
    }
    context->listen_fd = nw_socket(AF_INET, SOCK_STREAM, 0);
    if (context->listen_fd < 0)
    {
        SSL_CTX_free(ssl_context);
        return ;
    }
    ft_bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = 0;
    if (nw_bind(context->listen_fd, reinterpret_cast<struct sockaddr*>(&server_address), sizeof(server_address)) != 0)
    {
        nw_close(context->listen_fd);
        context->listen_fd = -1;
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (nw_listen(context->listen_fd, 1) != 0)
    {
        nw_close(context->listen_fd);
        context->listen_fd = -1;
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (!tls_get_socket_port(context->listen_fd, context->port))
    {
        nw_close(context->listen_fd);
        context->listen_fd = -1;
        SSL_CTX_free(ssl_context);
        return ;
    }
    context->ready.store(true);
    client_length = sizeof(client_address);
    context->client_fd = nw_accept(context->listen_fd, reinterpret_cast<struct sockaddr*>(&client_address), &client_length);
    if (context->client_fd < 0)
    {
        nw_close(context->listen_fd);
        context->listen_fd = -1;
        SSL_CTX_free(ssl_context);
        return ;
    }
    ssl_session = SSL_new(ssl_context);
    if (!ssl_session)
    {
        nw_close(context->client_fd);
        nw_close(context->listen_fd);
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (SSL_set_fd(ssl_session, context->client_fd) != 1)
    {
        SSL_free(ssl_session);
        nw_close(context->client_fd);
        nw_close(context->listen_fd);
        SSL_CTX_free(ssl_context);
        return ;
    }
    if (SSL_accept(ssl_session) <= 0)
    {
        SSL_free(ssl_session);
        nw_close(context->client_fd);
        context->client_fd = -1;
        nw_close(context->listen_fd);
        context->listen_fd = -1;
        SSL_CTX_free(ssl_context);
        return ;
    }
    context->handshake_complete.store(true);
    while (!context->stop.load())
        time_sleep_ms(5);
    SSL_shutdown(ssl_session);
    SSL_free(ssl_session);
    if (context->client_fd >= 0)
    {
        nw_close(context->client_fd);
        context->client_fd = -1;
    }
    if (context->listen_fd >= 0)
    {
        nw_close(context->listen_fd);
        context->listen_fd = -1;
    }
    SSL_CTX_free(ssl_context);
    context->result = 0;
    return ;
}

static bool tls_compute_expected_fingerprint(ft_string &fingerprint)
{
    BIO *memory;
    X509 *certificate;
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length;
    size_t index;
    char byte_buffer[3];

    fingerprint.clear();
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (false);
    memory = BIO_new_mem_buf(g_tls_test_server_certificate, -1);
    if (!memory)
        return (false);
    certificate = PEM_read_bio_X509(memory, ft_nullptr, ft_nullptr, ft_nullptr);
    BIO_free(memory);
    if (!certificate)
        return (false);
    if (X509_digest(certificate, EVP_sha256(), digest, &digest_length) != 1)
    {
        X509_free(certificate);
        return (false);
    }
    index = 0;
    while (index < digest_length)
    {
        if (pf_snprintf(byte_buffer, sizeof(byte_buffer), "%02X", digest[index]) < 0)
        {
            X509_free(certificate);
            return (false);
        }
        fingerprint.append(byte_buffer, 2);
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            X509_free(certificate);
            return (false);
        }
        if (index + 1 < digest_length)
        {
            fingerprint.append(':');
            if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
            {
                X509_free(certificate);
                return (false);
            }
        }
        index++;
    }
    X509_free(certificate);
    return (true);
}

FT_TEST(test_api_tls_client_populates_handshake_diagnostics,
    "api_tls_client captures TLS handshake certificate metadata")
{
    ft_string ca_path;
    ft_string cert_path;
    ft_string key_path;
    tls_test_server_context server_context;
    ft_thread server_thread;
    const char *original_cert_file;
    ft_string original_cert_file_copy;
    bool has_original_cert_file;
    ft_string fingerprint;
    bool fingerprint_result;

    if (!tls_write_temp_file("/tmp/libft_tls_root", g_tls_test_root_certificate, ca_path))
        return (0);
    if (!tls_write_temp_file("/tmp/libft_tls_cert", g_tls_test_server_certificate, cert_path))
    {
        TLS_TEST_UNLINK(ca_path.c_str());
        return (0);
    }
    if (!tls_write_temp_file("/tmp/libft_tls_key", g_tls_test_server_key, key_path))
    {
        TLS_TEST_UNLINK(ca_path.c_str());
        TLS_TEST_UNLINK(cert_path.c_str());
        return (0);
    }
    original_cert_file = getenv("SSL_CERT_FILE");
    has_original_cert_file = (original_cert_file != ft_nullptr);
    if (has_original_cert_file)
    {
        original_cert_file_copy = original_cert_file;
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
    }
    if (setenv("SSL_CERT_FILE", ca_path.c_str(), 1) != 0)
    {
        TLS_TEST_UNLINK(ca_path.c_str());
        TLS_TEST_UNLINK(cert_path.c_str());
        TLS_TEST_UNLINK(key_path.c_str());
        return (0);
    }
    server_context.certificate_path = cert_path;
    server_context.key_path = key_path;
    server_context.ready.store(false);
    server_context.handshake_complete.store(false);
    server_context.stop.store(false);
    server_context.listen_fd = -1;
    server_context.client_fd = -1;
    server_thread = ft_thread(tls_test_server_run, &server_context);
    if (server_thread.joinable() == false)
    {
        if (has_original_cert_file)
            setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
        else
            unsetenv("SSL_CERT_FILE");
        TLS_TEST_UNLINK(ca_path.c_str());
        TLS_TEST_UNLINK(cert_path.c_str());
        TLS_TEST_UNLINK(key_path.c_str());
        return (0);
    }
    while (!server_context.ready.load())
        time_sleep_ms(5);
    if (!server_context.ready.load())
    {
        tls_signal_server_stop(server_context);
        server_thread.join();
        if (has_original_cert_file)
            setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
        else
            unsetenv("SSL_CERT_FILE");
        TLS_TEST_UNLINK(ca_path.c_str());
        TLS_TEST_UNLINK(cert_path.c_str());
        TLS_TEST_UNLINK(key_path.c_str());
        return (0);
    }
    int test_result;

    test_result = 0;
    {
        api_tls_client client("localhost", server_context.port, 2000);

        if (!client.is_valid())
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        while (!server_context.handshake_complete.load())
            time_sleep_ms(5);
        if (!client.refresh_handshake_diagnostics())
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        const api_tls_handshake_diagnostics &diagnostics = client.get_handshake_diagnostics();

        if (diagnostics.protocol.size() == 0)
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        if (diagnostics.cipher.size() == 0)
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        if (diagnostics.certificates.size() == 0)
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        const api_tls_certificate_diagnostics &leaf = diagnostics.certificates[0];

        if (!(leaf.subject == "CN=localhost"))
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        if (!(leaf.issuer == "CN=Libft Test Root"))
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        if (!(leaf.serial_number == "1000"))
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        fingerprint_result = tls_compute_expected_fingerprint(fingerprint);
        if (!fingerprint_result)
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        if (!(leaf.fingerprint_sha256 == fingerprint))
        {
            tls_signal_server_stop(server_context);
            server_thread.join();
            if (has_original_cert_file)
                setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
            else
                unsetenv("SSL_CERT_FILE");
            TLS_TEST_UNLINK(ca_path.c_str());
            TLS_TEST_UNLINK(cert_path.c_str());
            TLS_TEST_UNLINK(key_path.c_str());
            return (0);
        }
        tls_signal_server_stop(server_context);
        test_result = 1;
    }
    server_thread.join();
    if (has_original_cert_file)
        setenv("SSL_CERT_FILE", original_cert_file_copy.c_str(), 1);
    else
        unsetenv("SSL_CERT_FILE");
    TLS_TEST_UNLINK(ca_path.c_str());
    TLS_TEST_UNLINK(cert_path.c_str());
    TLS_TEST_UNLINK(key_path.c_str());
    return (test_result);
}

#endif
