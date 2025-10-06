#include "../../API/api_http_internal.hpp"
#include "../../API/api.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../PThread/thread.hpp"
#ifndef _WIN32
# include <errno.h>
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <atomic>
#include <cstdint>
#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

static const uint16_t g_api_authorization_bearer_port = 54710;
static const uint16_t g_api_authorization_basic_port = 54711;

struct api_request_header_server_context
{
    std::atomic<bool> ready;
    std::atomic<int> result;
    ft_string request;
    uint16_t port;
};

static void api_request_test_sleep_small(void)
{
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    return ;
}

static void api_request_authorization_server(
    api_request_header_server_context *context)
{
    if (!context)
        return ;
    context->result.store(ER_SUCCESS);
    SocketConfig server_configuration;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = context->port;
    ft_socket server_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
    {
        context->result.store(server_socket.get_error());
        context->ready.store(true);
        return ;
    }
    context->ready.store(true);
    struct sockaddr_storage address_storage;
    socklen_t address_length;

    address_length = sizeof(address_storage);
    int client_fd;

    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
    {
        context->result.store(FT_EIO);
        return ;
    }
    bool connection_active;
    bool header_complete;
    size_t terminator_match;
    ft_string request_storage;

    connection_active = true;
    header_complete = false;
    terminator_match = 0;
    while (connection_active && !header_complete)
    {
        char buffer[1024];
        ssize_t bytes_received;

        bytes_received = nw_recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            connection_active = false;
            break ;
        }
        size_t buffer_index;

        buffer_index = 0;
        while (buffer_index < static_cast<size_t>(bytes_received))
        {
            char current_char;

            current_char = buffer[buffer_index];
            request_storage.append(current_char);
            if (request_storage.get_error() != ER_SUCCESS)
            {
                context->result.store(request_storage.get_error());
                connection_active = false;
                break ;
            }
            if (current_char == '\r')
            {
                if (terminator_match == 0 || terminator_match == 2)
                    terminator_match++;
                else
                    terminator_match = 1;
            }
            else if (current_char == '\n')
            {
                if (terminator_match == 1 || terminator_match == 3)
                    terminator_match++;
                else
                    terminator_match = 0;
            }
            else
                terminator_match = 0;
            if (terminator_match == 4)
            {
                header_complete = true;
                break ;
            }
            buffer_index++;
        }
    }
    if (header_complete)
    {
        context->request = request_storage;
        if (context->request.get_error() != ER_SUCCESS)
            context->result.store(context->request.get_error());
    }
    else if (context->result.load() == ER_SUCCESS)
        context->result.store(FT_EIO);
    if (connection_active && header_complete
            && context->result.load() == ER_SUCCESS)
    {
        const char *response;
        size_t response_length;
        size_t total_sent;

        response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\nConnection: keep-alive\r\n\r\nHello";
        response_length = ft_strlen(response);
        total_sent = 0;
        while (total_sent < response_length)
        {
            ssize_t bytes_sent;

            bytes_sent = nw_send(client_fd, response + total_sent,
                    response_length - total_sent, 0);
            if (bytes_sent <= 0)
            {
                context->result.store(FT_EIO);
                break ;
            }
            total_sent += static_cast<size_t>(bytes_sent);
        }
    }
    if (client_fd >= 0)
        FT_CLOSE_SOCKET(client_fd);
    return ;
}

static void api_request_authorization_server_wake(uint16_t port)
{
    SocketConfig client_configuration;

    client_configuration._type = SocketType::CLIENT;
    client_configuration._ip = "127.0.0.1";
    client_configuration._port = port;
    client_configuration._recv_timeout = 100;
    client_configuration._send_timeout = 100;
    ft_socket wake_socket(client_configuration);
    if (wake_socket.get_error() == ER_SUCCESS)
        wake_socket.close_socket();
    return ;
}

typedef char *(*api_host_authorization_function)(const char *host,
        uint16_t port, const char *method, const char *path,
        const char *credential, json_group *payload, const char *headers,
        int *status, int timeout, const api_retry_policy *retry_policy);

