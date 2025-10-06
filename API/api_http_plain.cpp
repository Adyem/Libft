#include "api_http_internal.hpp"
#include "api.hpp"
#include "api_http_common.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/http2_client.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Logger/logger.hpp"
#include "../Printf/printf.hpp"
#include "../Time/time.hpp"
#include <errno.h>
#include <utility>
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

static bool api_http_apply_timeouts(ft_socket &socket_wrapper, int timeout)
{
    if (timeout <= 0)
        return (true);
#ifdef _WIN32
    DWORD win_timeout;

    win_timeout = static_cast<DWORD>(timeout);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_RCVTIMEO,
            reinterpret_cast<const char*>(&win_timeout), sizeof(win_timeout)) != 0)
        return (false);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
            reinterpret_cast<const char*>(&win_timeout), sizeof(win_timeout)) != 0)
        return (false);
#else
    struct timeval tv;

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_RCVTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
    if (setsockopt(socket_wrapper.get_fd(), SOL_SOCKET, SO_SNDTIMEO,
            &tv, sizeof(tv)) != 0)
        return (false);
#endif
    return (true);
}

static bool api_http_is_recoverable_send_error(int error_code)
{
    if (error_code == SOCKET_SEND_FAILED)
        return (true);
#ifdef _WIN32
    if (error_code == (WSAECONNRESET + ERRNO_OFFSET))
        return (true);
    if (error_code == (WSAECONNABORTED + ERRNO_OFFSET))
        return (true);
#else
    if (error_code == (ECONNRESET + ERRNO_OFFSET))
        return (true);
    if (error_code == (EPIPE + ERRNO_OFFSET))
        return (true);
#endif
    return (false);
}

static bool api_http_should_retry_plain(int error_code)
{
    if (api_http_is_recoverable_send_error(error_code))
        return (true);
    if (error_code == SOCKET_RECEIVE_FAILED)
        return (true);
    if (error_code == SOCKET_CONNECT_FAILED)
        return (true);
    return (false);
}

static bool api_http_prepare_plain_socket(
    api_connection_pool_handle &connection_handle, const char *host,
    uint16_t port, int timeout, int &error_code)
{
    bool pooled_connection;

    if (connection_handle.has_socket)
        return (true);
    pooled_connection = api_connection_pool_acquire(connection_handle, host, port,
            api_connection_security_mode::PLAIN, ft_nullptr);
    if (pooled_connection)
        return (true);
    SocketConfig config;

    config._type = SocketType::CLIENT;
    config._ip = host;
    config._port = port;
    config._recv_timeout = timeout;
    config._send_timeout = timeout;
    ft_socket new_socket(config);
    if (new_socket.get_error())
    {
        int socket_error_code;

        socket_error_code = new_socket.get_error();
        if (socket_error_code == SOCKET_INVALID_CONFIGURATION)
            error_code = SOCKET_INVALID_CONFIGURATION;
        else
            error_code = SOCKET_CONNECT_FAILED;
        return (false);
    }
    connection_handle.socket = std::move(new_socket);
    connection_handle.has_socket = true;
    connection_handle.from_pool = false;
    connection_handle.should_store = true;
    connection_handle.security_mode = api_connection_security_mode::PLAIN;
    return (true);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    bool &used_http2, int &error_code);

static bool api_http_send_request(ft_socket &socket_wrapper,
    const ft_string &request, int &error_code)
{
    ssize_t sent;

    if (request.empty())
        return (true);
    sent = socket_wrapper.send_all(request.c_str(), request.size());
    if (sent < 0)
    {
        if (socket_wrapper.get_error())
            error_code = socket_wrapper.get_error();
        else if (ft_errno != ER_SUCCESS)
            error_code = ft_errno;
        else
            error_code = FT_EIO;
        return (false);
    }
    return (true);
}

static bool api_http_prepare_request(const char *method, const char *path,
    const char *host_header, json_group *payload, const char *headers,
    ft_string &request, ft_string &body_string, int &error_code)
{
    char *temporary_string;

    request.clear();
    body_string.clear();
    if (!method || !path)
    {
        error_code = FT_EINVAL;
        return (false);
    }
    request = method;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += " ";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += path;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += " HTTP/1.1\r\n";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (!host_header)
        host_header = "";
    request += "Host: ";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    request += host_header;
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (headers && headers[0])
    {
        request += "\r\n";
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
        request += headers;
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
    }
    if (payload)
    {
        temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            if (ft_errno == ER_SUCCESS)
                error_code = FT_EALLOC;
            else
                error_code = ft_errno;
            return (false);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        if (body_string.get_error())
        {
            error_code = body_string.get_error();
            return (false);
        }
        request += "\r\nContent-Type: application/json";
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
        if (!api_append_content_length_header(request, body_string.size()))
        {
            if (ft_errno == ER_SUCCESS)
                error_code = FT_EIO;
            else
                error_code = ft_errno;
            return (false);
        }
    }
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (request.get_error())
    {
        error_code = request.get_error();
        return (false);
    }
    if (payload)
    {
        request += body_string.c_str();
        if (request.get_error())
        {
            error_code = request.get_error();
            return (false);
        }
    }
    return (true);
}

