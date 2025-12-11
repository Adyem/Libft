#include "../../API/api.hpp"
#include "../../API/api_internal.hpp"
#include "../../API/api_http_internal.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/http2_client.hpp"
#include "../../PThread/thread.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Printf/printf.hpp"
#include "../../Libft/libft.hpp"
#include "../../Time/time.hpp"
#include <cerrno>
#include <csignal>
#include <climits>
#include <atomic>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

static void api_request_noop_callback(char *body, int status, void *user_data)
{
    (void)body;
    (void)status;
    (void)user_data;
    return ;
}

FT_TEST(test_api_request_hmac_signature_basic,
    "api_sign_request_hmac_sha256 produces expected digest")
{
    api_hmac_signature_input input;
    unsigned char key_buffer[6];
    ft_string signature;
    size_t index;

    input.method = "POST";
    input.path = "/v1/resource";
    input.canonical_headers = "content-type:application/json";
    input.canonical_query = ft_nullptr;
    input.body = "{\"ok\":true}";
    index = 0;
    while (index < sizeof(key_buffer))
    {
        key_buffer[index] = "secret"[index];
        index += 1;
    }
    signature = "";
    if (api_sign_request_hmac_sha256(input, key_buffer,
            sizeof(key_buffer), signature) != 0)
        return (0);
    if (ft_strcmp(signature.c_str(),
            "jyahK7KdXeTAsWB9y99qdzuUS5V6UK8fdyx51G18uBY=") != 0)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

FT_TEST(test_api_request_oauth1_header_hmac_sha256,
    "api_build_oauth1_authorization_header emits oauth signature")
{
    api_oauth1_parameter extras[1];
    api_oauth1_parameters params;
    ft_string header;

    extras[0].key = "status";
    extras[0].value = "active";
    params.method = "POST";
    params.url = "https://api.example.com/resource";
    params.consumer_key = "key123";
    params.consumer_secret = "secret456";
    params.token = "token789";
    params.token_secret = "tokensecret";
    params.timestamp = "1700000000";
    params.nonce = "randomstring";
    params.signature_method = "HMAC-SHA256";
    params.version = "1.0";
    params.additional_parameters = extras;
    params.additional_parameter_count = 1;
    header = "";
    if (api_build_oauth1_authorization_header(params, header) != 0)
        return (0);
    if (ft_strcmp(header.c_str(),
            "Authorization: OAuth oauth_consumer_key=\"key123\", "
            "oauth_nonce=\"randomstring\", oauth_signature_method=\"HMAC-SHA256\", "
            "oauth_timestamp=\"1700000000\", oauth_token=\"token789\", "
            "oauth_version=\"1.0\", status=\"active\", "
            "oauth_signature=\"Lih20ttr%2Fmuyb8m5AqpjPKa%2FNzc%2BYT%2FRvRZKId3RS2o%3D\"") != 0)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

struct api_stream_test_context
{
    size_t total_bytes;
    size_t chunk_count;
    bool headers_received;
    bool final_received;
    int status_code;
};

static bool g_api_request_test_http2_called = false;
static bool g_api_request_test_http1_called = false;

static bool api_request_test_stream_http2_hook(const char *ip, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout, bool *used_http2,
    const api_retry_policy *retry_policy, void *user_data)
{
    (void)ip;
    (void)port;
    (void)method;
    (void)path;
    (void)streaming_handler;
    (void)payload;
    (void)headers;
    (void)timeout;
    (void)retry_policy;
    (void)user_data;
    g_api_request_test_http2_called = true;
    if (used_http2)
        *used_http2 = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

static bool api_request_test_stream_http1_hook(const char *ip, uint16_t port,
    const char *method, const char *path,
    const api_streaming_handler *streaming_handler, json_group *payload,
    const char *headers, int timeout,
    const api_retry_policy *retry_policy, void *user_data)
{
    (void)ip;
    (void)port;
    (void)method;
    (void)path;
    (void)streaming_handler;
    (void)payload;
    (void)headers;
    (void)timeout;
    (void)retry_policy;
    (void)user_data;
    g_api_request_test_http1_called = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (true);
}

FT_TEST(test_api_request_prefers_http2_streaming,
    "api_request routes to http2 stream hook when enabled")
{
    api_transport_hooks hooks;
    api_streaming_handler handler;
    bool used_http2;
    bool request_result;

    ft_memset(&hooks, 0, sizeof(hooks));
    hooks.request_stream = api_request_test_stream_http1_hook;
    hooks.request_stream_http2 = api_request_test_stream_http2_hook;
    hooks.user_data = ft_nullptr;
    api_set_transport_hooks(&hooks);
    g_api_request_test_http1_called = false;
    g_api_request_test_http2_called = false;
    handler.reset();
    ft_errno = FT_ERR_SUCCESSS;
    request_result = api_request("127.0.0.1", 8080, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 0, true, &used_http2, ft_nullptr);
    api_clear_transport_hooks();
    if (!request_result)
        return (0);
    if (!g_api_request_test_http2_called)
        return (0);
    if (g_api_request_test_http1_called)
        return (0);
    if (!used_http2)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

FT_TEST(test_api_request_disables_http2_streaming,
    "api_request routes to http1 stream hook when http2 disabled")
{
    api_transport_hooks hooks;
    api_streaming_handler handler;
    bool used_http2;
    bool request_result;

    ft_memset(&hooks, 0, sizeof(hooks));
    hooks.request_stream = api_request_test_stream_http1_hook;
    hooks.request_stream_http2 = api_request_test_stream_http2_hook;
    hooks.user_data = ft_nullptr;
    api_set_transport_hooks(&hooks);
    g_api_request_test_http1_called = false;
    g_api_request_test_http2_called = false;
    handler.reset();
    ft_errno = FT_ERR_SUCCESSS;
    request_result = api_request("127.0.0.1", 8080, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 0, false, &used_http2, ft_nullptr);
    api_clear_transport_hooks();
    if (!request_result)
        return (0);
    if (!g_api_request_test_http1_called)
        return (0);
    if (g_api_request_test_http2_called)
        return (0);
    if (used_http2)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

struct api_request_bearer_server_context
{
    std::atomic<bool> ready;
    std::atomic<bool> header_received;
    std::atomic<int> result;
    int client_fd;
    ft_string request_data;
};

struct api_request_basic_server_context
{
    std::atomic<bool> ready;
    std::atomic<bool> basic_header_received;
    std::atomic<bool> basic_header_after_custom;
    std::atomic<int> result;
    int client_fd;
    ft_string request_data;
};

struct api_request_circuit_server_context
{
    std::atomic<bool> ready;
    uint16_t port;
    int responses;
};

static void api_request_stream_headers_callback(int status_code,
    const char *headers, void *user_data)
{
    api_stream_test_context *context;

    (void)headers;
    context = static_cast<api_stream_test_context*>(user_data);
    if (!context)
        return ;
    context->headers_received = true;
    context->status_code = status_code;
    return ;
}

static bool api_request_stream_body_callback(const char *chunk_data,
    size_t chunk_size, bool is_final_chunk, void *user_data)
{
    api_stream_test_context *context;

    context = static_cast<api_stream_test_context*>(user_data);
    if (!context)
        return (false);
    if (chunk_data && chunk_size > 0)
    {
        context->total_bytes += chunk_size;
        context->chunk_count += 1;
    }
    if (is_final_chunk)
        context->final_received = true;
    return (true);
}

static std::atomic<size_t> g_api_async_retry_server_bytes_received(0);
static std::atomic<bool> g_api_async_retry_server_header_complete(false);
static std::atomic<int> g_api_async_retry_server_last_recv_result(0);
static std::atomic<int> g_api_async_retry_server_last_errno(0);
static std::atomic<bool> g_api_request_success_server_ready(false);
static std::atomic<int> g_api_request_success_server_start_error(FT_ERR_SUCCESSS);
static std::atomic<bool> g_api_request_stream_large_server_ready(false);
static std::atomic<int> g_api_request_stream_large_server_start_error(FT_ERR_SUCCESSS);
static std::atomic<bool> g_api_request_stream_chunked_server_ready(false);
static std::atomic<int> g_api_request_stream_chunked_server_start_error(FT_ERR_SUCCESSS);

static void api_request_log_async_transfer_stats(void)
{
    size_t request_size;
    size_t bytes_sent;
    size_t bytes_received;
    int send_state;
    int send_timeout;
    int receive_state;
    int receive_timeout;
    size_t server_bytes_received;
    bool server_header_complete;
    const char *server_header_status;
    int server_last_recv_result;
    int server_last_errno;

    request_size = api_debug_get_last_async_request_size();
    bytes_sent = api_debug_get_last_async_bytes_sent();
    bytes_received = api_debug_get_last_async_bytes_received();
    send_state = api_debug_get_last_async_send_state();
    send_timeout = api_debug_get_last_async_send_timeout();
    receive_state = api_debug_get_last_async_receive_state();
    receive_timeout = api_debug_get_last_async_receive_timeout();
    server_bytes_received = g_api_async_retry_server_bytes_received.load();
    server_header_complete = g_api_async_retry_server_header_complete.load();
    server_last_recv_result = g_api_async_retry_server_last_recv_result.load();
    server_last_errno = g_api_async_retry_server_last_errno.load();
    server_header_status = "incomplete";
    if (server_header_complete)
        server_header_status = "complete";
    pf_printf("[test639-debug] request_size=%zu bytes_sent=%zu send_state=%d send_timeout=%d bytes_received=%zu receive_state=%d receive_timeout=%d ft_errno=%d server_bytes=%zu server_header=%s server_recv_result=%d server_errno=%d\n",
              request_size, bytes_sent, send_state, send_timeout, bytes_received, receive_state, receive_timeout, ft_errno, server_bytes_received,
              server_header_status, server_last_recv_result, server_last_errno);
    return ;
}

static void api_request_send_failure_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54337;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd >= 0)
        nw_close(client_fd);
    return ;
}

static void api_request_small_delay(void)
{
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    return ;
}

static void api_request_success_server_reset_state(void)
{
    g_api_request_success_server_ready.store(false, std::memory_order_relaxed);
    g_api_request_success_server_start_error.store(FT_ERR_SUCCESSS, std::memory_order_relaxed);
    return ;
}

static void api_request_success_server_signal_ready(int error_code)
{
    g_api_request_success_server_start_error.store(error_code, std::memory_order_relaxed);
    g_api_request_success_server_ready.store(true, std::memory_order_release);
    return ;
}

static void api_request_stream_large_server_reset_state(void)
{
    g_api_request_stream_large_server_ready.store(false, std::memory_order_relaxed);
    g_api_request_stream_large_server_start_error.store(FT_ERR_SUCCESSS, std::memory_order_relaxed);
    return ;
}

static void api_request_stream_large_server_signal_ready(int error_code)
{
    g_api_request_stream_large_server_start_error.store(error_code, std::memory_order_relaxed);
    g_api_request_stream_large_server_ready.store(true, std::memory_order_release);
    return ;
}

static bool api_request_stream_large_server_wait_until_ready(void)
{
    size_t wait_iterations;

    wait_iterations = 0;
    while (!g_api_request_stream_large_server_ready.load(std::memory_order_acquire))
    {
        if (wait_iterations >= 50)
            return (false);
        api_request_small_delay();
        wait_iterations += 1;
    }
    if (g_api_request_stream_large_server_start_error.load(std::memory_order_acquire) != FT_ERR_SUCCESSS)
        return (false);
    return (true);
}

static void api_request_stream_chunked_server_reset_state(void)
{
    g_api_request_stream_chunked_server_ready.store(false, std::memory_order_relaxed);
    g_api_request_stream_chunked_server_start_error.store(FT_ERR_SUCCESSS, std::memory_order_relaxed);
    return ;
}

static void api_request_stream_chunked_server_signal_ready(int error_code)
{
    g_api_request_stream_chunked_server_start_error.store(error_code, std::memory_order_relaxed);
    g_api_request_stream_chunked_server_ready.store(true, std::memory_order_release);
    return ;
}

static bool api_request_stream_chunked_server_wait_until_ready(void)
{
    size_t wait_iterations;

    wait_iterations = 0;
    while (!g_api_request_stream_chunked_server_ready.load(std::memory_order_acquire))
    {
        if (wait_iterations >= 50)
            return (false);
        api_request_small_delay();
        wait_iterations += 1;
    }
    if (g_api_request_stream_chunked_server_start_error.load(std::memory_order_acquire) != FT_ERR_SUCCESSS)
        return (false);
    return (true);
}

static void api_request_bearer_server(api_request_bearer_server_context *context)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    char buffer[512];
    ssize_t bytes_received;
    const char *header_location;
    const char *response;
    ssize_t send_result;

    if (!context)
        return ;
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54365;
    context->client_fd = -1;
    context->request_data.clear();
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        context->result.store(server_socket.get_error(), std::memory_order_relaxed);
        context->ready.store(true, std::memory_order_release);
        return ;
    }
    context->result.store(0, std::memory_order_relaxed);
    context->ready.store(true, std::memory_order_release);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
    {
        context->result.store(-1, std::memory_order_relaxed);
        return ;
    }
    context->client_fd = client_fd;
    while (1)
    {
        bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            break;
        buffer[bytes_received] = '\0';
        context->request_data.append(buffer);
        if (ft_strstr(context->request_data.c_str(), "\r\n\r\n"))
            break;
    }
    header_location = ft_strstr(context->request_data.c_str(), "Authorization: Bearer test-token");
    if (header_location)
        context->header_received.store(true, std::memory_order_relaxed);
    response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send_result = nw_send(client_fd, response, ft_strlen(response), 0);
    if (send_result < 0)
        context->result.store(-2, std::memory_order_relaxed);
    nw_close(client_fd);
    return ;
}

static void api_request_basic_server(api_request_basic_server_context *context)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    char buffer[512];
    ssize_t bytes_received;
    const char *request_cstring;
    const char *custom_header_location;
    const char *basic_header_location;
    const char *response;
    ssize_t send_result;

    if (!context)
        return ;
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54366;
    context->client_fd = -1;
    context->request_data.clear();
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        context->result.store(server_socket.get_error(), std::memory_order_relaxed);
        context->ready.store(true, std::memory_order_release);
        return ;
    }
    context->result.store(0, std::memory_order_relaxed);
    context->ready.store(true, std::memory_order_release);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd < 0)
    {
        context->result.store(-1, std::memory_order_relaxed);
        return ;
    }
    context->client_fd = client_fd;
    while (1)
    {
        bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            break;
        buffer[bytes_received] = '\0';
        context->request_data.append(buffer);
        if (ft_strstr(context->request_data.c_str(), "\r\n\r\n"))
            break;
    }
    request_cstring = context->request_data.c_str();
    basic_header_location = ft_strstr(request_cstring, "Authorization: Basic dXNlcjpwYXNz");
    if (basic_header_location)
        context->basic_header_received.store(true, std::memory_order_relaxed);
    custom_header_location = ft_strstr(request_cstring, "X-Test-Header: Value");
    if (custom_header_location && basic_header_location && custom_header_location < basic_header_location)
        context->basic_header_after_custom.store(true, std::memory_order_relaxed);
    response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
    send_result = nw_send(client_fd, response, ft_strlen(response), 0);
    if (send_result < 0)
        context->result.store(-2, std::memory_order_relaxed);
    nw_close(client_fd);
    return ;
}