static int api_request_run_authorization_header_test(
    api_host_authorization_function function, const char *credential,
    const char *expected_header, const char *existing_header,
    uint16_t port)
{
    api_request_header_server_context context;

    context.ready.store(false);
    context.result.store(ER_SUCCESS);
    context.request.clear();
    context.port = port;
    ft_thread server_thread(api_request_authorization_server, &context);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    int wait_attempts;

    wait_attempts = 0;
    while (!context.ready.load())
    {
        if (wait_attempts > 100)
        {
            server_thread.join();
            return (0);
        }
        api_request_test_sleep_small();
        wait_attempts++;
    }
    api_connection_pool_flush();
    int status_value;
    char *body;

    status_value = 0;
    ft_errno = ER_SUCCESS;
    body = function("127.0.0.1", port, "GET", "/auth", credential,
            ft_nullptr, existing_header, &status_value, 2000, ft_nullptr);
    if (!body)
    {
        api_request_authorization_server_wake(port);
        server_thread.join();
        api_connection_pool_flush();
        return (0);
    }
    bool body_valid;

    body_valid = (ft_strncmp(body, "Hello", 5) == 0);
    cma_free(body);
    server_thread.join();
    api_connection_pool_flush();
    if (context.result.load() != ER_SUCCESS)
        return (0);
    if (!body_valid)
        return (0);
    if (status_value != 200)
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    const char *request_text;

    request_text = context.request.c_str();
    if (!request_text)
        return (0);
    if (!ft_strstr(request_text, "GET /auth HTTP/1.1"))
        return (0);
    if (!ft_strstr(request_text, "Host: 127.0.0.1"))
        return (0);
    if (!ft_strstr(request_text, "Connection: keep-alive"))
        return (0);
    if (existing_header && existing_header[0])
    {
        if (!ft_strstr(request_text, existing_header))
            return (0);
    }
    if (!ft_strstr(request_text, expected_header))
        return (0);
    return (1);
}

FT_TEST(test_api_request_string_host_validates_arguments,
    "api_request_string_host returns null and sets errno on invalid arguments")
{
    ft_errno = ER_SUCCESS;
    int status_value;
    char *result_body;

    status_value = 0;
    result_body = api_request_string_host(ft_nullptr, 80, "GET", "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    ft_errno = ER_SUCCESS;
    status_value = 0;
    result_body = api_request_string_host("localhost", 80, ft_nullptr, "/",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    ft_errno = ER_SUCCESS;
    status_value = 0;
    result_body = api_request_string_host("localhost", 80, "GET",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_api_request_string_url_validates_arguments,
    "api_request_string_url validates url and method parameters")
{
    int status_value;
    char *result_body;

    ft_errno = ER_SUCCESS;
    status_value = 0;
    result_body = api_request_string_url(ft_nullptr, "GET", ft_nullptr,
            ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    ft_errno = ER_SUCCESS;
    status_value = 0;
    result_body = api_request_string_url("example.com/test", "GET",
            ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    ft_errno = ER_SUCCESS;
    status_value = 0;
    result_body = api_request_string_url("http://example.com/test",
            ft_nullptr, ft_nullptr, ft_nullptr, &status_value, 1000);
    FT_ASSERT(result_body == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_resolve_error_maps_known_codes,
    "api_request_set_resolve_error maps resolver codes to errno values")
{
#ifdef EAI_BADFLAGS
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_BADFLAGS);
    FT_ASSERT_EQ(SOCKET_RESOLVE_BAD_FLAGS, ft_errno);
#endif
#ifdef EAI_AGAIN
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_AGAIN);
    FT_ASSERT_EQ(SOCKET_RESOLVE_AGAIN, ft_errno);
#endif
#ifdef EAI_FAIL
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_FAIL);
    FT_ASSERT_EQ(SOCKET_RESOLVE_FAIL, ft_errno);
#endif
#ifdef EAI_FAMILY
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_FAMILY);
    FT_ASSERT_EQ(SOCKET_RESOLVE_FAMILY, ft_errno);
#endif
#ifdef EAI_ADDRFAMILY
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_ADDRFAMILY);
    FT_ASSERT_EQ(SOCKET_RESOLVE_FAMILY, ft_errno);
#endif
#ifdef EAI_SOCKTYPE
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_SOCKTYPE);
    FT_ASSERT_EQ(SOCKET_RESOLVE_SOCKTYPE, ft_errno);
#endif
#ifdef EAI_SERVICE
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_SERVICE);
    FT_ASSERT_EQ(SOCKET_RESOLVE_SERVICE, ft_errno);
#endif
#ifdef EAI_MEMORY
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_MEMORY);
    FT_ASSERT_EQ(SOCKET_RESOLVE_MEMORY, ft_errno);
