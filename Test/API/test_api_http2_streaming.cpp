#include "../../API/api_http_internal.hpp"
#include "../../API/api.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Networking/networking.hpp"
#include "../../Networking/socket_class.hpp"
#include "../../Networking/http2_client.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include <atomic>
#include <cstdint>
#include <cstring>
#include <thread>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

struct http2_test_server_state
{
    std::atomic<bool> ready;
    std::atomic<int> start_error;
    std::atomic<int> result;
};

struct http2_stream_capture
{
    ft_string headers;
    ft_string body;
    unsigned int header_calls;
    unsigned int body_calls;
    bool final_chunk_seen;
    int status_code;
};

static const uint16_t g_http2_test_server_port = 54613;

static void http2_test_server_reset_state(http2_test_server_state *state)
{
    if (!state)
        return ;
    state->ready.store(false, std::memory_order_relaxed);
    state->start_error.store(ER_SUCCESS, std::memory_order_relaxed);
    state->result.store(FT_ERR_IO, std::memory_order_relaxed);
    return ;
}

static void http2_test_sleep_briefly(void)
{
#ifdef _WIN32
    Sleep(100);
#else
    usleep(100000);
#endif
    return ;
}

static bool http2_test_server_wait_until_ready(http2_test_server_state *state)
{
    size_t wait_iterations;

    if (!state)
        return (false);
    wait_iterations = 0;
    while (!state->ready.load(std::memory_order_acquire))
    {
        if (wait_iterations >= 50)
            return (false);
        http2_test_sleep_briefly();
        wait_iterations += 1;
    }
    return (true);
}

static bool http2_test_read_exact(int socket_fd, char *buffer, size_t length)
{
    size_t total_bytes;

    if (socket_fd < 0 || !buffer)
        return (false);
    total_bytes = 0;
    while (total_bytes < length)
    {
        ssize_t received_bytes;

        received_bytes = nw_recv(socket_fd, buffer + total_bytes,
            length - total_bytes, 0);
        if (received_bytes <= 0)
            return (false);
        total_bytes += static_cast<size_t>(received_bytes);
    }
    return (true);
}

static bool http2_test_send_all(int socket_fd, const char *buffer, size_t length)
{
    size_t total_bytes;

    if (socket_fd < 0 || !buffer)
        return (false);
    total_bytes = 0;
    while (total_bytes < length)
    {
        ssize_t sent_bytes;

        sent_bytes = nw_send(socket_fd, buffer + total_bytes,
            length - total_bytes, 0);
        if (sent_bytes <= 0)
            return (false);
        total_bytes += static_cast<size_t>(sent_bytes);
    }
    return (true);
}

static bool http2_test_receive_single_frame(int socket_fd, http2_frame &out_frame)
{
    unsigned char header_buffer[9];
    size_t payload_length;
    uint32_t stream_identifier_value;

    if (!http2_test_read_exact(socket_fd,
            reinterpret_cast<char*>(header_buffer), sizeof(header_buffer)))
        return (false);
    payload_length = (static_cast<size_t>(header_buffer[0]) << 16);
    payload_length |= (static_cast<size_t>(header_buffer[1]) << 8);
    payload_length |= static_cast<size_t>(header_buffer[2]);
    if (!out_frame.set_type(header_buffer[3]))
        return (false);
    if (!out_frame.set_flags(header_buffer[4]))
        return (false);
    stream_identifier_value = (static_cast<uint32_t>(header_buffer[5] & 0x7F) << 24);
    stream_identifier_value |= (static_cast<uint32_t>(header_buffer[6]) << 16);
    stream_identifier_value |= (static_cast<uint32_t>(header_buffer[7]) << 8);
    stream_identifier_value |= static_cast<uint32_t>(header_buffer[8]);
    if (!out_frame.set_stream_identifier(stream_identifier_value))
        return (false);
    if (payload_length == 0)
    {
        out_frame.clear_payload();
        if (out_frame.get_error() != ER_SUCCESS)
            return (false);
        return (true);
    }
    ft_string payload_buffer;

    payload_buffer.clear();
    if (payload_buffer.get_error() != ER_SUCCESS)
        return (false);
    size_t remaining_length;

    remaining_length = payload_length;
    while (remaining_length > 0)
    {
        char chunk_buffer[1024];
        size_t request_length;
        ssize_t received_bytes;

        request_length = sizeof(chunk_buffer);
        if (request_length > remaining_length)
            request_length = remaining_length;
        received_bytes = nw_recv(socket_fd, chunk_buffer, request_length, 0);
        if (received_bytes <= 0)
            return (false);
        payload_buffer.append(chunk_buffer,
            static_cast<size_t>(received_bytes));
        if (payload_buffer.get_error() != ER_SUCCESS)
            return (false);
        remaining_length -= static_cast<size_t>(received_bytes);
    }
    if (!out_frame.set_payload(payload_buffer))
        return (false);
    return (true);
}