static bool api_http_receive_response(ft_socket &socket_wrapper,
    ft_string &response, size_t &header_length, bool &connection_close,
    bool &chunked_encoding, bool &has_length, long long &content_length,
    int &error_code)
{
    char buffer[1024];
    ssize_t received;

    response.clear();
    header_length = 0;
    connection_close = false;
    chunked_encoding = false;
    has_length = false;
    content_length = 0;
    while (true)
    {
        received = socket_wrapper.receive_data(buffer, sizeof(buffer) - 1);
        if (received < 0)
        {
            if (socket_wrapper.get_error())
                error_code = socket_wrapper.get_error();
            else if (ft_errno != ER_SUCCESS)
                error_code = ft_errno;
            else
                error_code = FT_EIO;
            return (false);
        }
        if (received == 0)
            break;
        buffer[received] = '\0';
        response += buffer;
        if (response.get_error())
        {
            error_code = response.get_error();
            return (false);
        }
        const char *headers_start;
        const char *headers_end;

        headers_start = response.c_str();
        headers_end = ft_strstr(response.c_str(), "\r\n\r\n");
        if (headers_end)
        {
            headers_end += 2;
            header_length = static_cast<size_t>(headers_end - headers_start) + 2;
            api_http_parse_headers(headers_start, headers_end, connection_close,
                chunked_encoding, has_length, content_length);
            if (chunked_encoding)
            {
                size_t body_size;
                size_t consumed_length;

                body_size = response.size() - header_length;
                if (api_http_chunked_body_complete(response.c_str() + header_length,
                        body_size, consumed_length))
                {
                    if (consumed_length <= body_size)
                        return (true);
                }
            }
            else if (has_length)
            {
                size_t body_size;

                body_size = response.size() - header_length;
                if (body_size >= static_cast<size_t>(content_length))
                    return (true);
            }
        }
        else if (response.size() >= sizeof(buffer))
        {
            error_code = FT_EIO;
            return (false);
        }
        if (received < static_cast<ssize_t>(sizeof(buffer) - 1))
        {
            if (!headers_end)
            {
                error_code = FT_EIO;
                return (false);
            }
            return (true);
        }
    }
    if (response.size() == 0)
    {
        error_code = FT_EIO;
        return (false);
    }
    const char *headers_start;
    const char *headers_end;

    headers_start = response.c_str();
    headers_end = ft_strstr(response.c_str(), "\r\n\r\n");
    if (!headers_end)
    {
        error_code = FT_EIO;
        return (false);
    }
    headers_end += 2;
    header_length = static_cast<size_t>(headers_end - headers_start) + 2;
    api_http_parse_headers(headers_start, headers_end, connection_close,
        chunked_encoding, has_length, content_length);
    return (true);
}

