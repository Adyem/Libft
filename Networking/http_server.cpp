#include "http_server.hpp"
#include "networking.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Time/time.hpp"
#include "../Observability/observability_networking_metrics.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>
#ifdef _WIN32
# include <winsock2.h>
#endif

ft_http_server::ft_http_server()
    : _server_socket(), _error_code(ER_SUCCESS), _non_blocking(false)
{
    return ;
}

ft_http_server::~ft_http_server()
{
    this->_server_socket.close_socket();
    return ;
}

void ft_http_server::set_error(int error_code) const
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

int ft_http_server::start(const char *ip, uint16_t port, int address_family, bool non_blocking)
{
    SocketConfig configuration;

    configuration._type = SocketType::SERVER;
    configuration._ip = ip;
    configuration._port = port;
    configuration._address_family = address_family;
    configuration._non_blocking = non_blocking;
    configuration._recv_timeout = 5000;
    configuration._send_timeout = 5000;
    this->_server_socket = ft_socket(configuration);
    if (this->_server_socket.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_server_socket.get_error());
        return (1);
    }
    ft_errno = ER_SUCCESS;
    this->_error_code = ER_SUCCESS;
    this->_non_blocking = non_blocking;
    return (0);
}

static int parse_request(const ft_string &request, ft_string &body, bool &is_post)
{
    const char *request_data;
    const char *body_separator;

    request_data = request.c_str();
    if (ft_strncmp(request_data, "POST ", 5) == 0)
        is_post = true;
    else if (ft_strncmp(request_data, "GET ", 4) == 0)
        is_post = false;
    else
        return (FT_ERR_INVALID_ARGUMENT);
    body_separator = ft_strstr(request_data, "\r\n\r\n");
    if (body_separator)
    {
        body_separator += 4;
        body = body_separator;
    }
    else
        body.clear();
    return (ER_SUCCESS);
}

static void http_server_record_metrics(const char *method, size_t request_bytes,
    size_t response_bytes, int status_code, int result,
    t_monotonic_time_point start_time)
{
    ft_networking_observability_sample sample;
    t_monotonic_time_point finish_time;
    long long duration_ms;
    int error_code;

    finish_time = time_monotonic_point_now();
    duration_ms = time_monotonic_point_diff_ms(start_time, finish_time);
    if (duration_ms < 0)
        duration_ms = 0;
    error_code = ER_SUCCESS;
    if (result != 0)
        error_code = ft_errno;
    sample.labels.component = "http_server";
    sample.labels.operation = method;
    sample.labels.target = "listener";
    sample.labels.resource = NULL;
    sample.duration_ms = duration_ms;
    sample.request_bytes = request_bytes;
    sample.response_bytes = response_bytes;
    sample.status_code = status_code;
    sample.error_code = error_code;
    if (error_code == ER_SUCCESS)
    {
        sample.success = true;
        sample.error_tag = "ok";
    }
    else
    {
        sample.success = false;
        sample.error_tag = ft_strerror(error_code);
    }
    observability_networking_metrics_record(sample);
    return ;
}

