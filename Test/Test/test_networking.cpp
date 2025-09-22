#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/udp_socket.hpp"
#include "../../Networking/http_client.hpp"
#include "../../Networking/ssl_wrapper.hpp"
#include "network_io_harness.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstring>
#include <cstdio>
#include <thread>
#include <chrono>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <cerrno>
#include <climits>
#ifndef _WIN32
# include <netdb.h>
#endif

static int g_mock_ssl_write_call_count = 0;
static int g_mock_ssl_write_last_length = 0;
static int g_mock_ssl_read_call_count = 0;
static int g_mock_ssl_read_last_length = 0;

extern "C"
{
    int SSL_write(SSL *ssl, const void *buffer, int length)
    {
        (void)ssl;
        (void)buffer;
        g_mock_ssl_write_call_count++;
        g_mock_ssl_write_last_length = length;
        return (length);
    }

    int SSL_read(SSL *ssl, void *buffer, int length)
    {
        (void)ssl;
        (void)buffer;
        g_mock_ssl_read_call_count++;
        g_mock_ssl_read_last_length = length;
        return (length);
    }
}

struct throttled_transfer_result
{
    ft_string data;
    size_t read_iterations;
    int status;
};

static void throttled_read_loop(int descriptor, size_t expected_length, size_t chunk_size,
                                int delay_milliseconds, throttled_transfer_result *result)
{
    char buffer[64];
    size_t limited_chunk_size;

    if (result == ft_nullptr)
        return ;
    result->data.clear();
    result->read_iterations = 0;
    result->status = 0;
    if (chunk_size == 0)
        limited_chunk_size = sizeof(buffer);
    else if (chunk_size > sizeof(buffer))
        limited_chunk_size = sizeof(buffer);
    else
        limited_chunk_size = chunk_size;
    while (result->data.size() < expected_length)
    {
        size_t remaining_length;
        size_t bytes_to_read;
        ssize_t read_result;

        remaining_length = expected_length - result->data.size();
        if (remaining_length < limited_chunk_size)
            bytes_to_read = remaining_length;
        else
            bytes_to_read = limited_chunk_size;
        read_result = nw_recv(descriptor, buffer, bytes_to_read, 0);
        if (read_result <= 0)
        {
            result->status = -1;
            return ;
        }
        result->data.append(buffer, static_cast<size_t>(read_result));
        result->read_iterations++;
        if (delay_milliseconds > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_milliseconds));
    }
    return ;
}

