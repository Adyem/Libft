#include "http_server.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <cstring>
#include <cstdio>
#include <cerrno>

ft_http_server::ft_http_server()
    : _server_socket(), _error_code(ER_SUCCESS)
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
    this->_error_code = ER_SUCCESS;
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
        return (FT_EINVAL);
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

int ft_http_server::run_once()
{
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

    address_length = sizeof(client_address);
    client_socket = nw_accept(this->_server_socket.get_fd(), reinterpret_cast<struct sockaddr*>(&client_address), &address_length);
    if (client_socket < 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    bytes_received = nw_recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0)
    {
        FT_CLOSE_SOCKET(client_socket);
        this->set_error(errno + ERRNO_OFFSET);
        return (1);
    }
    buffer[bytes_received] = '\0';
    request = buffer;
    parse_error = parse_request(request, body, is_post);
    if (parse_error != ER_SUCCESS)
    {
        FT_CLOSE_SOCKET(client_socket);
        this->set_error(parse_error);
        return (1);
    }
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
    nw_send(client_socket, response.c_str(), response.size(), 0);
    FT_CLOSE_SOCKET(client_socket);
    this->_error_code = ER_SUCCESS;
    return (0);
}

int ft_http_server::get_error() const
{
    return (this->_error_code);
}

const char *ft_http_server::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
