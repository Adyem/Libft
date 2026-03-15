#include "../test_internal.hpp"
#include "../../Networking/http_server.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../PThread/thread.hpp"
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstddef>

#ifndef LIBFT_TEST_BUILD
#endif

#define FT_TEST_REQUIRE(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            ft_test_fail(#condition, __FILE__, __LINE__); \
            goto cleanup; \
        } \
    } while (0)

struct http_server_context
{
    ft_http_server *server;
    int result;
};

static void http_server_run_once(http_server_context *context)
{
    if (context == ft_nullptr)
        return ;
    if (context->server == ft_nullptr)
        return ;
    context->result = context->server->run_once();
    return ;
}

static int collect_response(int socket_fd, ft_string &response)
{
    char buffer[256];
    ssize_t bytes_received;

    response.clear();
    while (1)
    {
        bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            break;
        buffer[bytes_received] = '\0';
        response.append(buffer);
    }
    if (response.empty())
        return (0);
    return (1);
}

FT_TEST(test_http_server_get_response)
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_string;
    ft_string response;
    ft_bool thread_started;
    ft_bool client_initialized;
    ft_bool response_initialized;
    ft_bool test_passed;

    thread_started = FT_FALSE;
    client_initialized = FT_FALSE;
    response_initialized = FT_FALSE;
    test_passed = FT_FALSE;
    context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54330) == FT_ERR_SUCCESS);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    FT_TEST_REQUIRE(server_thread.joinable());
    thread_started = FT_TRUE;
    FT_TEST_REQUIRE(response.initialize() == FT_ERR_SUCCESS);
    response_initialized = FT_TRUE;
    FT_TEST_REQUIRE(client_configuration.initialize() == FT_ERR_SUCCESS);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._port = 54330;
    FT_TEST_REQUIRE(client_socket.initialize(client_configuration) == FT_ERR_SUCCESS);
    client_initialized = FT_TRUE;
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    FT_TEST_REQUIRE(client_socket.send_all(request_string, ft_strlen(request_string), 0)
        == static_cast<ssize_t>(ft_strlen(request_string)));
    FT_TEST_REQUIRE(collect_response(client_socket.get_file_descriptor(), response) == 1);
    FT_TEST_REQUIRE(context.result == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(ft_strnstr(response.c_str(), "HTTP/1.1 200 OK", response.size()) != ft_nullptr);
    FT_TEST_REQUIRE(ft_strnstr(response.c_str(), "GET", response.size()) != ft_nullptr);
    test_passed = FT_TRUE;
cleanup:
    if (response_initialized == FT_TRUE)
        (void)response.destroy();
    if (client_initialized == FT_TRUE)
        (void)client_socket.destroy();
    if (thread_started == FT_TRUE)
        server_thread.join();
    (void)server.destroy();
    if (test_passed == FT_FALSE)
        return (0);
    return (1);
}

FT_TEST(test_http_server_short_write_sets_error)
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_string;
    int error_code;
    ft_bool thread_started;
    ft_bool client_initialized;
    ft_bool test_passed;

    error_code = FT_ERR_INVALID_STATE;
    thread_started = FT_FALSE;
    client_initialized = FT_FALSE;
    test_passed = FT_FALSE;
    context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54332) == FT_ERR_SUCCESS);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    FT_TEST_REQUIRE(server_thread.joinable());
    thread_started = FT_TRUE;
    FT_TEST_REQUIRE(client_configuration.initialize() == FT_ERR_SUCCESS);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._port = 54332;
    FT_TEST_REQUIRE(client_socket.initialize(client_configuration) == FT_ERR_SUCCESS);
    client_initialized = FT_TRUE;
    request_string = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    client_socket.send_all(request_string, ft_strlen(request_string), 0);
    client_socket.close_socket();
    FT_TEST_REQUIRE(context.result != FT_ERR_SUCCESS);
    error_code = context.result;
    FT_TEST_REQUIRE(error_code != FT_ERR_SUCCESS);
    test_passed = FT_TRUE;
cleanup:
    if (client_initialized == FT_TRUE)
        (void)client_socket.destroy();
    if (thread_started == FT_TRUE)
        server_thread.join();
    (void)server.destroy();
    if (test_passed == FT_FALSE)
        return (0);
    return (1);
}