static const char g_test_certificate_pem[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDCTCCAfGgAwIBAgIUTryqC5sa0G864PXa4SXOGpTQe6EwDQYJKoZIhvcNAQEL\n"
    "BQAwFDESMBAGA1UEAwwJbG9jYWxob3N0MB4XDTI1MDkyMjE4MTMxMloXDTI1MDky\n"
    "MzE4MTMxMlowFDESMBAGA1UEAwwJbG9jYWxob3N0MIIBIjANBgkqhkiG9w0BAQEF\n"
    "AAOCAQ8AMIIBCgKCAQEA9j1SpJmiSrAR5biFakQczfZvVENXyXT4I5z7qnM+0ydh\n"
    "p3cAI8B21425tHtB4rkZNg1kXW0+9t1dqFqEXuGHPWIEmNGFh4Cyt8ApIT7x01A6\n"
    "Ci1k7blvnD2qYqFxARcRQvOf5OFfqvksvTjrwKWAg0iji70+UyLlpVnvLsnoSgTj\n"
    "Kom5g1npElt6XorjdKNMkeMJgH15QV6GzAjVIFrjrgaGTsPy6u69OwuC17+42NZ5\n"
    "FSWvG4RtLzXTTAZPXg8qgjNLaOggJ3JanqVhGorrKltWbj+mJ2n4jNcFkD05Ag15\n"
    "cPxZJu0uip/d/oczeGkwvilyHZ3szhtnAp8KtkD7QQIDAQABo1MwUTAdBgNVHQ4E\n"
    "FgQUtefBZNW6voCuL0U5xwWgOcNrOX8wHwYDVR0jBBgwFoAUtefBZNW6voCuL0U5\n"
    "xwWgOcNrOX8wDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAhPAv\n"
    "YOBnayAMQOZ5R2dCSHSDVZmS9QkvBOtg6Cy3k/zy133uuJPGEVT1L5GOgM27QPh2\n"
    "LPq4heO1h7HlWwwLSk4YMJ3LW90P/ADP65QCuoSs6nXNNPhLPMiXYRAD1ejcS3Ly\n"
    "t5bCnVnhgdXHG9FVVSC/iLhSpTUNryRR38g/Mm08WeVnZLCQ3BUceVrHlrBjr71m\n"
    "+cnrQve7NS/KO1foHhxQMQNj9kFIktUVl7wX4NjjbbRT+LgaeQ/kqJg6+KTIWH8R\n"
    "kJVLS+AGsMVp1HebS0gVLGAt5hrX5S9+5i8a6fmxdRD2iHwm0s770+7iGRzGDrdM\n"
    "sLaEfk5D380xGioiZQ==\n"
    "-----END CERTIFICATE-----\n";

static const char g_test_private_key_pem[] =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQD2PVKkmaJKsBHl\n"
    "uIVqRBzN9m9UQ1fJdPgjnPuqcz7TJ2GndwAjwHbXjbm0e0HiuRk2DWRdbT723V2o\n"
    "WoRe4Yc9YgSY0YWHgLK3wCkhPvHTUDoKLWTtuW+cPapioXEBFxFC85/k4V+q+Sy9\n"
    "OOvApYCDSKOLvT5TIuWlWe8uyehKBOMqibmDWekSW3peiuN0o0yR4wmAfXlBXobM\n"
    "CNUgWuOuBoZOw/Lq7r07C4LXv7jY1nkVJa8bhG0vNdNMBk9eDyqCM0to6CAnclqe\n"
    "pWEaiusqW1ZuP6YnafiM1wWQPTkCDXlw/Fkm7S6Kn93+hzN4aTC+KXIdnezOG2cC\n"
    "nwq2QPtBAgMBAAECggEAKl4oRVaC6FdtqOlMGVnxW9XaV3UD2o+j71rAKZaNOX8l\n"
    "7A7LaRthR1rlGEL59eTzy8tXmMSmiArUJda3Pm0PHDZshI/Oah9wgLHRUd1W9g0w\n"
    "g1ZGrqmQpOEuWyyceTvLFko9WYAWuiAH/OHZnIrNdylNLIrJ+buAKGh2QUZvz6Y7\n"
    "unZ0AukdNI/btm21rs/IYQva6RkslWKPiBULR+a5MuSemJGO2caR8dsXy9c1H9al\n"
    "fwQt2F//njJTahpaPswANIVS4sNfvib7GVoXmu/0oZWM33Z/M40lCH/naJHzSVCl\n"
    "KbBVPTRpcW3Kil8d9UkuoHRfuZ+8KhTjBR2paDRkbQKBgQD7ysOl4LdZAgZkopLm\n"
    "AUFZfiHx7F5tp9SS32RYvC9IcIXTB1SdUTS+C9TD1wWbqXPDJ3VpDJ641Wo31OVL\n"
    "5nCu+cQXBYPb2AyZUj/U1fsrQPtukfSw0VklR4Z/7hXLoc/KqODiZPoiUqsrIQHU\n"
    "njtsB6M68JtfXabn7/7ZdjjF9QKBgQD6Ws2tTnZbskBc0LE3x2IJN1AC8tznI1iQ\n"
    "tIaUD1p3yw61/Nvtp6I44n1XjeoOlhdzHeNKHXQECzNFOzAl9GvCXsTRdbN52/jz\n"
    "8i2Nj6p8btbId91+fiUbrWhFX9TlMD/OUb9ZjMVWoUGnzL7xROObUrTpIUvdK1MU\n"
    "4C0uOmDEnQKBgQDlJbv2a1MRGwmsblQYJI5Pnr3PyJiLXRDVdZ6CLwks+NKhoes8\n"
    "nJnOV0AT1s0QxB9JPNDagJS2i+r10re24oosg7wUY32EBtQ4eKUE0pSZ4bKxBv3U\n"
    "KUHi7nwWUxvczW8o9ZZsEw+dSvTh9FERKcvUXgMMccpE+Rsbu8uIJtI2PQKBgElP\n"
    "g8EHI+OQHAbR2cNCsHOQbe6fHY7Vq96b7ni20kAYBxPFmsVJ/Ff6Asg7t1lUHEPK\n"
    "cb9HRqzUdswEHAJRQ6jkRXAsrkS15H3S4gH9GuhbMKv9Za/f0o/7RHhkKpx+d4C8\n"
    "MXNQpIzMpcQ0UYmnKZtXKcDGzkdSo36OcsyBrcqFAoGAefprVXT9QxL5/3fYSAQX\n"
    "cYBZ5Ped+lh44H440q2nJMATmRJTTcnwpNUa9FI0S90u61lYF0eQB8MhIVRU3buO\n"
    "7yOLoaQi8ZWvHwWT8kHrbsK80HGtZSO9A7vkn/n6ih9pAofPvv3xd8ajDHdaz89m\n"
    "qeNgmHMRpUhqJ4pBjT4TGvE=\n"
    "-----END PRIVATE KEY-----\n";

static X509 *load_test_certificate()
{
    BIO *certificate_bio;
    X509 *certificate;

    certificate_bio = BIO_new_mem_buf(g_test_certificate_pem, sizeof(g_test_certificate_pem) - 1);
    if (certificate_bio == ft_nullptr)
        return (ft_nullptr);
    certificate = PEM_read_bio_X509(certificate_bio, ft_nullptr, ft_nullptr, ft_nullptr);
    BIO_free(certificate_bio);
    return (certificate);
}

static EVP_PKEY *load_test_private_key()
{
    BIO *key_bio;
    EVP_PKEY *private_key;

    key_bio = BIO_new_mem_buf(g_test_private_key_pem, sizeof(g_test_private_key_pem) - 1);
    if (key_bio == ft_nullptr)
        return (ft_nullptr);
    private_key = PEM_read_bio_PrivateKey(key_bio, ft_nullptr, ft_nullptr, ft_nullptr);
    BIO_free(key_bio);
    return (private_key);
}

static int initialize_ssl_pair(network_io_harness &harness, SSL_CTX **client_context,
                               SSL **client_ssl, SSL_CTX **server_context, SSL **server_ssl)
{
    SSL_CTX *client_ctx;
    SSL_CTX *server_ctx;
    SSL *client;
    SSL *server;
    X509 *certificate;
    EVP_PKEY *private_key;

    if (client_context == ft_nullptr || client_ssl == ft_nullptr
        || server_context == ft_nullptr || server_ssl == ft_nullptr)
        return (FT_EINVAL);
    SSL_library_init();
    client_ctx = SSL_CTX_new(TLS_client_method());
    if (client_ctx == ft_nullptr)
        return (FT_EINVAL);
    server_ctx = SSL_CTX_new(TLS_server_method());
    if (server_ctx == ft_nullptr)
    {
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    certificate = load_test_certificate();
    if (certificate == ft_nullptr)
    {
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    if (SSL_CTX_use_certificate(server_ctx, certificate) != 1)
    {
        X509_free(certificate);
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    X509_free(certificate);
    private_key = load_test_private_key();
    if (private_key == ft_nullptr)
    {
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    if (SSL_CTX_use_PrivateKey(server_ctx, private_key) != 1)
    {
        EVP_PKEY_free(private_key);
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    EVP_PKEY_free(private_key);
    client = SSL_new(client_ctx);
    server = SSL_new(server_ctx);
    if (client == ft_nullptr || server == ft_nullptr)
    {
        if (client != ft_nullptr)
            SSL_free(client);
        if (server != ft_nullptr)
            SSL_free(server);
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    if (SSL_set_fd(client, harness.get_client_fd()) != 1
        || SSL_set_fd(server, harness.get_server_fd()) != 1)
    {
        SSL_free(client);
        SSL_free(server);
        SSL_CTX_free(server_ctx);
        SSL_CTX_free(client_ctx);
        return (FT_EINVAL);
    }
    SSL_set_connect_state(client);
    SSL_set_accept_state(server);
    *client_context = client_ctx;
    *server_context = server_ctx;
    *client_ssl = client;
    *server_ssl = server;
    return (ER_SUCCESS);
}

static void cleanup_ssl_pair(SSL_CTX *client_context, SSL *client_ssl,
                             SSL_CTX *server_context, SSL *server_ssl)
{
    if (client_ssl != ft_nullptr)
        SSL_free(client_ssl);
    if (server_ssl != ft_nullptr)
        SSL_free(server_ssl);
    if (client_context != ft_nullptr)
        SSL_CTX_free(client_context);
    if (server_context != ft_nullptr)
        SSL_CTX_free(server_context);
    return ;
}

static void throttled_ssl_read_loop(SSL *ssl, size_t expected_length, size_t chunk_size,
                                    int delay_milliseconds, throttled_transfer_result *result)
{
    unsigned char buffer[64];
    size_t limited_chunk_size;

    if (result == ft_nullptr)
        return ;
    result->data.clear();
    result->read_iterations = 0;
    result->status = 0;
    if (chunk_size == 0)
        limited_chunk_size = sizeof(buffer);
    else if (chunk_size > sizeof(buffer))
        limited_chunk_size = sizeof(buffer);
    else
        limited_chunk_size = chunk_size;
    while (result->data.size() < expected_length)
    {
        size_t remaining_length;
        size_t bytes_to_read;
        int read_result;

        remaining_length = expected_length - result->data.size();
        if (remaining_length < limited_chunk_size)
            bytes_to_read = remaining_length;
        else
            bytes_to_read = limited_chunk_size;
        read_result = SSL_read(ssl, buffer, static_cast<int>(bytes_to_read));
        if (read_result <= 0)
        {
            result->status = -1;
            return ;
        }
        result->data.append(reinterpret_cast<const char *>(buffer), static_cast<size_t>(read_result));
        result->read_iterations++;
        if (delay_milliseconds > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_milliseconds));
    }
    return ;
}

FT_TEST(test_ssl_write_rejects_oversize_length, "nw_ssl_write rejects oversize length")
{
    size_t oversize_length;
    ssize_t result;

    g_mock_ssl_write_call_count = 0;
    g_mock_ssl_write_last_length = 0;
    oversize_length = static_cast<size_t>(INT_MAX);
    oversize_length = oversize_length + 1;
    result = nw_ssl_write(reinterpret_cast<SSL *>(0x1), "data", oversize_length);
    if (result >= 0)
        return (0);
    if (g_mock_ssl_write_call_count != 0)
        return (0);
    return (1);
}

FT_TEST(test_ssl_read_rejects_oversize_length, "nw_ssl_read rejects oversize length")
{
    size_t oversize_length;
    ssize_t result;
    char buffer[4];

    g_mock_ssl_read_call_count = 0;
    g_mock_ssl_read_last_length = 0;
    oversize_length = static_cast<size_t>(INT_MAX);
    oversize_length = oversize_length + 1;
    result = nw_ssl_read(reinterpret_cast<SSL *>(0x1), buffer, oversize_length);
    if (result >= 0)
        return (0);
    if (g_mock_ssl_read_call_count != 0)
        return (0);
    return (1);
}

static ssize_t send_returns_zero_then_error(int socket_fd, const void *buffer,
                                            size_t length, int flags)
{
    (void)socket_fd;
    (void)buffer;
    (void)length;
    (void)flags;
    g_send_stub_call_count++;
    if (g_send_stub_call_count < 3)
        return (0);
#ifdef EPIPE
    errno = EPIPE;
#else
    errno = ECONNRESET;
#endif
    return (-1);
}

FT_TEST(test_network_send_receive, "nw_send/nw_recv IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54321;
    server_configuration._type = SocketType::SERVER;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54321;
    client_configuration._type = SocketType::CLIENT;
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage address;
    socklen_t address_length = sizeof(address);
    int client_file_descriptor = nw_accept(server.get_fd(),
                                           reinterpret_cast<struct sockaddr*>(&address),
                                           &address_length);
    if (client_file_descriptor < 0)
        return (0);

    const char *message = "ping";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_udp_send_receive, "nw_sendto/nw_recvfrom IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54329;
    server_configuration._type = SocketType::SERVER;
    server_configuration._protocol = IPPROTO_UDP;
    udp_socket server;
    if (server.initialize(server_configuration) != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54329;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._protocol = IPPROTO_UDP;
    udp_socket client;
    if (client.initialize(client_configuration) != ER_SUCCESS)
        return (0);

    struct sockaddr_storage dest;
    dest = server.get_address();
    const char *message = "data";
    ssize_t sent = client.send_to(message, ft_strlen(message), 0,
                                  reinterpret_cast<const struct sockaddr*>(&dest),
                                  sizeof(struct sockaddr_in));
    if (sent != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    char buffer[16];
    socklen_t addr_len = sizeof(dest);
    ssize_t received = server.receive_from(buffer, sizeof(buffer) - 1, 0,
                                          reinterpret_cast<struct sockaddr*>(&dest),
                                          &addr_len);
    if (received < 0)
        return (0);
    buffer[received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_invalid_ip, "invalid IPv4 address")
{
    SocketConfig configuration;
    configuration._type = SocketType::SERVER;
    configuration._port = 54324;
    configuration._ip = "256.0.0.1";
    ft_socket server(configuration);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_send_uninitialized, "send on uninitialized socket")
{
    ft_socket socket_object;
    const char *message = "fail";
    ssize_t result = socket_object.send_all(message, ft_strlen(message), 0);
    return (result < 0 && socket_object.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_ipv6_send_receive, "nw_send/nw_recv IPv6")
{
    SocketConfig server_configuration;
    server_configuration._port = 54325;
    server_configuration._type = SocketType::SERVER;
    server_configuration._address_family = AF_INET6;
    server_configuration._ip = "::1";
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54325;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._address_family = AF_INET6;
    client_configuration._ip = "::1";
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage address;
    socklen_t address_length = sizeof(address);
    int client_file_descriptor = nw_accept(server.get_fd(),
                                           reinterpret_cast<struct sockaddr*>(&address),
                                           &address_length);
    if (client_file_descriptor < 0)
        return (0);

    const char *message = "pong";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_ipv6_invalid_ip, "invalid IPv6 address")
{
    SocketConfig configuration;
    configuration._type = SocketType::SERVER;
    configuration._address_family = AF_INET6;
    configuration._port = 54326;
    configuration._ip = "gggg::1";
    ft_socket server(configuration);
    return (server.get_error() == SOCKET_INVALID_CONFIGURATION);
}

FT_TEST(test_network_poll_ipv4, "nw_poll IPv4")
{
    SocketConfig server_configuration;
    server_configuration._port = 54327;
    server_configuration._type = SocketType::SERVER;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54327;
    client_configuration._type = SocketType::CLIENT;
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage address;
    socklen_t address_length = sizeof(address);
    int client_file_descriptor = nw_accept(server.get_fd(),
                                           reinterpret_cast<struct sockaddr*>(&address),
                                           &address_length);
    if (client_file_descriptor < 0)
        return (0);
    if (nw_set_nonblocking(client_file_descriptor) != 0)
        return (0);

    const char *message = "ready";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    int read_descriptors[1];
    read_descriptors[0] = client_file_descriptor;
    int result = nw_poll(read_descriptors, 1, NULL, 0, 1000);
    if (result != 1 || read_descriptors[0] == -1)
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

FT_TEST(test_network_send_all_peer_close, "send_all handles peer disconnect")
{
    network_io_harness harness;
    const char *message;
    ssize_t send_result;
    int error_code;

    if (harness.initialize(0) != ER_SUCCESS)
        return (0);
    harness.close_server();
    message = "halt";
    errno = 0;
    send_result = cmp_socket_send_all(&harness.get_client_socket(), message, ft_strlen(message), 0);
    error_code = harness.get_client_socket().get_error();
    if (send_result >= 0)
        return (0);
#ifdef _WIN32
    if (error_code != (WSAECONNRESET + ERRNO_OFFSET)
        && error_code != (WSAENOTCONN + ERRNO_OFFSET)
        && error_code != (WSAECONNABORTED + ERRNO_OFFSET))
        return (0);
#else
    if (error_code != (ECONNRESET + ERRNO_OFFSET)
#ifdef EPIPE
        && error_code != (EPIPE + ERRNO_OFFSET)
#endif
    )
        return (0);
#endif
    return (1);
}

FT_TEST(test_network_poll_ipv6, "nw_poll IPv6")
{
    SocketConfig server_configuration;
    server_configuration._port = 54328;
    server_configuration._type = SocketType::SERVER;
    server_configuration._address_family = AF_INET6;
    server_configuration._ip = "::1";
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);

    SocketConfig client_configuration;
    client_configuration._port = 54328;
    client_configuration._type = SocketType::CLIENT;
    client_configuration._address_family = AF_INET6;
    client_configuration._ip = "::1";
    ft_socket client(client_configuration);
    if (client.get_error() != ER_SUCCESS)
        return (0);

    struct sockaddr_storage address;
    socklen_t address_length = sizeof(address);
    int client_file_descriptor = nw_accept(server.get_fd(),
                                           reinterpret_cast<struct sockaddr*>(&address),
                                           &address_length);
    if (client_file_descriptor < 0)
        return (0);
    if (nw_set_nonblocking(client_file_descriptor) != 0)
        return (0);

    const char *message = "start";
    if (client.send_all(message, ft_strlen(message), 0)
            != static_cast<ssize_t>(ft_strlen(message)))
        return (0);
    int read_descriptors[1];
    read_descriptors[0] = client_file_descriptor;
    int result = nw_poll(read_descriptors, 1, NULL, 0, 1000);
    if (result != 1 || read_descriptors[0] == -1)
        return (0);
    char buffer[16];
    ssize_t bytes_received = nw_recv(client_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0)
        return (0);
    buffer[bytes_received] = '\0';
    return (ft_strcmp(buffer, message) == 0);
}

struct http_test_server_context
{
    int server_fd;
    bool success;
};

static void http_test_server_run(http_test_server_context *context)
{
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int client_socket;
    char buffer[1024];
    ssize_t bytes_received;
    const char *response_message;
    ssize_t send_result;
    int attempt;

    context->success = false;
    attempt = 0;
    client_socket = -1;
    while (attempt < 200)
    {
        address_length = sizeof(client_address);
        client_socket = nw_accept(context->server_fd,
                                  reinterpret_cast<struct sockaddr*>(&client_address),
                                  &address_length);
        if (client_socket >= 0)
            break;
#ifdef _WIN32
        int error_code = WSAGetLastError();
        if (error_code != WSAEWOULDBLOCK && error_code != WSAEINPROGRESS)
            return ;
#else
        if (errno != EWOULDBLOCK && errno != EAGAIN)
            return ;
#endif
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        attempt++;
    }
    if (client_socket < 0)
        return ;
    bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        FT_CLOSE_SOCKET(client_socket);
        return ;
    }
    buffer[bytes_received] = '\0';
    response_message = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPASS";
    send_result = nw_send(client_socket, response_message, ft_strlen(response_message), 0);
    FT_CLOSE_SOCKET(client_socket);
    if (send_result <= 0)
        return ;
    context->success = true;
    return ;
}

FT_TEST(test_http_client_address_fallback, "http client retries multiple addresses")
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
    bool has_ipv4;
    bool has_ipv6;
    int first_family;
    int server_family;
    const char *server_ip;
    SocketConfig server_configuration;
    http_test_server_context server_context;
    ft_string response;
    const struct sockaddr_in *ipv4_address;
    const struct sockaddr_in6 *ipv6_address;
    struct sockaddr_storage local_address;
    socklen_t local_length;
    int formatted_port;
    char port_string[16];
    int client_result;

    std::memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo("localhost", "80", &address_hints, &address_info) != 0)
        return (0);
    has_ipv4 = false;
    has_ipv6 = false;
    first_family = address_info->ai_family;
    current_info = address_info;
    while (current_info != NULL)
    {
        if (current_info->ai_family == AF_INET)
            has_ipv4 = true;
        else if (current_info->ai_family == AF_INET6)
            has_ipv6 = true;
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (!has_ipv4 || !has_ipv6)
        return (1);
    if (first_family == AF_INET6 && has_ipv4)
    {
        server_family = AF_INET;
        server_ip = "127.0.0.1";
    }
    else if (first_family == AF_INET && has_ipv6)
    {
        server_family = AF_INET6;
        server_ip = "::1";
    }
    else
        return (1);
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = server_ip;
    server_configuration._port = 0;
    server_configuration._address_family = server_family;
    ft_socket server(server_configuration);
    if (server.get_error() != ER_SUCCESS)
        return (0);
    std::memset(&local_address, 0, sizeof(local_address));
    local_length = sizeof(local_address);
    if (getsockname(server.get_fd(), reinterpret_cast<struct sockaddr*>(&local_address), &local_length) != 0)
    {
        server.close_socket();
        return (0);
    }
    std::memset(port_string, 0, sizeof(port_string));
    if (local_address.ss_family == AF_INET)
    {
        ipv4_address = reinterpret_cast<const struct sockaddr_in*>(&local_address);
        formatted_port = std::snprintf(port_string, sizeof(port_string), "%u", static_cast<unsigned int>(ntohs(ipv4_address->sin_port)));
        if (formatted_port <= 0)
        {
            server.close_socket();
            return (0);
        }
    }
    else if (local_address.ss_family == AF_INET6)
    {
        ipv6_address = reinterpret_cast<const struct sockaddr_in6*>(&local_address);
        formatted_port = std::snprintf(port_string, sizeof(port_string), "%u", static_cast<unsigned int>(ntohs(ipv6_address->sin6_port)));
        if (formatted_port <= 0)
        {
            server.close_socket();
            return (0);
        }
    }
    else
    {
        server.close_socket();
        return (0);
    }
    if (nw_set_nonblocking(server.get_fd()) != 0)
    {
        server.close_socket();
        return (0);
    }
    server_context.server_fd = server.get_fd();
    server_context.success = false;
    std::thread server_thread(http_test_server_run, &server_context);
    response.clear();
    client_result = http_get("localhost", "/", response, false, port_string);
    server_thread.join();
    server.close_socket();
    if (!server_context.success)
        return (0);
    if (client_result != 0)
        return (0);
    return (response == "HTTP/1.1 200 OK\r\nContent-Length: 4\r\n\r\nPASS");
}

FT_TEST(test_http_client_plain_partial_retry, "http client retries partial nw_send")
{
    const char *request_string;
    network_io_harness harness;
    throttled_transfer_result read_result;
    size_t request_length;
    std::thread reader_thread;
    int buffer_result;
    int send_result;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    request_length = ft_strlen(request_string);
    if (harness.initialize(0) != ER_SUCCESS)
        return (0);
    buffer_result = harness.set_client_send_buffer(32);
    if (buffer_result != ER_SUCCESS)
        return (0);
    reader_thread = std::thread([&harness, request_length, &read_result]()
    {
        throttled_read_loop(harness.get_server_fd(), request_length, 8, 10, &read_result);
    });
    send_result = http_client_send_plain_request(harness.get_client_fd(), request_string, request_length);
    if (reader_thread.joinable())
        reader_thread.join();
    if (send_result != 0)
        return (0);
    if (read_result.status != 0)
        return (0);
    if (read_result.data != request_string)
        return (0);
    return (read_result.read_iterations > 1);
}

FT_TEST(test_http_client_plain_short_write_sets_errno, "http client detects short write")
{
    const char *request_string;
    int result;
    network_io_harness harness;
    std::thread closer_thread;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    ft_errno = ER_SUCCESS;
    if (harness.initialize(0) != ER_SUCCESS)
        return (0);
    closer_thread = std::thread([&harness]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        harness.close_server();
    });
    result = http_client_send_plain_request(harness.get_client_fd(), request_string, ft_strlen(request_string));
    if (closer_thread.joinable())
        closer_thread.join();
    if (result != -1)
        return (0);
    if (ft_errno == SOCKET_SEND_FAILED)
        return (1);
#ifdef _WIN32
    if (ft_errno == (WSAECONNRESET + ERRNO_OFFSET)
        || ft_errno == (WSAENOTCONN + ERRNO_OFFSET)
        || ft_errno == (WSAECONNABORTED + ERRNO_OFFSET))
        return (1);
#else
    if (ft_errno == (ECONNRESET + ERRNO_OFFSET))
        return (1);
#ifdef EPIPE
    if (ft_errno == (EPIPE + ERRNO_OFFSET))
        return (1);
#endif
#endif
    return (0);
}

FT_TEST(test_http_client_ssl_partial_retry, "http client retries partial SSL write")
{
    const char *request_string;
    network_io_harness harness;
    SSL_CTX *client_context;
    SSL_CTX *server_context;
    SSL *client_ssl;
    SSL *server_ssl;
    int initialization_result;
    int connect_result;
    int accept_result;
    throttled_transfer_result read_result;
    std::thread accept_thread;
    std::thread reader_thread;
    size_t request_length;
    int send_result;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    request_length = ft_strlen(request_string);
    if (harness.initialize(0) != ER_SUCCESS)
        return (0);
    initialization_result = initialize_ssl_pair(harness, &client_context, &client_ssl, &server_context, &server_ssl);
    if (initialization_result != ER_SUCCESS)
        return (0);
    accept_result = 0;
    accept_thread = std::thread([server_ssl, &accept_result]()
    {
        if (SSL_accept(server_ssl) <= 0)
            accept_result = 1;
    });
    connect_result = SSL_connect(client_ssl);
    if (accept_thread.joinable())
        accept_thread.join();
    if (connect_result <= 0 || accept_result != 0)
    {
        cleanup_ssl_pair(client_context, client_ssl, server_context, server_ssl);
        return (0);
    }
    if (harness.set_client_send_buffer(32) != ER_SUCCESS)
    {
        cleanup_ssl_pair(client_context, client_ssl, server_context, server_ssl);
        return (0);
    }
    reader_thread = std::thread([server_ssl, request_length, &read_result]()
    {
        throttled_ssl_read_loop(server_ssl, request_length, 8, 10, &read_result);
    });
    send_result = http_client_send_ssl_request(client_ssl, request_string, request_length);
    if (reader_thread.joinable())
        reader_thread.join();
    cleanup_ssl_pair(client_context, client_ssl, server_context, server_ssl);
    if (send_result != 0)
        return (0);
    if (read_result.status != 0)
        return (0);
    if (read_result.data != request_string)
        return (0);
    return (read_result.read_iterations > 1);
}

FT_TEST(test_http_client_ssl_short_write_sets_errno, "http client SSL detects short write")
{
    const char *request_string;
    int result;
    network_io_harness harness;
    SSL_CTX *client_context;
    SSL_CTX *server_context;
    SSL *client_ssl;
    SSL *server_ssl;
    int initialization_result;
    int connect_result;
    int accept_result;
    std::thread accept_thread;
    std::thread closer_thread;

    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    ft_errno = ER_SUCCESS;
    if (harness.initialize(0) != ER_SUCCESS)
        return (0);
    initialization_result = initialize_ssl_pair(harness, &client_context, &client_ssl, &server_context, &server_ssl);
    if (initialization_result != ER_SUCCESS)
        return (0);
    accept_result = 0;
    accept_thread = std::thread([server_ssl, &accept_result]()
    {
        if (SSL_accept(server_ssl) <= 0)
            accept_result = 1;
    });
    connect_result = SSL_connect(client_ssl);
    if (accept_thread.joinable())
        accept_thread.join();
    if (connect_result <= 0 || accept_result != 0)
    {
        cleanup_ssl_pair(client_context, client_ssl, server_context, server_ssl);
        return (0);
    }
    closer_thread = std::thread([&harness]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        harness.close_server();
    });
    result = http_client_send_ssl_request(client_ssl, request_string, ft_strlen(request_string));
    if (closer_thread.joinable())
        closer_thread.join();
    cleanup_ssl_pair(client_context, client_ssl, server_context, server_ssl);
    if (result != -1)
        return (0);
    if (ft_errno == SOCKET_SEND_FAILED)
        return (1);
#ifdef _WIN32
    if (ft_errno == (WSAECONNRESET + ERRNO_OFFSET)
        || ft_errno == (WSAENOTCONN + ERRNO_OFFSET)
        || ft_errno == (WSAECONNABORTED + ERRNO_OFFSET))
        return (1);
#else
    if (ft_errno == (ECONNRESET + ERRNO_OFFSET))
        return (1);
#ifdef EPIPE
    if (ft_errno == (EPIPE + ERRNO_OFFSET))
        return (1);
#endif
#endif
    return (0);
}