static bool http2_test_drain_client_headers(int socket_fd)
{
    ft_string buffer;
    bool headers_complete;

    buffer.clear();
    if (buffer.get_error() != ER_SUCCESS)
        return (false);
    headers_complete = false;
    while (!headers_complete)
    {
        char receive_buffer[1024];
        ssize_t received_bytes;

        received_bytes = nw_recv(socket_fd, receive_buffer,
            sizeof(receive_buffer), 0);
        if (received_bytes <= 0)
            return (false);
        buffer.append(receive_buffer,
            static_cast<size_t>(received_bytes));
        if (buffer.get_error() != ER_SUCCESS)
            return (false);
        size_t parse_offset;

        parse_offset = 0;
        while (true)
        {
            size_t previous_offset;
            http2_frame incoming_frame;
            int decode_error;

            previous_offset = parse_offset;
            decode_error = ER_SUCCESS;
            if (!http2_decode_frame(
                    reinterpret_cast<const unsigned char*>(buffer.c_str()),
                    buffer.size(), parse_offset, incoming_frame, decode_error))
            {
                if (decode_error == FT_ERR_OUT_OF_RANGE)
                {
                    parse_offset = previous_offset;
                    break ;
                }
                return (false);
            }
            uint8_t frame_type;
            uint8_t frame_flags;

            if (!incoming_frame.get_type(frame_type))
                return (false);
            if (!incoming_frame.get_flags(frame_flags))
                return (false);
            if (frame_type == 0x1)
            {
                if ((frame_flags & 0x4) != 0)
                    headers_complete = true;
            }
        }
        if (parse_offset > 0)
        {
            buffer.erase(0, parse_offset);
            if (buffer.get_error() != ER_SUCCESS)
                return (false);
        }
    }
    return (true);
}

