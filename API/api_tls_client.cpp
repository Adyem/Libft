#include "tls_client.hpp"
#include "api_internal.hpp"
#include "../Printf/printf.hpp"
#include "../Networking/socket_class.hpp"
#include "../Networking/ssl_wrapper.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../Template/move.hpp"
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
#include <openssl/err.h>
#include <openssl/x509v3.h>

static ssize_t ssl_send_all(SSL *ssl, const void *data, size_t size)
{
    size_t total = 0;
    const char *ptr = static_cast<const char*>(data);
    while (total < size)
    {
        ssize_t sent = nw_ssl_write(ssl, ptr + total, size - total);
        if (sent <= 0)
            return (-1);
        total += sent;
    }
    return (static_cast<ssize_t>(total));
}

api_tls_client::api_tls_client(const char *host_c, uint16_t port, int timeout_ms)
: _ctx(ft_nullptr), _ssl(ft_nullptr), _sock(-1), _host(""), _timeout(timeout_ms), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    if (!host_c)
    {
        this->set_error(FT_EINVAL);
        return ;
    }
    this->_host = host_c;
    if (!OPENSSL_init_ssl(0, ft_nullptr))
    {
        this->set_error(FT_ETERM);
        return ;
    }

    this->_ctx = SSL_CTX_new(TLS_client_method());
    if (!this->_ctx)
    {
        this->set_error(FT_EALLOC);
        return ;
    }
    if (SSL_CTX_set_default_verify_paths(this->_ctx) != 1)
    {
        SSL_CTX_free(this->_ctx);
        this->_ctx = ft_nullptr;
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return ;
    }
    SSL_CTX_set_verify(this->_ctx, SSL_VERIFY_PEER, ft_nullptr);

    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    char port_string[6];
    pf_snprintf(port_string, sizeof(port_string), "%u", port);
    if (getaddrinfo(host_c, port_string, &hints, &address_results) != 0)
    {
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return ;
    }

    address_info = address_results;
    while (address_info != ft_nullptr)
    {
        this->_sock = nw_socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
        if (this->_sock >= 0)
        {
            if (timeout_ms > 0)
            {
                struct timeval time_value;
                time_value.tv_sec = timeout_ms / 1000;
                time_value.tv_usec = (timeout_ms % 1000) * 1000;
                setsockopt(this->_sock, SOL_SOCKET, SO_RCVTIMEO, &time_value, sizeof(time_value));
                setsockopt(this->_sock, SOL_SOCKET, SO_SNDTIMEO, &time_value, sizeof(time_value));
            }
            if (nw_connect(this->_sock, address_info->ai_addr, static_cast<socklen_t>(address_info->ai_addrlen)) == 0)
                break;
            FT_CLOSE_SOCKET(this->_sock);
            this->_sock = -1;
        }
        address_info = address_info->ai_next;
    }
    if (address_results)
        freeaddrinfo(address_results);
    if (this->_sock < 0)
    {
        this->set_error(SOCKET_CONNECT_FAILED);
        return ;
    }

    this->_ssl = SSL_new(this->_ctx);
    if (!this->_ssl)
    {
        this->set_error(FT_EALLOC);
        return ;
    }
    if (SSL_set1_host(this->_ssl, this->_host.c_str()) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        FT_CLOSE_SOCKET(this->_sock);
        this->_sock = -1;
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return ;
    }
    SSL_set_hostflags(this->_ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    void *host_name_argument;

    host_name_argument = static_cast<void*>(const_cast<char*>(this->_host.c_str()));
    if (SSL_ctrl(this->_ssl, SSL_CTRL_SET_TLSEXT_HOSTNAME, TLSEXT_NAMETYPE_host_name, host_name_argument) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        FT_CLOSE_SOCKET(this->_sock);
        this->_sock = -1;
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return ;
    }
    if (SSL_set_fd(this->_ssl, this->_sock) != 1)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        FT_CLOSE_SOCKET(this->_sock);
        this->_sock = -1;
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return ;
    }
    if (SSL_connect(this->_ssl) <= 0)
    {
        SSL_free(this->_ssl);
        this->_ssl = ft_nullptr;
        FT_CLOSE_SOCKET(this->_sock);
        this->_sock = -1;
        this->set_error(SOCKET_CONNECT_FAILED);
        return ;
    }
    this->set_error(ER_SUCCESS);
}