int ft_http_server::run_once()
{
    t_monotonic_time_point start_time;
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int client_socket;
    char buffer[1024];
    ssize_t bytes_received;
    ft_string request;
    ft_string body;
    ft_string response;
    bool is_post;
    int parse_error;
    char length_string[32];
    bool header_complete;
    bool request_complete;
    bool has_content_length;
    size_t expected_body_length;
    size_t header_length;
    const size_t max_request_size = 65536;
    bool metrics_enabled;
    const char *method_label;
    size_t request_bytes;
    size_t response_bytes;
    int status_code_value;
    int final_result;

    start_time = time_monotonic_point_now();
    address_length = sizeof(client_address);
    client_socket = nw_accept(this->_server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&client_address), &address_length);
    if (client_socket < 0)
    {
#ifdef _WIN32
        int last_error;

        last_error = WSAGetLastError();
        if (this->_non_blocking != false && last_error == WSAEWOULDBLOCK)
        {
            ft_errno = ER_SUCCESS;
            this->_error_code = ER_SUCCESS;
            return (0);
        }
        this->set_error(ft_map_system_error(last_error));
#else
        int last_error;

        last_error = errno;
        if (this->_non_blocking != false && (last_error == EAGAIN || last_error == EWOULDBLOCK))
        {
            ft_errno = ER_SUCCESS;
            this->_error_code = ER_SUCCESS;
            return (0);
        }
        this->set_error(ft_map_system_error(last_error));
#endif
        return (1);
    }
    metrics_enabled = true;
    method_label = "UNKNOWN";
    request_bytes = 0;
    response_bytes = 0;
    status_code_value = 0;
    final_result = 1;
    header_complete = false;
    request_complete = false;
    has_content_length = false;
    expected_body_length = 0;
    header_length = 0;
    while (request_complete == false)
    {
        bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received < 0)
        {
            nw_close(client_socket);
#ifdef _WIN32
            int last_error;
            int library_error_code;

            last_error = WSAGetLastError();
            library_error_code = ft_map_system_error(last_error);
            this->set_error(library_error_code);
#else
            this->set_error(ft_map_system_error(errno));
#endif
            request_bytes = request.size();
            final_result = 1;
            goto cleanup;
        }
        if (bytes_received == 0)
        {
            nw_close(client_socket);
            this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
            request_bytes = request.size();
            final_result = 1;
            goto cleanup;
        }
        buffer[bytes_received] = '\0';
        request.append(buffer);
        if (request.size() > max_request_size)
        {
            nw_close(client_socket);
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            request_bytes = request.size();
            final_result = 1;
            goto cleanup;
        }
        if (header_complete == false)
        {
            const char *full_request;
            const char *header_end_pointer;

            full_request = request.c_str();
            header_end_pointer = ft_strstr(full_request, "\r\n\r\n");
            if (header_end_pointer != ft_nullptr)
            {
                const char *content_length_header;
                const char *content_length_key;
                size_t content_length_key_length;

                header_end_pointer += 4;
                header_length = static_cast<size_t>(header_end_pointer - full_request);
                header_complete = true;
                content_length_key = "Content-Length:";
                content_length_key_length = sizeof("Content-Length:") - 1;
                content_length_header = ft_strnstr(full_request, content_length_key, header_length);
                if (content_length_header != ft_nullptr)
                {
                    const char *length_value_pointer;
                    bool has_length_digits;

                    length_value_pointer = content_length_header + content_length_key_length;
                    while (*length_value_pointer != '\0'
                        && ft_isspace(static_cast<unsigned char>(*length_value_pointer)) != 0)
                    {
                        length_value_pointer++;
                    }
                    if (*length_value_pointer == '\0'
                        || ft_isdigit(static_cast<unsigned char>(*length_value_pointer)) == 0)
                    {
                        nw_close(client_socket);
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        request_bytes = request.size();
                        final_result = 1;
                        goto cleanup;
                    }
                    has_length_digits = false;
                    expected_body_length = 0;
                    while (*length_value_pointer != '\0'
                        && ft_isdigit(static_cast<unsigned char>(*length_value_pointer)) != 0)
                    {
                        size_t digit_value;

                        has_length_digits = true;
                        digit_value = static_cast<size_t>(*length_value_pointer - '0');
                        if (expected_body_length > (max_request_size - digit_value) / 10)
                        {
                            nw_close(client_socket);
                            this->set_error(FT_ERR_INVALID_ARGUMENT);
                            request_bytes = request.size();
                            final_result = 1;
                            goto cleanup;
                        }
                        expected_body_length = expected_body_length * 10 + digit_value;
                        length_value_pointer++;
                    }
                    if (has_length_digits == false)
                    {
                        nw_close(client_socket);
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        request_bytes = request.size();
                        final_result = 1;
                        goto cleanup;
                    }
                    if (expected_body_length > max_request_size)
                    {
                        nw_close(client_socket);
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        request_bytes = request.size();
                        final_result = 1;
                        goto cleanup;
                    }
                    has_content_length = true;
                }
            }
        }
        if (header_complete != false)
        {
            size_t current_body_size;

            if (request.size() >= header_length)
                current_body_size = request.size() - header_length;
            else
                current_body_size = 0;
            if (has_content_length != false)
            {
                if (current_body_size >= expected_body_length)
                    request_complete = true;
            }
            else
                request_complete = true;
        }
    }
    request_bytes = request.size();
    if (has_content_length != false)
    {
        size_t total_expected_size;

        total_expected_size = header_length + expected_body_length;
        if (request.size() > total_expected_size)
            request.erase(total_expected_size, request.size() - total_expected_size);
    }
    parse_error = parse_request(request, body, is_post);
    if (parse_error != ER_SUCCESS)
    {
        nw_close(client_socket);
        this->set_error(parse_error);
        final_result = 1;
        goto cleanup;
    }
    if (is_post)
        method_label = "POST";
    else
        method_label = "GET";
    if (is_post && !body.empty())
    {
        std::snprintf(length_string, sizeof(length_string), "%zu", body.size());
        response.append("HTTP/1.1 200 OK\r\nContent-Length: ");
        response.append(length_string);
        response.append("\r\n\r\n");
        response.append(body);
    }
    else if (!is_post)
        response.append("HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nGET");
    else
        response.append("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n");
    const char *response_data;
    size_t total_sent;
    ssize_t send_result;

    response_data = response.c_str();
    total_sent = 0;
    response_bytes = response.size();
    while (total_sent < response.size())
    {
        send_result = nw_send(client_socket, response_data + total_sent, response.size() - total_sent, 0);
        if (send_result <= 0)
        {
            int last_socket_error;

#ifdef _WIN32
            last_socket_error = WSAGetLastError();
#else
            last_socket_error = errno;
#endif
            nw_close(client_socket);
            if (send_result < 0)
                this->set_error(ft_map_system_error(last_socket_error));
            else
                this->set_error(FT_ERR_SOCKET_SEND_FAILED);
            final_result = 1;
            goto cleanup;
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_socket_after_send(client_socket) != 0)
    {
        nw_close(client_socket);
        this->set_error(ft_errno);
        final_result = 1;
        goto cleanup;
    }
    nw_close(client_socket);
    ft_errno = ER_SUCCESS;
    this->_error_code = ER_SUCCESS;
    status_code_value = 200;
    final_result = 0;
    goto cleanup;

cleanup:
    if (metrics_enabled != false)
        http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, final_result, start_time);
    return (final_result);
}

int ft_http_server::get_error() const
{
    return (this->_error_code);
}

const char *ft_http_server::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