static void http2_test_server(http2_test_server_state *state)
{
    SocketConfig server_configuration;
    ft_socket server_socket;
    struct sockaddr_storage address_storage;
    socklen_t address_length;
    int client_fd;
    char preface_buffer[24];
    http2_frame client_frame;
    http2_frame server_frame;
    ft_string encoded_frame;
    int encode_error;

    if (!state)
        return ;
    server_configuration._type = SocketType::SERVER;
    server_configuration._ip = "127.0.0.1";
    server_configuration._port = g_http2_test_server_port;
    server_configuration._reuse_address = true;
    server_socket = ft_socket(server_configuration);
    if (server_socket.get_error() != ER_SUCCESS)
    {
        state->start_error.store(server_socket.get_error(),
            std::memory_order_relaxed);
        state->ready.store(true, std::memory_order_release);
        return ;
    }
    state->start_error.store(ER_SUCCESS, std::memory_order_relaxed);
    state->ready.store(true, std::memory_order_release);
    address_length = sizeof(address_storage);
    client_fd = nw_accept(server_socket.get_fd(),
        reinterpret_cast<struct sockaddr*>(&address_storage),
        &address_length);
    if (client_fd < 0)
    {
        state->result.store(FT_ERR_SOCKET_ACCEPT_FAILED,
            std::memory_order_relaxed);
        return ;
    }
    if (!http2_test_read_exact(client_fd, preface_buffer, sizeof(preface_buffer)))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (ft_strncmp(preface_buffer, "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n",
            sizeof(preface_buffer)) != 0)
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_receive_single_frame(client_fd, client_frame))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_type(0x4))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_flags(0x0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_stream_identifier(0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    server_frame.clear_payload();
    if (server_frame.get_error() != ER_SUCCESS)
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_encode_frame(server_frame, encoded_frame, encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_send_all(client_fd, encoded_frame.c_str(),
            encoded_frame.size()))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_receive_single_frame(client_fd, client_frame))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_type(0x4))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_flags(0x1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_stream_identifier(0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    server_frame.clear_payload();
    if (server_frame.get_error() != ER_SUCCESS)
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_encode_frame(server_frame, encoded_frame, encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_send_all(client_fd, encoded_frame.c_str(),
            encoded_frame.size()))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_drain_client_headers(client_fd))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    ft_vector<http2_header_field> response_headers;
    http2_header_field header_entry;
    ft_string compressed_headers;

    response_headers.clear();
    if (response_headers.get_error() != ER_SUCCESS)
    {
        state->result.store(response_headers.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!header_entry.assign_from_cstr(":status", "200"))
    {
        state->result.store(header_entry.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    response_headers.push_back(header_entry);
    if (response_headers.get_error() != ER_SUCCESS)
    {
        state->result.store(response_headers.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!header_entry.assign_from_cstr("content-type", "text/plain"))
    {
        state->result.store(header_entry.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    response_headers.push_back(header_entry);
    if (response_headers.get_error() != ER_SUCCESS)
    {
        state->result.store(response_headers.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    compressed_headers.clear();
    if (compressed_headers.get_error() != ER_SUCCESS)
    {
        state->result.store(compressed_headers.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_compress_headers(response_headers, compressed_headers,
            encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_type(0x1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_flags(0x4))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_stream_identifier(1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_payload(compressed_headers))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_encode_frame(server_frame, encoded_frame, encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_send_all(client_fd, encoded_frame.c_str(),
            encoded_frame.size()))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_type(0x0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_flags(0x0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_stream_identifier(1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_payload_from_buffer("hello ", 6))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_encode_frame(server_frame, encoded_frame, encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_send_all(client_fd, encoded_frame.c_str(),
            encoded_frame.size()))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_type(0x0))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_flags(0x1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_stream_identifier(1))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!server_frame.set_payload_from_buffer("world", 5))
    {
        state->result.store(server_frame.get_error(),
            std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_encode_frame(server_frame, encoded_frame, encode_error))
    {
        if (encode_error == ER_SUCCESS)
            encode_error = FT_ERR_IO;
        state->result.store(encode_error, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    if (!http2_test_send_all(client_fd, encoded_frame.c_str(),
            encoded_frame.size()))
    {
        state->result.store(FT_ERR_IO, std::memory_order_relaxed);
        nw_close(client_fd);
        return ;
    }
    state->result.store(ER_SUCCESS, std::memory_order_relaxed);
    nw_close(client_fd);
    return ;
}

static void http2_test_capture_reset(http2_stream_capture &capture)
{
    capture.headers.clear();
    capture.body.clear();
    capture.header_calls = 0;
    capture.body_calls = 0;
    capture.final_chunk_seen = false;
    capture.status_code = -1;
    return ;
}

static void http2_test_headers_callback(int status_code, const char *headers,
    void *user_data)
{
    http2_stream_capture *capture;
    size_t index;

    capture = reinterpret_cast<http2_stream_capture*>(user_data);
    if (!capture)
        return ;
    if (headers)
    {
        index = 0;
        while (headers[index] != '\0')
        {
            capture->headers.append(headers[index]);
            index += 1;
        }
    }
    capture->header_calls += 1;
    capture->status_code = status_code;
    return ;
}

static bool http2_test_body_callback(const char *chunk_data, size_t chunk_size,
    bool is_final, void *user_data)
{
    http2_stream_capture *capture;
    size_t index;

    capture = reinterpret_cast<http2_stream_capture*>(user_data);
    if (!capture)
        return (true);
    index = 0;
    while (chunk_data && index < chunk_size)
    {
        capture->body.append(chunk_data[index]);
        index += 1;
    }
    capture->body_calls += 1;
    if (is_final)
        capture->final_chunk_seen = true;
    return (true);
}

FT_TEST(test_api_http_plain_http2_streaming_success,
    "api_http_execute_plain_http2_streaming streams response bodies")
{
    http2_test_server_state server_state;
    std::thread server_thread;
    bool ready_result;
    int start_error;
    api_connection_pool_handle connection_handle;
    api_streaming_handler streaming_handler;
    http2_stream_capture capture;
    bool used_http2;
    int error_code;
    bool request_result;

    http2_test_server_reset_state(&server_state);
    server_thread = std::thread(http2_test_server, &server_state);
    ready_result = http2_test_server_wait_until_ready(&server_state);
    if (!ready_result)
    {
        server_thread.join();
        FT_ASSERT(ready_result);
    }
    start_error = server_state.start_error.load(std::memory_order_acquire);
    if (start_error != ER_SUCCESS)
    {
        server_thread.join();
        FT_ASSERT_EQ(ER_SUCCESS, start_error);
    }
    http2_test_capture_reset(capture);
    streaming_handler.reset();
    streaming_handler.set_headers_callback(http2_test_headers_callback);
    streaming_handler.set_body_callback(http2_test_body_callback);
    streaming_handler.set_user_data(&capture);
    used_http2 = false;
    error_code = ER_SUCCESS;
    request_result = api_http_execute_plain_http2_streaming(
        connection_handle, "GET", "/resource", "localhost",
        ft_nullptr, ft_nullptr, 2000, "127.0.0.1",
        g_http2_test_server_port, ft_nullptr, &streaming_handler,
        used_http2, error_code);
    server_thread.join();
    FT_ASSERT(request_result);
    FT_ASSERT(used_http2);
    FT_ASSERT_EQ(ER_SUCCESS, error_code);
    FT_ASSERT_EQ(static_cast<unsigned int>(1), capture.header_calls);
    FT_ASSERT_EQ(200, capture.status_code);
    FT_ASSERT(capture.headers.find("content-type: text/plain")
        != ft_string::npos);
    FT_ASSERT_EQ(static_cast<unsigned int>(2), capture.body_calls);
    FT_ASSERT(capture.final_chunk_seen);
    FT_ASSERT_EQ(0, ft_strcmp(capture.body.c_str(), "hello world"));
    FT_ASSERT_EQ(ER_SUCCESS,
        server_state.result.load(std::memory_order_acquire));
    return (1);
}