api_tls_client::~api_tls_client()
{
    size_t worker_index;

    worker_index = 0;
    while (worker_index < this->_async_workers.size())
    {
        if (this->_async_workers[worker_index].joinable())
            this->_async_workers[worker_index].join();
        worker_index += 1;
    }
    this->_async_workers.clear();
    if (this->_ssl)
    {
        SSL_shutdown(this->_ssl);
        SSL_free(this->_ssl);
    }
    if (this->_sock >= 0)
        FT_CLOSE_SOCKET(this->_sock);
    if (this->_ctx)
        SSL_CTX_free(this->_ctx);
}

bool api_tls_client::is_valid() const
{
    if (this->_ssl != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (true);
    }
    this->set_error(SOCKET_INVALID_CONFIGURATION);
    return (false);
}

char *api_tls_client::request(const char *method, const char *path, json_group *payload,
                              const char *headers, int *status)
{
    if (!method || !path)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    if (this->_ssl == ft_nullptr)
    {
        this->set_error(SOCKET_INVALID_CONFIGURATION);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    if (ft_log_get_api_logging())
    {
        const char *log_method = "(null)";
        const char *log_path = "(null)";
        if (method)
            log_method = method;
        if (path)
            log_path = path;
        ft_log_debug("api_tls_client::request %s %s",
            log_method, log_path);
    }

    ft_string request(method);
    request += " ";
    request += path;
    request += " HTTP/1.1\r\nHost: ";
    request += this->_host.c_str();
    if (headers && headers[0])
    {
        request += "\r\n";
        request += headers;
    }

    ft_string body_string;
    if (payload)
    {
        char *temporary_string = json_write_to_string(payload);
        if (!temporary_string)
        {
            int json_error = ft_errno;
            if (json_error == ER_SUCCESS)
                json_error = JSON_MALLOC_FAIL;
            this->set_error(json_error);
            return (ft_nullptr);
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        if (!api_append_content_length_header(request, body_string.size()))
        {
            this->set_error(FT_EIO);
            return (ft_nullptr);
        }
    }
    request += "\r\nConnection: keep-alive\r\n\r\n";
    if (payload)
        request += body_string.c_str();

    if (ssl_send_all(this->_ssl, request.c_str(), request.size()) < 0)
    {
        this->set_error(SOCKET_SEND_FAILED);
        return (ft_nullptr);
    }

    ft_string response;
    char buffer[1024];
    ssize_t bytes_received;
    const char *header_end_ptr = ft_nullptr;

    while (!header_end_ptr)
    {
        bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0)
        {
            this->set_error(SOCKET_RECEIVE_FAILED);
            return (ft_nullptr);
        }
        buffer[bytes_received] = '\0';
        response += buffer;
        header_end_ptr = ft_strstr(response.c_str(), "\r\n\r\n");
    }

    if (status)
    {
        *status = -1;
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            *status = ft_atoi(space + 1);
    }

    size_t header_len = static_cast<size_t>(header_end_ptr - response.c_str()) + 4;
    size_t content_length = 0;
    bool has_content_length = false;
    bool is_chunked = false;
    ft_string headers_section;
    headers_section = response.substr(0, header_len);
    size_t line_offset = 0;
    while (line_offset < headers_section.size())
    {
        size_t line_end = headers_section.find("\r\n", line_offset);
        if (line_end == ft_string::npos)
            break;
        ft_string header_line;
        header_line = headers_section.substr(line_offset, line_end - line_offset);
        line_offset = line_end + 2;
        if (header_line.empty())
            continue;
        size_t colon_index = header_line.find(':');
        if (colon_index == ft_string::npos)
            continue;
        ft_string header_name;
        header_name = header_line.substr(0, colon_index);
        while (!header_name.empty())
        {
            char last_char;
            last_char = header_name[header_name.size() - 1];
            if (last_char != ' ' && last_char != '\t')
                break;
            header_name.erase(header_name.size() - 1, 1);
        }
        ft_string header_value;
        header_value = header_line.substr(colon_index + 1);
        size_t value_trim_index = 0;
        while (value_trim_index < header_value.size() &&
               (header_value[value_trim_index] == ' ' || header_value[value_trim_index] == '\t'))
        {
            header_value.erase(value_trim_index, 1);
        }
        size_t value_end_index = header_value.size();
        while (value_end_index > 0)
        {
            char trim_char;
            trim_char = header_value[value_end_index - 1];
            if (trim_char != ' ' && trim_char != '\t')
                break;
            header_value.erase(value_end_index - 1, 1);
            value_end_index--;
        }
        size_t name_index = 0;
        while (name_index < header_name.size())
        {
            char current_char;
            current_char = header_name[name_index];
            if (current_char >= 'A' && current_char <= 'Z')
                header_name[name_index] = static_cast<char>(current_char + 32);
            name_index++;
        }
        if (header_name == "content-length" && !has_content_length)
        {
            ft_errno = ER_SUCCESS;
            unsigned long parsed_length;
            parsed_length = ft_strtoul(header_value.c_str(), ft_nullptr, 10);
            if (ft_errno == ER_SUCCESS)
            {
                content_length = static_cast<size_t>(parsed_length);
                has_content_length = true;
            }
        }
        if (header_name == "transfer-encoding")
        {
            ft_string lowered_value;
            lowered_value = header_value;
            size_t lower_index = 0;
            while (lower_index < lowered_value.size())
            {
                char lowered_char;
                lowered_char = lowered_value[lower_index];
                if (lowered_char >= 'A' && lowered_char <= 'Z')
                    lowered_value[lower_index] = static_cast<char>(lowered_char + 32);
                lower_index++;
            }
            size_t token_start = 0;
            while (token_start <= lowered_value.size())
            {
                size_t token_end = token_start;
                while (token_end < lowered_value.size() && lowered_value[token_end] != ',')
                    token_end++;
                ft_string token_value;
                token_value = lowered_value.substr(token_start, token_end - token_start);
                while (!token_value.empty() && (token_value[0] == ' ' || token_value[0] == '\t'))
                {
                    token_value.erase(0, 1);
                }
                size_t token_trim = token_value.size();
                while (token_trim > 0)
                {
                    char token_char;
                    token_char = token_value[token_trim - 1];
                    if (token_char != ' ' && token_char != '\t')
                        break;
                    token_value.erase(token_trim - 1, 1);
                    token_trim--;
                }
                if (token_value == "chunked")
                    is_chunked = true;
                if (token_end == lowered_value.size())
                    break;
                token_start = token_end + 1;
            }
        }
    }

    ft_string body;
    body += response.c_str() + header_len;

    if (is_chunked)
    {
        ft_string chunk_buffer;
        chunk_buffer += body.c_str();
        body.clear();
        size_t parse_offset = 0;
        while (true)
        {
            const char *chunk_start = chunk_buffer.c_str() + parse_offset;
            const char *line_end = ft_strstr(chunk_start, "\r\n");
            while (!line_end)
            {
                bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                if (bytes_received <= 0)
                {
                    this->set_error(SOCKET_RECEIVE_FAILED);
                    return (ft_nullptr);
                }
                buffer[bytes_received] = '\0';
                chunk_buffer += buffer;
                chunk_start = chunk_buffer.c_str() + parse_offset;
                line_end = ft_strstr(chunk_start, "\r\n");
            }
            size_t line_length = static_cast<size_t>(line_end - chunk_start);
            size_t index = 0;
            while (index < line_length && (chunk_start[index] == ' ' || chunk_start[index] == '\t'))
            {
                index++;
            }
            ft_string size_string;
            while (index < line_length && chunk_start[index] != ';')
            {
                size_string.append(chunk_start[index]);
                index++;
            }
            size_t size_trim = size_string.size();
            while (size_trim > 0)
            {
                const char *size_cstr = size_string.c_str();
                if (size_cstr[size_trim - 1] != ' ' && size_cstr[size_trim - 1] != '\t')
                    break;
                size_string.erase(size_trim - 1, 1);
                size_trim--;
            }
            ft_errno = ER_SUCCESS;
            unsigned long chunk_length = ft_strtoul(size_string.c_str(), ft_nullptr, 16);
            int chunk_error = ft_errno;
            if (chunk_error != ER_SUCCESS)
            {
                this->set_error(chunk_error);
                return (ft_nullptr);
            }
            parse_offset += line_length + 2;
            while (chunk_buffer.size() < parse_offset + chunk_length + 2)
            {
                bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                if (bytes_received <= 0)
                {
                    this->set_error(SOCKET_RECEIVE_FAILED);
                    return (ft_nullptr);
                }
                buffer[bytes_received] = '\0';
                chunk_buffer += buffer;
            }
            const char *data_ptr = chunk_buffer.c_str() + parse_offset;
            size_t copy_index = 0;
            while (copy_index < chunk_length)
            {
                body.append(data_ptr[copy_index]);
                copy_index++;
            }
            parse_offset += chunk_length;
            if (chunk_buffer.size() < parse_offset + 2)
            {
                while (chunk_buffer.size() < parse_offset + 2)
                {
                    bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                    if (bytes_received <= 0)
                    {
                        this->set_error(SOCKET_RECEIVE_FAILED);
                        return (ft_nullptr);
                    }
                    buffer[bytes_received] = '\0';
                    chunk_buffer += buffer;
                }
            }
            if (chunk_length == 0)
            {
                bool trailers_complete = false;
                while (!trailers_complete)
                {
                    const char *trailer_ptr = chunk_buffer.c_str() + parse_offset;
                    const char *trailer_end = ft_strstr(trailer_ptr, "\r\n");
                    while (!trailer_end)
                    {
                        bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
                        if (bytes_received <= 0)
                        {
                            this->set_error(SOCKET_RECEIVE_FAILED);
                            return (ft_nullptr);
                        }
                        buffer[bytes_received] = '\0';
                        chunk_buffer += buffer;
                        trailer_ptr = chunk_buffer.c_str() + parse_offset;
                        trailer_end = ft_strstr(trailer_ptr, "\r\n");
                    }
                    size_t trailer_length = static_cast<size_t>(trailer_end - trailer_ptr);
                    parse_offset += trailer_length + 2;
                    if (trailer_length == 0)
                        trailers_complete = true;
                }
                break;
            }
            parse_offset += 2;
        }
    }
    else if (has_content_length)
    {
        while (body.size() < content_length)
        {
            bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1);
            if (bytes_received <= 0)
            {
                this->set_error(SOCKET_RECEIVE_FAILED);
                return (ft_nullptr);
            }
            buffer[bytes_received] = '\0';
            body += buffer;
        }
        if (body.size() > content_length)
            body.erase(content_length, body.size() - content_length);
    }
    else
    {
        while ((bytes_received = nw_ssl_read(this->_ssl, buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytes_received] = '\0';
            body += buffer;
        }
        if (bytes_received < 0)
        {
            this->set_error(SOCKET_RECEIVE_FAILED);
            return (ft_nullptr);
        }
    }

    char *result_body = cma_strdup(body.c_str());
    if (!result_body)
    {
        this->set_error(CMA_BAD_ALLOC);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (result_body);
}

json_group *api_tls_client::request_json(const char *method, const char *path,
                                         json_group *payload,
                                         const char *headers, int *status)
{
    char *body = this->request(method, path, payload, headers, status);
    if (!body)
        return (ft_nullptr);
    json_group *result = json_read_from_string(body);
    cma_free(body);
    if (!result)
    {
        int json_error = ft_errno;
        if (json_error == ER_SUCCESS)
            json_error = JSON_MALLOC_FAIL;
        this->set_error(json_error);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (result);
}

bool api_tls_client::request_async(const char *method, const char *path,
                                   json_group *payload,
                                   const char *headers,
                                   api_callback callback,
                                   void *user_data)
{
    if (!callback)
    {
        this->set_error(FT_EINVAL);
        return (false);
    }
    ft_thread worker([this, method, path, payload, headers, callback, user_data]()
    {
        int status_local = -1;
        char *result_body = this->request(method, path, payload, headers, &status_local);
        callback(result_body, status_local, user_data);
    });
    if (worker.get_error() != ER_SUCCESS)
    {
        this->set_error(worker.get_error());
        return (false);
    }
    size_t worker_count_before;
    size_t worker_count_after;
    int vector_error;

    worker_count_before = this->_async_workers.size();
    this->_async_workers.push_back(ft_move(worker));
    worker_count_after = this->_async_workers.size();
    if (worker_count_after < worker_count_before + 1)
    {
        if (worker.joinable())
            worker.join();
        vector_error = this->_async_workers.get_error();
        if (vector_error == ER_SUCCESS)
            vector_error = VECTOR_ALLOC_FAIL;
        this->set_error(vector_error);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

int api_tls_client::get_error() const noexcept
{
    return (this->_error_code);
}

const char *api_tls_client::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void api_tls_client::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

