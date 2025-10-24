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
    : _server_socket(), _error_code(ER_SUCCESS), _non_blocking(false), _mutex()
{
    return ;
}

ft_http_server::~ft_http_server()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    this->_server_socket.close_socket();
    this->_non_blocking = false;
    this->set_error(ER_SUCCESS);
    ft_http_server::restore_errno(guard, entry_errno);
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
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_http_server::restore_errno(guard, entry_errno);
        return (1);
    }

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
        ft_http_server::restore_errno(guard, entry_errno);
        return (1);
    }
    this->set_error(ER_SUCCESS);
    this->_non_blocking = non_blocking;
    ft_http_server::restore_errno(guard, entry_errno);
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

void ft_http_server::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int operation_errno;

    operation_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (operation_errno != ER_SUCCESS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

static char http_server_to_lower(char character)
{
    if (character >= 'A' && character <= 'Z')
        return (static_cast<char>(character - 'A' + 'a'));
    return (character);
}

static bool http_server_token_equals(const char *token_start, size_t token_length, const char *token)
{
    size_t index;

    index = 0;
    while (index < token_length && token[index] != '\0')
    {
        char lhs;
        char rhs;

        lhs = http_server_to_lower(token_start[index]);
        rhs = http_server_to_lower(token[index]);
        if (lhs != rhs)
            return (false);
        index++;
    }
    if (index != token_length || token[index] != '\0')
        return (false);
    return (true);
}

static bool http_server_request_wants_keep_alive(const ft_string &request, size_t header_length)
{
    const char *data;
    const char *connection_header;
    const char *value_pointer;
    bool keep_alive;
    size_t header_limit;

    data = request.c_str();
    header_limit = header_length;
    if (header_limit > request.size())
        header_limit = request.size();
    keep_alive = true;
    if (ft_strnstr(data, "HTTP/1.0", header_limit) != ft_nullptr)
        keep_alive = false;
    connection_header = ft_strnstr(data, "Connection:", header_limit);
    if (connection_header == ft_nullptr)
        connection_header = ft_strnstr(data, "connection:", header_limit);
    if (connection_header == ft_nullptr)
        return (keep_alive);
    value_pointer = connection_header;
    while (*value_pointer != '\0' && value_pointer < data + header_limit && *value_pointer != ':')
        value_pointer++;
    if (*value_pointer != ':')
        return (keep_alive);
    value_pointer++;
    while (*value_pointer == ' ' || *value_pointer == '\t')
        value_pointer++;
    while (value_pointer < data + header_limit
        && *value_pointer != '\0' && *value_pointer != '\r'
        && *value_pointer != '\n')
    {
        const char *token_start;
        size_t token_length;

        token_start = value_pointer;
        while (value_pointer < data + header_limit
            && *value_pointer != '\0' && *value_pointer != '\r'
            && *value_pointer != '\n' && *value_pointer != ','
            && *value_pointer != ';')
        {
            value_pointer++;
        }
        token_length = static_cast<size_t>(value_pointer - token_start);
        while (token_length > 0
            && (token_start[token_length - 1] == ' '
                || token_start[token_length - 1] == '\t'))
        {
            token_length--;
        }
        if (token_length > 0)
        {
            if (http_server_token_equals(token_start, token_length, "close") != false)
                keep_alive = false;
            if (http_server_token_equals(token_start, token_length, "keep-alive") != false)
                keep_alive = true;
        }
        while (*value_pointer == ' ' || *value_pointer == '\t'
            || *value_pointer == ',' || *value_pointer == ';')
        {
            value_pointer++;
        }
    }
    return (keep_alive);
}


int ft_http_server::run_once()
{
    int entry_errno;
    int result;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_http_server::restore_errno(guard, entry_errno);
        return (1);
    }
    result = this->run_once_locked(guard);
    ft_http_server::restore_errno(guard, entry_errno);
    return (result);
}