static bool api_request_success_server_wait_until_ready(void)
{
    while (!g_api_request_success_server_ready.load(std::memory_order_acquire))
        api_request_small_delay();
    if (g_api_request_success_server_start_error.load(std::memory_order_acquire) != FT_ERR_SUCCESSS)
        return (false);
    return (true);
}

static void api_request_success_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *response;
    size_t response_length;
    size_t total_sent;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54338;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        api_request_success_server_signal_ready(server_socket.get_error());
        return ;
    }
    api_request_success_server_signal_ready(FT_ERR_SUCCESSS);
    response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
    response_length = ft_strlen(response);
    int remaining_connections;

    remaining_connections = 2;
    while (remaining_connections > 0)
    {
        ssize_t bytes_sent;

        address_length = sizeof(address_storage);
        client_fd = nw_accept(server_socket.get_fd(),
                              reinterpret_cast<struct sockaddr*>(&address_storage),
                              &address_length);
        if (client_fd < 0)
            break;
        total_sent = 0;
        while (total_sent < response_length)
        {
            bytes_sent = nw_send(client_fd, response + total_sent,
                                 response_length - total_sent, 0);
            if (bytes_sent <= 0)
                break;
            total_sent += static_cast<size_t>(bytes_sent);
        }
        nw_close(client_fd);
        remaining_connections -= 1;
    }
    return ;
}