FT_TEST(test_http_server_post_echoes_body)
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *body_content;
    ft_string request_string;
    char content_length_buffer[32];
    ft_string response;
    const char *status_match;
    const char *body_start;
    size_t body_length;
    ft_bool thread_started;
    ft_bool client_initialized;
    ft_bool request_initialized;
    ft_bool response_initialized;
    ft_bool test_passed;

    thread_started = FT_FALSE;
    client_initialized = FT_FALSE;
    request_initialized = FT_FALSE;
    response_initialized = FT_FALSE;
    test_passed = FT_FALSE;
    context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54331) == FT_ERR_SUCCESS);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    FT_TEST_REQUIRE(server_thread.joinable());
    thread_started = FT_TRUE;
    FT_TEST_REQUIRE(request_string.initialize() == FT_ERR_SUCCESS);
    request_initialized = FT_TRUE;
    FT_TEST_REQUIRE(response.initialize() == FT_ERR_SUCCESS);
    response_initialized = FT_TRUE;
    FT_TEST_REQUIRE(client_configuration.initialize() == FT_ERR_SUCCESS);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._port = 54331;
    FT_TEST_REQUIRE(client_socket.initialize(client_configuration) == FT_ERR_SUCCESS);
    client_initialized = FT_TRUE;
    body_content = "payload=echo-body";
    body_length = static_cast<size_t>(ft_strlen(body_content));
    request_string = "POST /echo HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\nContent-Length: ";
    std::snprintf(content_length_buffer, sizeof(content_length_buffer), "%zu", body_length);
    request_string.append(content_length_buffer);
    request_string.append("\r\n\r\n");
    request_string.append(body_content);
    FT_TEST_REQUIRE(client_socket.send_all(request_string.c_str(), request_string.size(), 0)
        == static_cast<ssize_t>(request_string.size()));
    FT_TEST_REQUIRE(collect_response(client_socket.get_file_descriptor(), response) == 1);
    FT_TEST_REQUIRE(context.result == FT_ERR_SUCCESS);
    status_match = ft_strnstr(response.c_str(), "HTTP/1.1 200 OK", response.size());
    FT_TEST_REQUIRE(status_match != ft_nullptr);
    body_start = ft_strnstr(response.c_str(), "\r\n\r\n", response.size());
    FT_TEST_REQUIRE(body_start != ft_nullptr);
    body_start += 4;
    FT_TEST_REQUIRE(ft_strncmp(body_start, body_content, body_length) == 0);
    test_passed = FT_TRUE;
cleanup:
    if (response_initialized == FT_TRUE)
        (void)response.destroy();
    if (request_initialized == FT_TRUE)
        (void)request_string.destroy();
    if (client_initialized == FT_TRUE)
        (void)client_socket.destroy();
    if (thread_started == FT_TRUE)
        server_thread.join();
    (void)server.destroy();
    if (test_passed == FT_FALSE)
        return (0);
    return (1);
}

FT_TEST(test_http_server_keep_alive_multiple_requests)
{
    ft_http_server server;
    http_server_context context;
    ft_thread server_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    ft_string request_string;
    ft_string response;
    const char *first_response;
    const char *second_response;
    ft_bool thread_started;
    ft_bool client_initialized;
    ft_bool request_initialized;
    ft_bool response_initialized;
    ft_bool test_passed;

    thread_started = FT_FALSE;
    client_initialized = FT_FALSE;
    request_initialized = FT_FALSE;
    response_initialized = FT_FALSE;
    test_passed = FT_FALSE;
    context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54334) == FT_ERR_SUCCESS);
    context.server = &server;
    context.result = -1;
    server_thread = ft_thread(http_server_run_once, &context);
    FT_TEST_REQUIRE(server_thread.joinable());
    thread_started = FT_TRUE;
    FT_TEST_REQUIRE(request_string.initialize() == FT_ERR_SUCCESS);
    request_initialized = FT_TRUE;
    FT_TEST_REQUIRE(response.initialize() == FT_ERR_SUCCESS);
    response_initialized = FT_TRUE;
    FT_TEST_REQUIRE(client_configuration.initialize() == FT_ERR_SUCCESS);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._port = 54334;
    FT_TEST_REQUIRE(client_socket.initialize(client_configuration) == FT_ERR_SUCCESS);
    client_initialized = FT_TRUE;
    request_string = "GET /first HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n";
    request_string.append("GET /second HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n");
    FT_TEST_REQUIRE(client_socket.send_all(request_string.c_str(), request_string.size(), 0)
        == static_cast<ssize_t>(request_string.size()));
    FT_TEST_REQUIRE(collect_response(client_socket.get_file_descriptor(), response) == 1);
    FT_TEST_REQUIRE(context.result == FT_ERR_SUCCESS);
    first_response = ft_strnstr(response.c_str(), "Connection: keep-alive", response.size());
    FT_TEST_REQUIRE(first_response != ft_nullptr);
    second_response = ft_strnstr(response.c_str() + (first_response - response.c_str()) + 1,
        "Connection: close", response.size() - (first_response - response.c_str()) - 1);
    FT_TEST_REQUIRE(second_response != ft_nullptr);
    test_passed = FT_TRUE;