int ft_http_server::run_once_locked(ft_unique_lock<pt_mutex> &guard)
{
    t_monotonic_time_point start_time;
    struct sockaddr_storage client_address;
    socklen_t address_length;
    int client_socket;
    int server_fd;
    bool connection_active;
    int overall_result;
    int processed_requests;
    int last_error_code;
    const int max_keep_alive_requests = 100;

    if (guard.owns_lock() == false)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (1);
    }
    start_time = time_monotonic_point_now();
    server_fd = this->_server_socket.get_fd();
    if (server_fd < 0)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (1);
    }
    address_length = sizeof(client_address);
    client_socket = nw_accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_address), &address_length);
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
    connection_active = true;
    overall_result = 0;
    processed_requests = 0;
    last_error_code = ER_SUCCESS;
    ft_string pending_data;
    while (connection_active != false)
    {
        t_monotonic_time_point request_start_time;
        ft_string request;
        ft_string body;
        ft_string response;
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
        bool is_post;
        int parse_error;
        bool request_failed;
        int request_result;
        bool should_keep_alive;
        char buffer[1024];
        ssize_t bytes_received;

        request_start_time = time_monotonic_point_now();
        header_complete = false;
        request_complete = false;
        has_content_length = false;
        expected_body_length = 0;
        header_length = 0;
        metrics_enabled = true;
        method_label = "UNKNOWN";
        request_bytes = 0;
        response_bytes = 0;
        status_code_value = 0;
        is_post = false;
        parse_error = ER_SUCCESS;
        request_failed = false;
        request_result = 1;
        should_keep_alive = false;
        request = pending_data;
        pending_data.clear();
        while (request_complete == false)
        {
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
                            length_value_pointer++;
                        if (*length_value_pointer == '\0'
                            || ft_isdigit(static_cast<unsigned char>(*length_value_pointer)) == 0)
                        {
                            nw_close(client_socket);
                            this->set_error(FT_ERR_INVALID_ARGUMENT);
                            overall_result = 1;
                            last_error_code = this->_error_code;
                            request_failed = true;
                            break;
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
                                overall_result = 1;
                                last_error_code = this->_error_code;
                                request_failed = true;
                                break;
                            }
                            expected_body_length = expected_body_length * 10 + digit_value;
                            length_value_pointer++;
                        }
                        if (request_failed != false)
                            break;
                        if (has_length_digits == false)
                        {
                            nw_close(client_socket);
                            this->set_error(FT_ERR_INVALID_ARGUMENT);
                            overall_result = 1;
                            last_error_code = this->_error_code;
                            request_failed = true;
                            break;
                        }
                        if (expected_body_length > max_request_size)
                        {
                            nw_close(client_socket);
                            this->set_error(FT_ERR_INVALID_ARGUMENT);
                            overall_result = 1;
                            last_error_code = this->_error_code;
                            request_failed = true;
                            break;
                        }
                        has_content_length = true;
                    }
                }
            }
            if (request_failed != false)
                break;
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
            if (request_complete != false)
                break;
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
                int last_error;

                last_error = errno;
                this->set_error(ft_map_system_error(last_error));