static void api_request_stream_large_response_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    size_t body_size;
    char header_buffer[128];
    char *body_buffer;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54358;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        api_request_stream_large_server_signal_ready(server_socket.get_error());
        return ;
    }
    api_request_stream_large_server_signal_ready(FT_ERR_SUCCESSS);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
    {
        g_api_request_stream_large_server_start_error.store(FT_ERR_SOCKET_ACCEPT_FAILED, std::memory_order_relaxed);
        return ;
    }
    char drain_buffer[512];
    const char *header_terminator;
    size_t header_match_index;
    bool header_complete;

    header_terminator = "\r\n\r\n";
    header_match_index = 0;
    header_complete = false;
    while (header_complete == false)
    {
        ssize_t bytes_received;
        size_t byte_index;

        bytes_received = nw_recv(client_fd, drain_buffer, sizeof(drain_buffer), 0);
        if (bytes_received <= 0)
            break ;
        byte_index = 0;
        while (byte_index < static_cast<size_t>(bytes_received)
            && header_complete == false)
        {
            if (drain_buffer[byte_index] == header_terminator[header_match_index])
            {
                header_match_index += 1;
                if (header_match_index == 4)
                    header_complete = true;
            }
            else if (drain_buffer[byte_index] == header_terminator[0])
            {
                header_match_index = 1;
            }
            else
            {
                header_match_index = 0;
            }
            byte_index += 1;
        }
        if (header_complete == true)
            break ;
    }
    body_size = 2 * 1024 * 1024;
    body_buffer = static_cast<char*>(cma_malloc(body_size));
    if (!body_buffer)
    {
        nw_close(client_fd);
        return ;
    }
    ft_memset(body_buffer, 'A', body_size);
    pf_snprintf(header_buffer, sizeof(header_buffer),
        "HTTP/1.1 200 OK\r\nContent-Length: %zu\r\n\r\n", body_size);
    nw_send(client_fd, header_buffer, ft_strlen(header_buffer), 0);
    size_t total_sent;

    total_sent = 0;
    while (total_sent < body_size)
    {
        size_t remaining;
        size_t chunk_size;
        ssize_t bytes_sent;

        remaining = body_size - total_sent;
        chunk_size = 32768;
        if (chunk_size > remaining)
            chunk_size = remaining;
        bytes_sent = nw_send(client_fd, body_buffer + total_sent,
                chunk_size, 0);
        if (bytes_sent <= 0)
            break ;
        total_sent += static_cast<size_t>(bytes_sent);
    }
    cma_free(body_buffer);
    nw_close(client_fd);
    return ;
}

static void api_request_retry_success_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    int accepted_count;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54339;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
        return ;
    accepted_count = 0;
    while (accepted_count < 2)
    {
        address_length = sizeof(address_storage);
        client_fd = nw_accept(server_socket.get_fd(),
                reinterpret_cast<struct sockaddr*>(&address_storage),
                &address_length);
        if (client_fd < 0)
            continue ;
        accepted_count++;
        if (accepted_count == 1)
        {
            nw_close(client_fd);
            continue ;
        }
        const char *response;
        size_t response_length;
        size_t total_sent;

        response = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
        response_length = ft_strlen(response);
        total_sent = 0;
        while (total_sent < response_length)
        {
            ssize_t bytes_sent;

            bytes_sent = nw_send(client_fd, response + total_sent,
                    response_length - total_sent, 0);
            if (bytes_sent <= 0)
                break ;
            total_sent += static_cast<size_t>(bytes_sent);
        }
        nw_close(client_fd);
    }
    return ;
}

static void api_request_retry_failure_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    int accepted_count;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54340;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
        return ;
    accepted_count = 0;
    while (accepted_count < 2)
    {
        address_length = sizeof(address_storage);
        client_fd = nw_accept(server_socket.get_fd(),
                reinterpret_cast<struct sockaddr*>(&address_storage),
                &address_length);
        if (client_fd < 0)
            continue ;
        accepted_count++;
        nw_close(client_fd);
    }
    return ;
}