static char *api_http_execute_plain_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    int &error_code)
{
    ft_socket &socket_wrapper = connection_handle.socket;

    if (socket_wrapper.get_error())
    {
        error_code = socket_wrapper.get_error();
        return (ft_nullptr);
    }
    if (!api_http_apply_timeouts(socket_wrapper, timeout))
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (last_error != 0)
            error_code = last_error + ERRNO_OFFSET;
        else
            error_code = SOCKET_INVALID_CONFIGURATION;
#else
        if (errno != 0)
            error_code = errno + ERRNO_OFFSET;
        else
            error_code = SOCKET_INVALID_CONFIGURATION;
#endif
        return (ft_nullptr);
    }

    ft_string request;
    ft_string body_string;
    if (!api_http_prepare_request(method, path, host_header, payload, headers,
            request, body_string, error_code))
        return (ft_nullptr);
    bool send_failed;
    int send_error_code;

    send_failed = false;
    send_error_code = ER_SUCCESS;
    if (!api_http_send_request(socket_wrapper, request, error_code))
    {
        send_failed = true;
        send_error_code = error_code;
        api_connection_pool_disable_store(connection_handle);
#ifdef _WIN32
        if (send_error_code == (WSAECONNRESET + ERRNO_OFFSET))
            send_error_code = SOCKET_SEND_FAILED;
        if (send_error_code == (WSAECONNABORTED + ERRNO_OFFSET))
            send_error_code = SOCKET_SEND_FAILED;
#else
        if (send_error_code == (ECONNRESET + ERRNO_OFFSET))
            send_error_code = SOCKET_SEND_FAILED;
        if (send_error_code == (EPIPE + ERRNO_OFFSET))
            send_error_code = SOCKET_SEND_FAILED;
#endif
        if (!api_http_is_recoverable_send_error(send_error_code))
            return (ft_nullptr);
    }

    ft_string response;
    size_t header_length;
    bool connection_close;
    bool chunked_encoding;
    bool has_length;
    long long content_length;

    if (send_failed)
        error_code = ER_SUCCESS;
    if (!api_http_receive_response(socket_wrapper, response, header_length,
            connection_close, chunked_encoding, has_length, content_length,
            error_code))
    {
        if (send_failed)
            error_code = send_error_code;
        return (ft_nullptr);
    }
    if (send_failed)
        connection_close = true;
    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }
    const char *body_start = response.c_str() + header_length;
    size_t body_length = response.size() - header_length;
    ft_string decoded_body;
    const char *result_source;
    size_t result_length;

    result_source = body_start;
    result_length = body_length;
    if (chunked_encoding)
    {
        size_t consumed_length;

        if (!api_http_decode_chunked(body_start, body_length, decoded_body,
                consumed_length))
        {
            error_code = FT_EIO;
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    else if (has_length)
    {
        size_t expected_length;
        size_t index;

        expected_length = static_cast<size_t>(content_length);
        if (body_length < expected_length)
        {
            error_code = FT_EIO;
            return (ft_nullptr);
        }
        decoded_body.clear();
        index = 0;
        while (index < expected_length)
        {
            decoded_body.append(body_start[index]);
            if (decoded_body.get_error())
            {
                error_code = decoded_body.get_error();
                return (ft_nullptr);
            }
            index++;
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    else
    {
        decoded_body.clear();
        decoded_body.append(body_start);
        if (decoded_body.get_error())
        {
            error_code = decoded_body.get_error();
            return (ft_nullptr);
        }
        result_source = decoded_body.c_str();
        result_length = decoded_body.size();
    }
    if (connection_close)
        api_connection_pool_disable_store(connection_handle);
    char *result_body;

    result_body = static_cast<char*>(cma_malloc(result_length + 1));
    if (!result_body)
    {
        if (ft_errno == ER_SUCCESS)
            error_code = FT_EALLOC;
        else
            error_code = ft_errno;
        return (ft_nullptr);
    }
    if (result_length > 0)
        ft_memcpy(result_body, result_source, result_length);
    result_body[result_length] = '\0';
    error_code = ER_SUCCESS;
    return (result_body);
}

char *api_http_execute_plain_http2(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, bool &used_http2, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;
    bool http2_used_local;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    http2_used_local = false;
    while (attempt_index < max_attempts)
    {
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            http2_used_local = false;
            char *result_body;

            result_body = api_http_execute_plain_http2_once(connection_handle,
                    method, path, host_header, payload, headers, status,
                    timeout, http2_used_local, error_code);
            if (result_body)
            {
                used_http2 = http2_used_local;
                return (result_body);
            }
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            break ;
        api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (attempt_index >= max_attempts)
            break;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay,
                    max_delay);
    }
    used_http2 = false;
    if (error_code == ER_SUCCESS)
        error_code = FT_EIO;
    return (ft_nullptr);
}

char *api_http_execute_plain(api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    const char *host, uint16_t port,
    const api_retry_policy *retry_policy, int &error_code)
{
    int max_attempts;
    int attempt_index;
    int initial_delay;
    int current_delay;
    int max_delay;
    int multiplier;

    max_attempts = api_retry_get_max_attempts(retry_policy);
    initial_delay = api_retry_get_initial_delay(retry_policy);
    max_delay = api_retry_get_max_delay(retry_policy);
    multiplier = api_retry_get_multiplier(retry_policy);
    current_delay = api_retry_prepare_delay(initial_delay, max_delay);
    attempt_index = 0;
    while (attempt_index < max_attempts)
    {
        bool socket_ready;
        bool should_retry;

        socket_ready = api_http_prepare_plain_socket(connection_handle, host,
                port, timeout, error_code);
        if (socket_ready)
        {
            char *result_body;

            result_body = api_http_execute_plain_once(connection_handle, method,
                    path, host_header, payload, headers, status, timeout,
                    error_code);
            if (result_body)
                return (result_body);
        }
        should_retry = api_http_should_retry_plain(error_code);
        if (!should_retry)
            return (ft_nullptr);
        api_connection_pool_evict(connection_handle);
        attempt_index++;
        if (attempt_index >= max_attempts)
            break;
        if (current_delay > 0)
        {
            int sleep_delay;

            sleep_delay = api_retry_prepare_delay(current_delay, max_delay);
            if (sleep_delay > 0)
                time_sleep_ms(static_cast<unsigned int>(sleep_delay));
        }
        current_delay = api_retry_next_delay(current_delay, max_delay,
                multiplier);
        if (current_delay <= 0)
            current_delay = api_retry_prepare_delay(initial_delay,
                    max_delay);
    }
    if (error_code == ER_SUCCESS)
        error_code = FT_EIO;
    return (ft_nullptr);
}

static char *api_http_execute_plain_http2_once(
    api_connection_pool_handle &connection_handle,
    const char *method, const char *path, const char *host_header,
    json_group *payload, const char *headers, int *status, int timeout,
    bool &used_http2, int &error_code)
{
    ft_vector<http2_header_field> header_fields;
    http2_header_field field_entry;
    ft_string compressed_headers;
    http2_frame headers_frame;
    ft_string encoded_frame;
    http2_stream_manager stream_manager;
    char *http_response;

    used_http2 = false;
    error_code = ER_SUCCESS;
    if (!method || !path)
    {
        error_code = FT_EINVAL;
        return (ft_nullptr);
    }
    field_entry.name = ":method";
    field_entry.value = method;
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":path";
    if (!path)
        field_entry.value = "";
    else
        field_entry.value = path;
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":scheme";
    field_entry.value = "http";
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    field_entry.name = ":authority";
    if (host_header)
        field_entry.value = host_header;
    else
        field_entry.value = "";
    header_fields.push_back(field_entry);
    if (header_fields.get_error() != ER_SUCCESS)
    {
        error_code = header_fields.get_error();
        return (ft_nullptr);
    }
    if (headers && headers[0])
    {
        const char *header_cursor;
        ft_string header_name;
        ft_string header_value;

        header_cursor = headers;
        while (*header_cursor != '\0')
        {
            size_t index;

            header_name.clear();
            header_value.clear();
            index = 0;
            while (header_cursor[index] && header_cursor[index] != ':' && header_cursor[index] != '\r')
            {
                header_name.append(header_cursor[index]);
                if (header_name.get_error() != ER_SUCCESS)
                {
                    error_code = header_name.get_error();
                    return (ft_nullptr);
                }
                index++;
            }
            while (header_cursor[index] == ':' || header_cursor[index] == ' ')
                index++;
            while (header_cursor[index] && header_cursor[index] != '\r' && header_cursor[index] != '\n')
            {
                header_value.append(header_cursor[index]);
                if (header_value.get_error() != ER_SUCCESS)
                {
                    error_code = header_value.get_error();
                    return (ft_nullptr);
                }
                index++;
            }
            if (header_name.size() > 0)
            {
                field_entry.name = header_name;
                field_entry.value = header_value;
                header_fields.push_back(field_entry);
                if (header_fields.get_error() != ER_SUCCESS)
                {
                    error_code = header_fields.get_error();
                    return (ft_nullptr);
                }
            }
            while (header_cursor[index] == '\r' || header_cursor[index] == '\n')
                index++;
            header_cursor += index;
        }
    }
    if (!http2_compress_headers(header_fields, compressed_headers, error_code))
    {
        if (error_code == ER_SUCCESS)
            error_code = FT_EIO;
        return (ft_nullptr);
    }
    headers_frame.type = 0x1;
    headers_frame.flags = 0x4;
    headers_frame.stream_id = 1;
    headers_frame.payload = compressed_headers;
    if (headers_frame.payload.get_error() != ER_SUCCESS)
    {
        error_code = headers_frame.payload.get_error();
        return (ft_nullptr);
    }
    if (!http2_encode_frame(headers_frame, encoded_frame, error_code))
    {
        if (error_code == ER_SUCCESS)
            error_code = FT_EIO;
        return (ft_nullptr);
    }
    if (!stream_manager.open_stream(1))
    {
        error_code = stream_manager.get_error();
        return (ft_nullptr);
    }
    used_http2 = false;
    http_response = api_http_execute_plain_once(connection_handle, method, path,
            host_header, payload, headers, status, timeout, error_code);
    if (!http_response)
        return (ft_nullptr);
    size_t body_length;

    body_length = ft_strlen(http_response);
    if (!stream_manager.append_data(1, http_response, body_length))
    {
        error_code = stream_manager.get_error();
        cma_free(http_response);
        return (ft_nullptr);
    }
    if (!stream_manager.close_stream(1))
    {
        error_code = stream_manager.get_error();
        cma_free(http_response);
        return (ft_nullptr);
    }
    error_code = ER_SUCCESS;
    return (http_response);
}
