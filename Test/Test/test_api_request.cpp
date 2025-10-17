#include "../../API/api.hpp"
#include "../../API/api_internal.hpp"
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

struct api_stream_test_context
{
    size_t total_bytes;
    size_t chunk_count;
    bool headers_received;
    bool final_received;
    int status_code;
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
static std::atomic<int> g_api_request_success_server_start_error(ER_SUCCESS);

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
    if (server_socket.get_error() != ER_SUCCESS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&address_storage), &address_length);
    if (client_fd >= 0)
        FT_CLOSE_SOCKET(client_fd);
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
    g_api_request_success_server_start_error.store(ER_SUCCESS, std::memory_order_relaxed);
    return ;
}

static void api_request_success_server_signal_ready(int error_code)
{
    g_api_request_success_server_start_error.store(error_code, std::memory_order_relaxed);
    g_api_request_success_server_ready.store(true, std::memory_order_release);
    return ;
}

static bool api_request_success_server_wait_until_ready(void)
{
    while (!g_api_request_success_server_ready.load(std::memory_order_acquire))
        api_request_small_delay();
    if (g_api_request_success_server_start_error.load(std::memory_order_acquire) != ER_SUCCESS)
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
    if (server_socket.get_error() != ER_SUCCESS)
    {
        api_request_success_server_signal_ready(server_socket.get_error());
        return ;
    }
    api_request_success_server_signal_ready(ER_SUCCESS);
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
        FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
        return ;
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
        FT_CLOSE_SOCKET(client_fd);
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
    FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
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
            FT_CLOSE_SOCKET(client_fd);
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
        FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
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
        FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
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
        FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
        return ;
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
            reinterpret_cast<struct sockaddr*>(&address_storage),
            &address_length);
    if (client_fd < 0)
        return ;
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
    FT_CLOSE_SOCKET(client_fd);
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
    if (server_socket.get_error() != ER_SUCCESS)
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
    FT_CLOSE_SOCKET(client_fd);
    return ;
}

FT_TEST(test_api_request_invalid_ip_sets_socket_error, "api_request_string invalid ip sets socket error")
{
    char *result;

    ft_errno = ER_SUCCESS;
    result = api_request_string("bad-ip", 8080, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 10);
    if (result != ft_nullptr)
        return (0);
    if (ft_errno != FT_ERR_CONFIGURATION)
        return (0);
    return (1);
}

FT_TEST(test_api_request_connect_failure_sets_errno, "api_request_string_host connect failure sets errno")
{
    char *result;

    ft_errno = ER_SUCCESS;
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
    ft_errno = ER_SUCCESS;
    server_thread = ft_thread(api_request_send_failure_server);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    result = api_request_string("127.0.0.1", 54337, "GET", "/", ft_nullptr, ft_nullptr, ft_nullptr, 1000);
    int request_errno = ft_errno;
    server_thread.join();
    if (result != ft_nullptr)
        return (0);
    if (request_errno != FT_ERR_SOCKET_SEND_FAILED && request_errno != (EPIPE + ERRNO_OFFSET))
        return (0);
    if (ft_errno != ER_SUCCESS)
        return (0);
    ft_errno = request_errno;
    return (1);
}

FT_TEST(test_api_request_async_alloc_failure_sets_errno, "api_request_string_async allocation failure sets errno")
{
    bool result;

    ft_errno = ER_SUCCESS;
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

    ft_errno = ER_SUCCESS;
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
    if (server_thread.get_error() != ER_SUCCESS)
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
    if (ft_errno != ER_SUCCESS)
        return (0);
    cma_free(body);
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
    ft_errno = ER_SUCCESS;
    context.total_bytes = 0;
    context.chunk_count = 0;
    context.headers_received = false;
    context.final_received = false;
    context.status_code = 0;
    handler.headers_callback = api_request_stream_headers_callback;
    handler.body_callback = api_request_stream_body_callback;
    handler.user_data = &context;
    server_thread = ft_thread(api_request_stream_large_response_server);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    expected_size = 2 * 1024 * 1024;
    result = api_request_stream("127.0.0.1", 54358, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 5000, ft_nullptr);
    printf("debug result=%d ft_errno=%d total=%zu final=%d chunk_count=%zu\n",
        result ? 1 : 0, ft_errno, context.total_bytes,
        context.final_received ? 1 : 0, context.chunk_count);
    server_thread.join();
    if (!result)
        return (0);
    if (ft_errno != ER_SUCCESS)
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
    ft_errno = ER_SUCCESS;
    context.total_bytes = 0;
    context.chunk_count = 0;
    context.headers_received = false;
    context.final_received = false;
    context.status_code = 0;
    handler.headers_callback = api_request_stream_headers_callback;
    handler.body_callback = api_request_stream_body_callback;
    handler.user_data = &context;
    server_thread = ft_thread(api_request_stream_chunked_response_server);
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    expected_size = 24;
    result = api_request_stream("127.0.0.1", 54359, "GET", "/", &handler,
            ft_nullptr, ft_nullptr, 2000, ft_nullptr);
    server_thread.join();
    if (!result)
        return (0);
    if (ft_errno != ER_SUCCESS)
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
    if (server_thread.get_error() != ER_SUCCESS)
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

    ft_errno = ER_SUCCESS;
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

    ft_errno = ER_SUCCESS;
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

    ft_errno = ER_SUCCESS;
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
    if (ft_errno != ER_SUCCESS)
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

    input_frame.type = 0x1;
    input_frame.flags = 0x5;
    input_frame.stream_id = 3;
    input_frame.payload = "Hello";
    error_code = ER_SUCCESS;
    if (!http2_encode_frame(input_frame, encoded, error_code))
        return (0);
    if (error_code != ER_SUCCESS)
        return (0);
    offset = 0;
    if (!http2_decode_frame(reinterpret_cast<const unsigned char*>(encoded.c_str()),
            encoded.size(), offset, decoded_frame, error_code))
        return (0);
    if (error_code != ER_SUCCESS)
        return (0);
    if (decoded_frame.type != input_frame.type)
        return (0);
    if (decoded_frame.flags != input_frame.flags)
        return (0);
    if (decoded_frame.stream_id != input_frame.stream_id)
        return (0);
    if (!(decoded_frame.payload == input_frame.payload))
        return (0);
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
    if (server_thread.get_error() != ER_SUCCESS)
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
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    retry_policy.max_attempts = 3;
    retry_policy.initial_delay_ms = 10;
    retry_policy.max_delay_ms = 0;
    retry_policy.backoff_multiplier = 1;
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
    if (ft_errno != ER_SUCCESS)
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
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    retry_policy.max_attempts = 2;
    retry_policy.initial_delay_ms = 5;
    retry_policy.max_delay_ms = 0;
    retry_policy.backoff_multiplier = 1;
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
    if (server_thread.get_error() != ER_SUCCESS)
        return (0);
    api_request_small_delay();
    retry_policy.max_attempts = 3;
    retry_policy.initial_delay_ms = 5;
    retry_policy.max_delay_ms = 10;
    retry_policy.backoff_multiplier = 2;
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