cleanup:
    if (response_initialized == FT_TRUE)
        (void)response.destroy();
    if (request_initialized == FT_TRUE)
        (void)request_string.destroy();
    if (client_initialized == FT_TRUE)
        (void)client_socket.destroy();
    if (thread_started == FT_TRUE)
        server_thread.join();
    (void)server.destroy();
    if (test_passed == FT_FALSE)
        return (0);
    return (1);
}

FT_TEST(test_http_server_thread_safe_run_once)
{
    ft_http_server server;
    http_server_context run_context;
    ft_thread server_thread;
    ft_thread error_thread;
    SocketConfig client_configuration;
    ft_socket client_socket;
    const char *request_string;
    ft_string response;
    ft_bool run_thread_started;
    ft_bool error_thread_started;
    ft_bool client_initialized;
    ft_bool response_initialized;
    ft_bool test_passed;

    run_thread_started = FT_FALSE;
    error_thread_started = FT_FALSE;
    client_initialized = FT_FALSE;
    response_initialized = FT_FALSE;
    test_passed = FT_FALSE;
    run_context.result = FT_ERR_INVALID_STATE;
    FT_TEST_REQUIRE(server.initialize() == FT_ERR_SUCCESS);
    FT_TEST_REQUIRE(server.start("127.0.0.1", 54336) == FT_ERR_SUCCESS);
    run_context.server = &server;
    run_context.result = -1;
    server_thread = ft_thread(http_server_run_once, &run_context);
    FT_TEST_REQUIRE(server_thread.joinable());
    run_thread_started = FT_TRUE;
    usleep(50000);
    error_thread = ft_thread(http_server_run_once, &run_context);
    FT_TEST_REQUIRE(error_thread.joinable());
    error_thread_started = FT_TRUE;
    FT_TEST_REQUIRE(response.initialize() == FT_ERR_SUCCESS);
    response_initialized = FT_TRUE;
    FT_TEST_REQUIRE(client_configuration.initialize() == FT_ERR_SUCCESS);
    client_configuration._type = SocketType::CLIENT;
    std::strncpy(client_configuration._ip, "127.0.0.1",
        sizeof(client_configuration._ip) - 1);
    client_configuration._ip[sizeof(client_configuration._ip) - 1] = '\0';
    client_configuration._port = 54336;
    FT_TEST_REQUIRE(client_socket.initialize(client_configuration) == FT_ERR_SUCCESS);
    client_initialized = FT_TRUE;
    request_string = "GET /thread HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";
    FT_TEST_REQUIRE(client_socket.send_all(request_string, ft_strlen(request_string), 0)
        == static_cast<ssize_t>(ft_strlen(request_string)));
    FT_TEST_REQUIRE(collect_response(client_socket.get_file_descriptor(), response) == 1);
    FT_TEST_REQUIRE(run_context.result == FT_ERR_SUCCESS || run_context.result == 1);
    FT_TEST_REQUIRE(ft_strnstr(response.c_str(), "HTTP/1.1 200 OK", response.size()) != ft_nullptr);
    test_passed = FT_TRUE;
cleanup:
    if (response_initialized == FT_TRUE)
        (void)response.destroy();
    if (client_initialized == FT_TRUE)
        (void)client_socket.destroy();
    if (error_thread_started == FT_TRUE)
        error_thread.join();
    if (run_thread_started == FT_TRUE)
        server_thread.join();
    (void)server.destroy();
    if (test_passed == FT_FALSE)
        return (0);
    return (1);
}