static void api_request_circuit_success_server(
    api_request_circuit_server_context *context)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int served_count;

    if (!context)
        return ;
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = context->port;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        context->ready.store(true, std::memory_order_release);
        return ;
    }
    served_count = 0;
    context->ready.store(true, std::memory_order_release);
    while (served_count < context->responses)
    {
        int client_fd;

        address_length = sizeof(address_storage);
        client_fd = nw_accept(server_socket.get_fd(),
                reinterpret_cast<struct sockaddr*>(&address_storage),
                &address_length);
        if (client_fd < 0)
            continue ;
        const char *response;
        size_t response_length;
        size_t total_sent;

        response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
        response_length = ft_strlen(response);
        total_sent = 0;
        while (total_sent < response_length)
        {
            ssize_t bytes_sent;

            bytes_sent = nw_send(client_fd, response + total_sent,
                    response_length - total_sent, 0);
            if (bytes_sent <= 0)
                break ;
            total_sent += static_cast<size_t>(bytes_sent);
        }
        nw_close(client_fd);
        served_count++;
    }
    return ;
}

static void api_request_retry_timeout_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    int accepted_count;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54341;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
        return ;
    accepted_count = 0;
    while (accepted_count < 3)
    {
        address_length = sizeof(address_storage);
        client_fd = nw_accept(server_socket.get_fd(),
                reinterpret_cast<struct sockaddr*>(&address_storage),
                &address_length);
        if (client_fd < 0)
            continue ;
        accepted_count++;
#ifdef _WIN32
        Sleep(200);
#else
        usleep(200000);
#endif
        nw_close(client_fd);
    }
    return ;
}

struct api_async_retry_callback_data
{
    std::atomic<bool> *completed;
    std::atomic<int> *status;
    std::atomic<char*> *body;
};

static char *api_request_build_large_header(void)
{
    const char *header_prefix;
    size_t header_prefix_length;
    size_t filler_length;
    size_t total_length;
    char *header_buffer;

    header_prefix = "X-Test: ";
    header_prefix_length = ft_strlen(header_prefix);
    filler_length = 4 * 1024 * 1024;
    total_length = header_prefix_length + filler_length + 2;
    header_buffer = static_cast<char*>(cma_malloc(total_length + 1));
    if (!header_buffer)
        return (ft_nullptr);
    ft_memcpy(header_buffer, header_prefix, header_prefix_length);
    ft_memset(header_buffer + header_prefix_length, 'A', filler_length);
    header_buffer[header_prefix_length + filler_length] = '\r';
    header_buffer[header_prefix_length + filler_length + 1] = '\n';
    header_buffer[header_prefix_length + filler_length + 2] = '\0';
    return (header_buffer);
}

static void api_request_async_retry_callback(char *body, int status, void *user_data)
{
    api_async_retry_callback_data *data;

    data = static_cast<api_async_retry_callback_data*>(user_data);
    if (!data || !data->completed || !data->status || !data->body)
    {
        if (body)
            cma_free(body);
        return ;
    }
    data->status->store(status);
    data->body->store(body);
    data->completed->store(true);
    return ;
}

static void api_request_stream_chunked_response_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *response;
    size_t response_length;
    size_t total_sent;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54359;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
    {
        api_request_stream_chunked_server_signal_ready(server_socket.get_error());
        return ;
    }
    api_request_stream_chunked_server_signal_ready(FT_ERR_SUCCESSS);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
    {
        g_api_request_stream_chunked_server_start_error.store(FT_ERR_SOCKET_ACCEPT_FAILED, std::memory_order_relaxed);
        return ;
    }
    response = "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n"
        "10\r\n0123456789ABCDEF\r\n"
        "08\r\nABCDEFGH\r\n"
        "0\r\n\r\n";
    response_length = ft_strlen(response);
    total_sent = 0;
    while (total_sent < response_length)
    {
        ssize_t bytes_sent;

        bytes_sent = nw_send(client_fd, response + total_sent,
                response_length - total_sent, 0);
        if (bytes_sent <= 0)
            break ;
        total_sent += static_cast<size_t>(bytes_sent);
    }
    nw_close(client_fd);
    return ;
}

static void api_request_async_retry_server(void)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    const char *response;
    char buffer[4096];
    size_t total_bytes_received;
    bool header_complete;
    int last_recv_result;
    int last_recv_errno;

    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = 54339;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != FT_ERR_SUCCESSS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
                          reinterpret_cast<struct sockaddr*>(&address_storage),
                          &address_length);
    if (client_fd < 0)
        return ;
    g_api_async_retry_server_bytes_received.store(0);
    g_api_async_retry_server_header_complete.store(false);
    g_api_async_retry_server_last_recv_result.store(0);
    g_api_async_retry_server_last_errno.store(0);
    api_request_small_delay();
    api_request_small_delay();
    total_bytes_received = 0;
    header_complete = false;
    last_recv_result = 0;
    last_recv_errno = 0;
    t_monotonic_time_point receive_start;
    t_monotonic_time_point receive_end;
    int terminator_state;

    receive_start = time_monotonic_point_now();
    terminator_state = 0;
    while (true)
    {
        ssize_t bytes_received;

        bytes_received = nw_recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            last_recv_result = static_cast<int>(bytes_received);
            last_recv_errno = errno;
            break;
        }
        total_bytes_received += static_cast<size_t>(bytes_received);
        buffer[bytes_received] = '\0';
        size_t buffer_index;

        buffer_index = 0;
        while (buffer_index < static_cast<size_t>(bytes_received))
        {
            char current_char;

            current_char = buffer[buffer_index];
            if ((terminator_state == 0 || terminator_state == 2) && current_char == '\r')
                terminator_state += 1;
            else if ((terminator_state == 1 || terminator_state == 3) && current_char == '\n')
            {
                terminator_state += 1;
                if (terminator_state == 4)
                {
                    header_complete = true;
                    break;
                }
            }
            else if (current_char == '\r')
                terminator_state = 1;
            else
                terminator_state = 0;
            buffer_index += 1;
        }
        if (header_complete)
            break;
    }
    receive_end = time_monotonic_point_now();
    g_api_async_retry_server_bytes_received.store(total_bytes_received);
    g_api_async_retry_server_header_complete.store(header_complete);
    g_api_async_retry_server_last_recv_result.store(last_recv_result);
    g_api_async_retry_server_last_errno.store(last_recv_errno);
    long long receive_duration_ms = time_monotonic_point_diff_ms(receive_start, receive_end);
    (void)receive_duration_ms;
    response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
    nw_send(client_fd, response, ft_strlen(response), 0);
    nw_close(client_fd);
    return ;
}

FT_TEST(test_api_request_invalid_ip_sets_socket_error, "api_request_string invalid ip sets socket error")
{
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string("bad-ip", 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 10);
    if (result != ft_nullptr)
        return (0);
    bool configuration_error;

    configuration_error = false;
    if (ft_errno == FT_ERR_CONFIGURATION)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_FAILED)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_AGAIN)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_FAIL)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_BAD_FLAGS)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_FAMILY)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_SOCKTYPE)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_SERVICE)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_MEMORY)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_NO_NAME)
        configuration_error = true;
    if (ft_errno == FT_ERR_SOCKET_RESOLVE_OVERFLOW)
        configuration_error = true;
    if (!configuration_error && ft_errno != FT_ERR_SOCKET_CONNECT_FAILED)
        return (0);
    return (1);
}