#endif
                overall_result = 1;
                last_error_code = this->_error_code;
                request_failed = true;
                break;
            }
            if (bytes_received == 0)
            {
                if (request.empty() != false && header_complete == false)
                {
                    nw_close(client_socket);
                    ft_errno = ER_SUCCESS;
                    this->_error_code = ER_SUCCESS;
                    return (0);
                }
                nw_close(client_socket);
                this->set_error(FT_ERR_SOCKET_RECEIVE_FAILED);
                overall_result = 1;
                last_error_code = this->_error_code;
                request_failed = true;
                break;
            }
            buffer[bytes_received] = '\0';
            request.append(buffer);
            if (request.size() > max_request_size)
            {
                nw_close(client_socket);
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                overall_result = 1;
                last_error_code = this->_error_code;
                request_failed = true;
                break;
            }
        }
        if (request_failed != false)
        {
            if (metrics_enabled != false)
                http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, request_result, request_start_time);
            connection_active = false;
            break;
        }
        size_t consumed_length;
        ft_string current_request;

        consumed_length = header_length;
        if (has_content_length != false)
            consumed_length += expected_body_length;
        if (request.size() > consumed_length)
            pending_data.assign(request.c_str() + consumed_length, request.size() - consumed_length);
        else
            pending_data.clear();
        current_request.assign(request.c_str(), consumed_length);
        request_bytes = consumed_length;
        parse_error = parse_request(current_request, body, is_post);
        if (parse_error != ER_SUCCESS)
        {
            nw_close(client_socket);
            this->set_error(parse_error);
            overall_result = 1;
            last_error_code = this->_error_code;
            if (metrics_enabled != false)
                http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, 1, request_start_time);
            connection_active = false;
            break;
        }
        if (is_post)
            method_label = "POST";
        else
            method_label = "GET";
        should_keep_alive = http_server_request_wants_keep_alive(current_request, header_length);
        if (processed_requests >= max_keep_alive_requests - 1)
            should_keep_alive = false;
        if (is_post && !body.empty())
        {
            std::snprintf(buffer, sizeof(buffer), "%zu", body.size());
            response.append("HTTP/1.1 200 OK\r\nContent-Length: ");
            response.append(buffer);
            if (should_keep_alive != false)
                response.append("\r\nConnection: keep-alive\r\n\r\n");
            else
                response.append("\r\nConnection: close\r\n\r\n");
            response.append(body);
        }
        else if (!is_post)
        {
            response.append("HTTP/1.1 200 OK\r\nContent-Length: 3\r\n");
            if (should_keep_alive != false)
                response.append("Connection: keep-alive\r\n\r\n");
            else
                response.append("Connection: close\r\n\r\n");
            response.append("GET");
        }
        else
        {
            response.append("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n");
            if (should_keep_alive != false)
                response.append("Connection: keep-alive\r\n\r\n");
            else
                response.append("Connection: close\r\n\r\n");
        }
        const char *response_data;
        size_t total_sent;
        ssize_t send_result;

        response_data = response.c_str();
        total_sent = 0;
        response_bytes = response.size();
        bool remote_closed_during_send;

        remote_closed_during_send = false;
        while (total_sent < response.size())
        {
            send_result = nw_send(client_socket, response_data + total_sent, response.size() - total_sent, 0);
            if (send_result <= 0)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (send_result == 0
                    || (send_result < 0
                        && (last_error == WSAECONNRESET || last_error == WSAECONNABORTED
                            || last_error == WSAESHUTDOWN)))
                {
                    ft_errno = ER_SUCCESS;
                    this->_error_code = ER_SUCCESS;
                    remote_closed_during_send = true;
                    break;
                }
#else
                int last_error;

                last_error = errno;
                if (send_result == 0
                    || (send_result < 0 && (last_error == EPIPE || last_error == ECONNRESET)))
                {
                    ft_errno = ER_SUCCESS;
                    this->_error_code = ER_SUCCESS;
                    remote_closed_during_send = true;
                    break;
                }
#endif
                nw_close(client_socket);
                if (send_result < 0)
                    this->set_error(ft_map_system_error(last_error));
                else
                    this->set_error(FT_ERR_SOCKET_SEND_FAILED);
                overall_result = 1;
                last_error_code = this->_error_code;
                request_result = 1;
                if (metrics_enabled != false)
                    http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, request_result, request_start_time);
                connection_active = false;
                break;
            }
            total_sent += static_cast<size_t>(send_result);
        }
        if (remote_closed_during_send != false)
            should_keep_alive = false;
        else if (connection_active == false)
            break;
        if (networking_check_socket_after_send(client_socket) != 0)
        {
            int post_send_error;

            post_send_error = ft_errno;
            if (post_send_error == FT_ERR_SOCKET_SEND_FAILED)
            {
                ft_errno = ER_SUCCESS;
                this->_error_code = ER_SUCCESS;
                should_keep_alive = false;
            }
            else
            {
                nw_close(client_socket);
                this->set_error(post_send_error);
                overall_result = 1;
                last_error_code = this->_error_code;
                request_result = 1;
                if (metrics_enabled != false)
                    http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, request_result, request_start_time);
                connection_active = false;
                break;
            }
        }
        ft_errno = ER_SUCCESS;
        this->_error_code = ER_SUCCESS;
        status_code_value = 200;
        request_result = 0;
        if (metrics_enabled != false)
            http_server_record_metrics(method_label, request_bytes, response_bytes, status_code_value, request_result, request_start_time);
        processed_requests++;
        if (should_keep_alive == false)
            connection_active = false;
    }
    nw_close(client_socket);
    if (overall_result == 0)
    {
        ft_errno = ER_SUCCESS;
        this->_error_code = ER_SUCCESS;
        return (0);
    }
    this->_error_code = last_error_code;
    return (1);
}

int ft_http_server::get_error() const
{
    int entry_errno;
    int error_value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_http_server::restore_errno(guard, entry_errno);
        return (this->_error_code);
    }
    error_value = this->_error_code;
    ft_http_server::restore_errno(guard, entry_errno);
    return (error_value);
}

const char *ft_http_server::get_error_str() const
{
    int entry_errno;
    int error_value;
    const char *error_string;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const char *guard_error_string;

        guard_error_string = ft_strerror(guard.get_error());
        ft_http_server::restore_errno(guard, entry_errno);
        return (guard_error_string);
    }
    error_value = this->_error_code;
    error_string = ft_strerror(error_value);
    ft_http_server::restore_errno(guard, entry_errno);
    return (error_string);
}