#endif
#ifdef EAI_NONAME
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_NONAME);
    FT_ASSERT_EQ(SOCKET_RESOLVE_NO_NAME, ft_errno);
#endif
#ifdef EAI_NODATA
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_NODATA);
    FT_ASSERT_EQ(SOCKET_RESOLVE_NO_NAME, ft_errno);
#endif
#ifdef EAI_OVERFLOW
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_OVERFLOW);
    FT_ASSERT_EQ(SOCKET_RESOLVE_OVERFLOW, ft_errno);
#endif
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(12345);
    FT_ASSERT_EQ(SOCKET_RESOLVE_FAILED, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_resolve_error_handles_system_errno,
    "api_request_set_resolve_error handles system errno reporting")
{
#ifdef EAI_SYSTEM
#ifdef _WIN32
    FT_ASSERT_EQ(1, 1);
#else
    int previous_errno;

    previous_errno = errno;
    errno = EDOM;
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_SYSTEM);
    FT_ASSERT_EQ(EDOM + ERRNO_OFFSET, ft_errno);
    errno = 0;
    ft_errno = ER_SUCCESS;
    api_request_set_resolve_error(EAI_SYSTEM);
    FT_ASSERT_EQ(SOCKET_RESOLVE_FAIL, ft_errno);
    errno = previous_errno;
#endif
#endif
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_prefers_openssl_queue,
    "api_request_set_ssl_error prioritizes library errors over session state")
{
    unsigned long expected_error;

    ERR_clear_error();
    ERR_put_error(ERR_LIB_SSL, 0, SSL_R_UNSUPPORTED_PROTOCOL, __FILE__, __LINE__);
    expected_error = ERR_peek_error();
    FT_ASSERT(expected_error != 0);
    ft_errno = ER_SUCCESS;
    api_request_set_ssl_error(ft_nullptr, 0);
    FT_ASSERT_EQ(static_cast<int>(expected_error), ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_handles_missing_session,
    "api_request_set_ssl_error falls back to FT_EIO without a session")
{
    ERR_clear_error();
    ft_errno = ER_SUCCESS;
    api_request_set_ssl_error(ft_nullptr, 0);
    FT_ASSERT_EQ(FT_EIO, ft_errno);
    return (1);
}

FT_TEST(test_api_request_set_ssl_error_reports_errno_for_syscall,
    "api_request_set_ssl_error propagates errno when SSL_ERROR_SYSCALL occurs")
{
#ifdef _WIN32
    FT_ASSERT_EQ(1, 1);
#else
    SSL_CTX *context;
    SSL *session;
    int previous_errno;

    if (!OPENSSL_init_ssl(0, ft_nullptr))
        return (0);
    context = SSL_CTX_new(TLS_method());
    if (!context)
        return (0);
    session = SSL_new(context);
    if (!session)
    {
        SSL_CTX_free(context);
        return (0);
    }
    ERR_clear_error();
    previous_errno = errno;
    errno = EDOM;
    ft_errno = ER_SUCCESS;
    api_request_set_ssl_error(session, -1);
    FT_ASSERT_EQ(EDOM + ERRNO_OFFSET, ft_errno);
    errno = 0;
    ft_errno = ER_SUCCESS;
    api_request_set_ssl_error(session, -1);
    FT_ASSERT_EQ(SSL_ERROR_SYSCALL + ERRNO_OFFSET, ft_errno);
    errno = previous_errno;
    SSL_free(session);
    SSL_CTX_free(context);
#endif
    return (1);
}

FT_TEST(test_api_request_string_host_bearer_appends_authorization_header,
    "api_request_string_host_bearer adds bearer authorization header")
{
    FT_ASSERT_EQ(1, api_request_run_authorization_header_test(
            api_request_string_host_bearer, "secret-token",
            "Authorization: Bearer secret-token", ft_nullptr,
            g_api_authorization_bearer_port));
    return (1);
}

FT_TEST(test_api_request_string_host_basic_appends_authorization_header,
    "api_request_string_host_basic adds basic authorization header after existing headers")
{
    FT_ASSERT_EQ(1, api_request_run_authorization_header_test(
            api_request_string_host_basic, "dXNlcjpwYXNz",
            "X-Test: value\r\nAuthorization: Basic dXNlcjpwYXNz",
            "X-Test: value", g_api_authorization_basic_port));
    return (1);
}