FT_TEST(test_api_request_connect_failure_sets_errno, "api_request_string_host connect failure sets errno")
{
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string_host("127.0.0.1", 59999, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 50);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_SOCKET_CONNECT_FAILED)
        return (0);
    return (1);
}

FT_TEST(test_api_request_send_failure_sets_errno, "api_request_string send failure sets errno")
{
    char *result;
    ft_thread server_thread;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = FT_ERR_SUCCESSS;
    server_thread = ft_thread(api_request_send_failure_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    api_request_small_delay();
    result = api_request_string("127.0.0.1", 54337, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    int request_errno = ft_errno;
    server_thread.join();
    if (result != ft_nullptr)
        return (0);
    if (request_errno != FT_ERR_SOCKET_SEND_FAILED && request_errno != FT_ERR_IO)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    ft_errno = request_errno;
    return (1);
}

FT_TEST(test_api_request_async_alloc_failure_sets_errno, "api_request_string_async allocation failure sets errno")
{
    bool result;

    ft_errno = FT_ERR_SUCCESSS;
    cma_set_alloc_limit(1);
    result = api_request_string_async("127.0.0.1", 8080, "GET", "/", api_request_noop_callback,
                                      ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    cma_set_alloc_limit(0);
    if (result)
        return (0);
    if (ft_errno != FT_ERR_NO_MEMORY)
        return (0);
    return (1);
}

FT_TEST(test_api_request_bad_input_sets_errno, "api_request_string_host bad input sets errno")
{
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string_host(ft_nullptr, 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 10);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_api_request_success_resets_errno, "api_request_string success resets errno")
{
    char *body;
    ft_thread server_thread;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = FT_ERR_NO_MEMORY;
    api_request_success_server_reset_state();
    server_thread = ft_thread(api_request_success_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!api_request_success_server_wait_until_ready())
    {
        server_thread.join();
        return (0);
    }
    body = api_request_string("127.0.0.1", 54338, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    server_thread.join();
    if (body == ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    cma_free(body);
    return (1);
}

FT_TEST(test_api_request_host_bearer_adds_header,
    "api_request_string_host_bearer adds bearer header")
{
    ft_thread server_thread;
    api_request_bearer_server_context context;
    char *body;
    int status_code;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    context.ready.store(false, std::memory_order_relaxed);
    context.header_received.store(false, std::memory_order_relaxed);
    context.result.store(-99, std::memory_order_relaxed);
    context.client_fd = -1;
    context.request_data.clear();
    ft_errno = FT_ERR_SUCCESSS;
    server_thread = ft_thread(api_request_bearer_server, &context);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    while (!context.ready.load(std::memory_order_acquire))
        api_request_small_delay();
    status_code = 0;
    body = api_request_string_host_bearer("127.0.0.1", 54365, "GET", "/",
                                          "test-token", ft_nullptr, ft_nullptr,
                                          &status_code, 1000, ft_nullptr);
    server_thread.join();
    if (body)
        cma_free(body);
    if (context.result.load(std::memory_order_relaxed) != 0)
        return (0);
    if (!context.header_received.load(std::memory_order_relaxed))
        return (0);
    if (status_code != 200)
        return (0);
    return (1);
}

FT_TEST(test_api_request_host_basic_appends_after_existing_header,
    "api_request_string_host_basic adds basic header after existing header")
{
    ft_thread server_thread;
    api_request_basic_server_context context;
    char *body;
    int status_code;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    context.ready.store(false, std::memory_order_relaxed);
    context.basic_header_received.store(false, std::memory_order_relaxed);
    context.basic_header_after_custom.store(false, std::memory_order_relaxed);
    context.result.store(-99, std::memory_order_relaxed);
    context.client_fd = -1;
    context.request_data.clear();
    ft_errno = FT_ERR_SUCCESSS;
    server_thread = ft_thread(api_request_basic_server, &context);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    while (!context.ready.load(std::memory_order_acquire))
        api_request_small_delay();
    status_code = 0;
    body = api_request_string_host_basic("127.0.0.1", 54366, "GET", "/",
                                         "dXNlcjpwYXNz", ft_nullptr,
                                         "X-Test-Header: Value",
                                         &status_code, 1000, ft_nullptr);
    server_thread.join();
    if (body)
        cma_free(body);
    if (context.result.load(std::memory_order_relaxed) != 0)
        return (0);
    if (!context.basic_header_received.load(std::memory_order_relaxed))
        return (0);
    if (!context.basic_header_after_custom.load(std::memory_order_relaxed))
        return (0);
    if (status_code != 200)
        return (0);
    return (1);
}

FT_TEST(test_api_request_stream_large_response,
    "api_request_stream streams large response")
{
    ft_thread server_thread;
    api_stream_test_context context;
    api_streaming_handler handler;
    bool result;
    size_t expected_size;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = FT_ERR_SUCCESSS;
    context.total_bytes = 0;
    context.chunk_count = 0;
    context.headers_received = false;
    context.final_received = false;
    context.status_code = 0;
    handler.set_headers_callback(api_request_stream_headers_callback);
    handler.set_body_callback(api_request_stream_body_callback);
    handler.set_user_data(&context);
    api_request_stream_large_server_reset_state();
    server_thread = ft_thread(api_request_stream_large_response_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!api_request_stream_large_server_wait_until_ready())
    {
        server_thread.join();
        return (0);
    }
    expected_size = 2 * 1024 * 1024;
    result = api_request_stream("127.0.0.1", 54358, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 5000, ft_nullptr);
    server_thread.join();
    if (!result)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    if (!context.headers_received)
        return (0);
    if (context.status_code != 200)
        return (0);
    if (!context.final_received)
        return (0);
    if (context.total_bytes != expected_size)
        return (0);
    if (context.chunk_count == 0)
        return (0);
    return (1);
}

FT_TEST(test_api_request_stream_chunked_response,
    "api_request_stream handles chunked encoding")
{
    ft_thread server_thread;
    api_stream_test_context context;
    api_streaming_handler handler;
    bool result;
    size_t expected_size;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    ft_errno = FT_ERR_SUCCESSS;
    context.total_bytes = 0;
    context.chunk_count = 0;
    context.headers_received = false;
    context.final_received = false;
    context.status_code = 0;
    handler.set_headers_callback(api_request_stream_headers_callback);
    handler.set_body_callback(api_request_stream_body_callback);
    handler.set_user_data(&context);
    api_request_stream_chunked_server_reset_state();
    server_thread = ft_thread(api_request_stream_chunked_response_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!api_request_stream_chunked_server_wait_until_ready())
    {
        server_thread.join();
        return (0);
    }
    expected_size = 24;
    result = api_request_stream("127.0.0.1", 54359, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 2000, ft_nullptr);
    server_thread.join();
    if (!result)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    if (!context.headers_received)
        return (0);
    if (context.status_code != 200)
        return (0);
    if (!context.final_received)
        return (0);
    if (context.total_bytes != expected_size)
        return (0);
    if (context.chunk_count < 2)
        return (0);
    return (1);
}

FT_TEST(test_api_request_async_large_send_retries_do_not_timeout, "api_request_string_async large send retries complete")
{
    std::atomic<bool> callback_completed(false);
    std::atomic<int> callback_status(0);
    std::atomic<char*> callback_body(ft_nullptr);
    api_async_retry_callback_data callback_data;
    ft_thread server_thread;
    char *headers;
    bool async_result;
    size_t wait_iterations;
    char *body_result;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    headers = api_request_build_large_header();
    if (!headers)
        return (0);
    server_thread = ft_thread(api_request_async_retry_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
    {
        cma_free(headers);
        return (0);
    }
    api_request_small_delay();
    callback_data.completed = &callback_completed;
    callback_data.status = &callback_status;
    callback_data.body = &callback_body;
    async_result = api_request_string_async("127.0.0.1", 54339, "POST", "/", api_request_async_retry_callback,
                                            &callback_data, ft_nullptr, headers, 1000);
    cma_free(headers);
    if (!async_result)
    {
        api_request_log_async_transfer_stats();
        server_thread.join();
        return (0);
    }
    wait_iterations = 0;
    while (!callback_completed.load() && wait_iterations < 20)
    {
        api_request_small_delay();
        wait_iterations += 1;
    }
    if (!callback_completed.load())
    {
        api_request_log_async_transfer_stats();
        server_thread.join();
        return (0);
    }
    server_thread.join();
    api_request_log_async_transfer_stats();
    if (callback_status.load() != 200)
    {
        api_request_log_async_transfer_stats();
        return (0);
    }
    body_result = callback_body.load();
    if (!body_result)
    {
        api_request_log_async_transfer_stats();
        return (0);
    }
    if (ft_strncmp(body_result, "OK", 2) != 0)
    {
        cma_free(body_result);
        api_request_log_async_transfer_stats();
        return (0);
    }
    cma_free(body_result);
    return (1);
}

FT_TEST(test_api_request_string_url_invalid_sets_errno, "api_request_string_url invalid input sets errno")
{
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string_url("example.com/path", "GET", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_api_request_tls_missing_host_sets_errno, "api_request_string_tls missing host sets errno")
{
    char *result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string_tls(ft_nullptr, 443, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_api_request_async_missing_callback_sets_errno, "api_request_string_async missing callback sets errno")
{
    bool result;

    ft_errno = FT_ERR_SUCCESSS;
    result = api_request_string_async("127.0.0.1", 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    if (result)
        return (0);
    if (ft_errno != FT_ERR_INVALID_ARGUMENT)
        return (0);
    return (1);
}

FT_TEST(test_api_request_async_success_resets_errno, "api_request_string_async success resets errno")
{
    bool result;

    ft_errno = FT_ERR_NO_MEMORY;
    result = api_request_string_async("127.0.0.1", 59999, "GET", "/", api_request_noop_callback,
                                      ft_nullptr, ft_nullptr, ft_nullptr, 100);
    if (!result)
        return (0);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

FT_TEST(test_api_request_formats_large_content_length, "api_request formats content length for large payloads")
{
    ft_string request;
    size_t payload_size;
    bool append_result;
    char expected_buffer[64];
    int expected_length;
    ft_string expected_header;
    const char *header_pointer;

    payload_size = static_cast<size_t>(static_cast<unsigned long long>(INT_MAX)) + 42;
    request = "POST /resource HTTP/1.1";
    append_result = api_append_content_length_header(request, payload_size);
    if (!append_result)
        return (0);
    expected_length = pf_snprintf(expected_buffer, sizeof(expected_buffer), "%zu", payload_size);
    if (expected_length < 0)
        return (0);
    if (static_cast<size_t>(expected_length) >= sizeof(expected_buffer))
        return (0);
    expected_header = "\r\nContent-Length: ";
    expected_header += expected_buffer;
    header_pointer = ft_strstr(request.c_str(), expected_header.c_str());
    if (header_pointer == ft_nullptr)
        return (0);
    return (1);
}

FT_TEST(test_http2_frame_roundtrip, "http2 frame encode decode roundtrip")
{
    http2_frame input_frame;
    ft_string encoded;
    http2_frame decoded_frame;
    size_t offset;
    int error_code;

    if (!input_frame.set_type(0x1))
        return (0);
    if (!input_frame.set_flags(0x5))
        return (0);
    if (!input_frame.set_stream_identifier(3))
        return (0);
    if (!input_frame.set_payload_from_buffer("Hello", 5))
        return (0);
    error_code = FT_ERR_SUCCESSS;
    if (!http2_encode_frame(input_frame, encoded, error_code))
        return (0);
    if (error_code != FT_ERR_SUCCESSS)
        return (0);
    offset = 0;
    if (!http2_decode_frame(reinterpret_cast<const unsigned char*>(encoded.c_str()),
            encoded.size(), offset, decoded_frame, error_code))
        return (0);
    if (error_code != FT_ERR_SUCCESSS)
        return (0);
    uint8_t decoded_type;
    uint8_t input_type;
    uint8_t decoded_flags;
    uint8_t input_flags;
    uint32_t decoded_stream_identifier;
    uint32_t input_stream_identifier;
    ft_string decoded_payload;
    ft_string input_payload;

    if (!decoded_frame.get_type(decoded_type))
        return (0);
    if (!input_frame.get_type(input_type))
        return (0);
    if (decoded_type != input_type)
        return (0);
    if (!decoded_frame.get_flags(decoded_flags))
        return (0);
    if (!input_frame.get_flags(input_flags))
        return (0);
    if (decoded_flags != input_flags)
        return (0);
    if (!decoded_frame.get_stream_identifier(decoded_stream_identifier))
        return (0);
    if (!input_frame.get_stream_identifier(input_stream_identifier))
        return (0);
    if (decoded_stream_identifier != input_stream_identifier)
        return (0);
    if (!decoded_frame.copy_payload(decoded_payload))
        return (0);
    if (!input_frame.copy_payload(input_payload))
        return (0);
    if (!(decoded_payload == input_payload))
        return (0);
    return (1);
}

FT_TEST(test_http2_header_compression_roundtrip, "http2 header compression roundtrip")
{
    ft_vector<http2_header_field> headers;
    ft_vector<http2_header_field> decoded_headers;
    http2_header_field field_entry;
    ft_string compressed;
    int error_code;
    size_t header_count;
    size_t index;

    if (!field_entry.assign_from_cstr(":method", "GET"))
        return (0);
    headers.push_back(field_entry);
    if (headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!field_entry.assign_from_cstr(":path", "/resource"))
        return (0);
    headers.push_back(field_entry);
    if (headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!field_entry.assign_from_cstr("user-agent", "libft-tests"))
        return (0);
    headers.push_back(field_entry);
    if (headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!field_entry.assign_from_cstr("accept", "*/*"))
        return (0);
    headers.push_back(field_entry);
    if (headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    error_code = FT_ERR_SUCCESSS;
    if (!http2_compress_headers(headers, compressed, error_code))
        return (0);
    if (error_code != FT_ERR_SUCCESSS)
        return (0);
    if (!http2_decompress_headers(compressed, decoded_headers, error_code))
        return (0);
    if (error_code != FT_ERR_SUCCESSS)
        return (0);
    header_count = decoded_headers.size();
    if (decoded_headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (headers.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (header_count != headers.size())
        return (0);
    index = 0;
    while (index < header_count)
    {
        ft_string decoded_name;
        ft_string decoded_value;
        ft_string original_name;
        ft_string original_value;

        if (!decoded_headers[index].copy_name(decoded_name))
            return (0);
        if (!headers[index].copy_name(original_name))
            return (0);
        if (!(decoded_name == original_name))
            return (0);
        if (!decoded_headers[index].copy_value(decoded_value))
            return (0);
        if (!headers[index].copy_value(original_value))
            return (0);
        if (!(decoded_value == original_value))
            return (0);
        index++;
    }
    return (1);
}

FT_TEST(test_http2_stream_manager_concurrent_streams, "http2 stream manager tracks streams")
{
    http2_stream_manager manager;
    ft_string buffer;

    if (!manager.open_stream(1))
        return (0);
    if (!manager.open_stream(3))
        return (0);
    if (!manager.append_data(1, "Ping", 4))
        return (0);
    if (!manager.append_data(3, "Pong", 4))
        return (0);
    if (!manager.get_stream_buffer(1, buffer))
        return (0);
    if (!(buffer == "Ping"))
        return (0);
    if (!manager.get_stream_buffer(3, buffer))
        return (0);
    if (!(buffer == "Pong"))
        return (0);
    if (!manager.close_stream(1))
        return (0);
    if (!manager.close_stream(3))
        return (0);
    return (1);
}

FT_TEST(test_http2_stream_manager_flow_control, "http2 stream manager enforces flow control windows")
{
    http2_stream_manager manager;
    ft_string buffer;
    uint32_t window_value;

    if (!manager.update_local_initial_window(8))
        return (0);
    if (!manager.update_remote_initial_window(12))
        return (0);
    if (!manager.open_stream(1))
        return (0);
    if (manager.append_data(1, "0123456789", 10))
        return (0);
    if (manager.get_error() != FT_ERR_OUT_OF_RANGE)
        return (0);
    if (!manager.append_data(1, "ABCD", 4))
        return (0);
    window_value = manager.get_local_window(1);
    if (manager.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (window_value != 4)
        return (0);
    if (manager.reserve_send_window(1, 16))
        return (0);
    if (manager.get_error() != FT_ERR_OUT_OF_RANGE)
        return (0);
    if (!manager.reserve_send_window(1, 6))
        return (0);
    window_value = manager.get_remote_window(1);
    if (manager.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (window_value != 6)
        return (0);
    if (!manager.append_data(1, "EFGH", 4))
        return (0);
    if (!manager.get_stream_buffer(1, buffer))
        return (0);
    if (!(buffer == "ABCDEFGH"))
        return (0);
    if (!manager.close_stream(1))
        return (0);
    return (1);
}

FT_TEST(test_http2_stream_manager_priority_reassignment, "http2 priority exclusive flag reassigns children")
{
    http2_stream_manager manager;
    uint32_t dependency_identifier;
    uint8_t weight_value;
    bool exclusive_flag;

    if (!manager.open_stream(1))
        return (0);
    if (!manager.open_stream(3))
        return (0);
    if (!manager.open_stream(5))
        return (0);
    if (!manager.update_priority(3, 0, 20, false))
        return (0);
    if (!manager.update_priority(5, 0, 10, false))
        return (0);
    if (!manager.update_priority(3, 0, 25, true))
        return (0);
    if (!manager.get_priority(5, dependency_identifier, weight_value, exclusive_flag))
        return (0);
    if (dependency_identifier != 3)
        return (0);
    if (!manager.get_priority(3, dependency_identifier, weight_value, exclusive_flag))
        return (0);
    if (dependency_identifier != 0)
        return (0);
    if (!manager.close_stream(5))
        return (0);
    if (!manager.close_stream(3))
        return (0);
    if (!manager.close_stream(1))
        return (0);
    return (1);
}

FT_TEST(test_http2_settings_apply_remote_settings, "http2 settings adjusts remote initial window")
{
    http2_stream_manager manager;
    http2_settings_state settings;
    http2_frame frame;
    char payload_bytes[6];
    uint32_t window_value;

    if (!manager.open_stream(1))
        return (0);
    if (!frame.set_type(0x4))
        return (0);
    if (!frame.set_flags(0x0))
        return (0);
    if (!frame.set_stream_identifier(0))
        return (0);
    payload_bytes[0] = 0x00;
    payload_bytes[1] = 0x04;
    payload_bytes[2] = 0x00;
    payload_bytes[3] = 0x00;
    payload_bytes[4] = 0x04;
    payload_bytes[5] = 0x00;
    if (!frame.set_payload_from_buffer(payload_bytes, 6))
        return (0);
    if (!settings.apply_remote_settings(frame, manager))
        return (0);
    window_value = manager.get_remote_window(1);
    if (manager.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (window_value != 1024)
        return (0);
    if (settings.get_initial_remote_window() != 1024)
        return (0);
    if (!manager.close_stream(1))
        return (0);
    return (1);
}

FT_TEST(test_api_request_http2_plain_fallback, "api_request_string_http2 falls back to http1")
{
    char *body;
    ft_thread server_thread;
    bool used_http2;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    api_request_success_server_reset_state();
    server_thread = ft_thread(api_request_success_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    if (!api_request_success_server_wait_until_ready())
    {
        server_thread.join();
        return (0);
    }
    api_request_set_downgrade_wait_hook(
        api_request_success_server_wait_until_ready);
    used_http2 = true;
    body = api_request_string_http2("127.0.0.1", 54338, "GET", "/", ft_nullptr,
            ft_nullptr, ft_nullptr, 1000, &used_http2);
    api_request_set_downgrade_wait_hook(ft_nullptr);
    server_thread.join();
    if (!body)
        return (0);
    if (used_http2)
    {
        cma_free(body);
        return (0);
    }
    if (ft_strcmp(body, "Hello") != 0)
    {
        cma_free(body);
        return (0);
    }
    cma_free(body);
    return (1);
}

FT_TEST(test_api_request_retry_policy_success, "api_request_string retries recoverable failures")
{
    ft_thread server_thread;
    api_retry_policy retry_policy;
    char *body;
    int status_value;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    server_thread = ft_thread(api_request_retry_success_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    api_request_small_delay();
    retry_policy.set_max_attempts(3);
    retry_policy.set_initial_delay_ms(10);
    retry_policy.set_max_delay_ms(0);
    retry_policy.set_backoff_multiplier(1);
    status_value = -123;
    body = api_request_string("127.0.0.1", 54339, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 100, &retry_policy);
    server_thread.join();
    if (!body)
        return (0);
    if (ft_strcmp(body, "Hello") != 0)
    {
        cma_free(body);
        return (0);
    }
    if (status_value != 200)
    {
        cma_free(body);
        return (0);
    }
    cma_free(body);
    if (ft_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

FT_TEST(test_api_request_retry_policy_exhaustion, "api_request_string stops after retry exhaustion")
{
    ft_thread server_thread;
    api_retry_policy retry_policy;
    char *body;
    int status_value;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    server_thread = ft_thread(api_request_retry_failure_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    api_request_small_delay();
    retry_policy.set_max_attempts(2);
    retry_policy.set_initial_delay_ms(5);
    retry_policy.set_max_delay_ms(0);
    retry_policy.set_backoff_multiplier(1);
    status_value = 777;
    body = api_request_string("127.0.0.1", 54340, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 100, &retry_policy);
    int request_errno;

    request_errno = ft_errno;
    server_thread.join();
    ft_errno = request_errno;
    if (body)
    {
        cma_free(body);
        return (0);
    }
    if (status_value != 777)
        return (0);
    if (request_errno != FT_ERR_SOCKET_RECEIVE_FAILED
        && request_errno != FT_ERR_IO
        && request_errno != FT_ERR_SOCKET_SEND_FAILED)
        return (0);
    return (1);
}

FT_TEST(test_api_request_retry_policy_timeout, "api_request_string retries until timeout exhaustion")
{
    ft_thread server_thread;
    api_retry_policy retry_policy;
    char *body;
    int status_value;

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    server_thread = ft_thread(api_request_retry_timeout_server);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    api_request_small_delay();
    retry_policy.set_max_attempts(3);
    retry_policy.set_initial_delay_ms(5);
    retry_policy.set_max_delay_ms(10);
    retry_policy.set_backoff_multiplier(2);
    status_value = -45;
    body = api_request_string("127.0.0.1", 54341, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    int request_errno;

    request_errno = ft_errno;
    server_thread.join();
    ft_errno = request_errno;
    if (body)
    {
        cma_free(body);
        return (0);
    }
    if (status_value != -45)
        return (0);
    if (request_errno != FT_ERR_SOCKET_RECEIVE_FAILED && request_errno != FT_ERR_IO)
        return (0);
    return (1);
}

FT_TEST(test_api_request_circuit_breaker_blocks_after_threshold,
    "api_request circuit breaker blocks after repeated failures")
{
    api_retry_policy retry_policy;
    char *body;
    int status_value;
    int request_errno;
    api_request_circuit_server_context server_context;
    ft_thread server_thread;
    int wait_attempts;

    api_retry_circuit_reset();
    retry_policy.set_max_attempts(1);
    retry_policy.set_initial_delay_ms(0);
    retry_policy.set_max_delay_ms(0);
    retry_policy.set_backoff_multiplier(1);
    retry_policy.set_circuit_breaker_threshold(2);
    retry_policy.set_circuit_breaker_cooldown_ms(200);
    retry_policy.set_circuit_breaker_half_open_successes(1);
    status_value = 0;
    body = api_request_string("127.0.0.1", 55310, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    if (body)
    {
        cma_free(body);
        return (0);
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55310, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    if (body)
    {
        cma_free(body);
        return (0);
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55310, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    request_errno = ft_errno;
    if (body)
    {
        cma_free(body);
        return (0);
    }
    if (request_errno != FT_ERR_API_CIRCUIT_OPEN)
        return (0);
    api_retry_circuit_reset();
    server_context.ready.store(false, std::memory_order_relaxed);
    server_context.port = 55310;
    server_context.responses = 1;
    server_thread = ft_thread(api_request_circuit_success_server,
            &server_context);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    wait_attempts = 0;
    while (!server_context.ready.load(std::memory_order_acquire)
        && wait_attempts < 100)
    {
        time_sleep_ms(10);
        wait_attempts++;
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55310, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 200, &retry_policy);
    request_errno = ft_errno;
    server_thread.join();
    ft_errno = request_errno;
    if (!body)
        return (0);
    if (ft_strcmp(body, "OK") != 0)
    {
        cma_free(body);
        return (0);
    }
    cma_free(body);
    if (status_value != 200)
        return (0);
    if (request_errno != FT_ERR_SUCCESSS)
        return (0);
    return (1);
}

FT_TEST(test_api_request_circuit_breaker_half_open_recovers,
    "api_request circuit breaker recovers after cooldown")
{
    api_retry_policy retry_policy;
    char *body;
    int status_value;
    int request_errno;
    api_request_circuit_server_context server_context;
    ft_thread server_thread;
    int wait_attempts;

    api_retry_circuit_reset();
    retry_policy.set_max_attempts(1);
    retry_policy.set_initial_delay_ms(0);
    retry_policy.set_max_delay_ms(0);
    retry_policy.set_backoff_multiplier(1);
    retry_policy.set_circuit_breaker_threshold(2);
    retry_policy.set_circuit_breaker_cooldown_ms(100);
    retry_policy.set_circuit_breaker_half_open_successes(1);
    status_value = 0;
    body = api_request_string("127.0.0.1", 55311, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    if (body)
    {
        cma_free(body);
        return (0);
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55311, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 50, &retry_policy);
    if (body)
    {
        cma_free(body);
        return (0);
    }
    time_sleep_ms(150);
    server_context.ready.store(false, std::memory_order_relaxed);
    server_context.port = 55311;
    server_context.responses = 2;
    server_thread = ft_thread(api_request_circuit_success_server,
            &server_context);
    if (server_thread.get_error() != FT_ERR_SUCCESSS)
        return (0);
    wait_attempts = 0;
    while (!server_context.ready.load(std::memory_order_acquire)
        && wait_attempts < 100)
    {
        time_sleep_ms(10);
        wait_attempts++;
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55311, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 200, &retry_policy);
    request_errno = ft_errno;
    if (!body)
    {
        server_thread.join();
        ft_errno = request_errno;
        return (0);
    }
    if (ft_strcmp(body, "OK") != 0)
    {
        cma_free(body);
        server_thread.join();
        ft_errno = request_errno;
        return (0);
    }
    cma_free(body);
    if (status_value != 200)
    {
        server_thread.join();
        ft_errno = request_errno;
        return (0);
    }
    if (request_errno != FT_ERR_SUCCESSS)
    {
        server_thread.join();
        ft_errno = request_errno;
        return (0);
    }
    status_value = 0;
    body = api_request_string("127.0.0.1", 55311, "GET", "/", ft_nullptr,
            ft_nullptr, &status_value, 200, &retry_policy);
    request_errno = ft_errno;
    server_thread.join();
    ft_errno = request_errno;
    if (!body)
        return (0);
    if (ft_strcmp(body, "OK") != 0)
    {
        cma_free(body);
        return (0);
    }
    cma_free(body);
    if (status_value != 200)
        return (0);
    if (request_errno != FT_ERR_SUCCESSS)
        return (0);
    api_retry_circuit_reset();
    return (1);
}

