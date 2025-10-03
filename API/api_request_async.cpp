#include "api.hpp"
#include "api_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/thread.hpp"
#include <errno.h>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/select.h>
#endif

static void api_request_set_resolve_error(int resolver_status)
{
#ifdef EAI_BADFLAGS
    if (resolver_status == EAI_BADFLAGS)
    {
        ft_errno = SOCKET_RESOLVE_BAD_FLAGS;
        return ;
    }
#endif
#ifdef EAI_AGAIN
    if (resolver_status == EAI_AGAIN)
    {
        ft_errno = SOCKET_RESOLVE_AGAIN;
        return ;
    }
#endif
#ifdef EAI_FAIL
    if (resolver_status == EAI_FAIL)
    {
        ft_errno = SOCKET_RESOLVE_FAIL;
        return ;
    }
#endif
#ifdef EAI_FAMILY
    if (resolver_status == EAI_FAMILY)
    {
        ft_errno = SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_ADDRFAMILY
    if (resolver_status == EAI_ADDRFAMILY)
    {
        ft_errno = SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_SOCKTYPE
    if (resolver_status == EAI_SOCKTYPE)
    {
        ft_errno = SOCKET_RESOLVE_SOCKTYPE;
        return ;
    }
#endif
#ifdef EAI_SERVICE
    if (resolver_status == EAI_SERVICE)
    {
        ft_errno = SOCKET_RESOLVE_SERVICE;
        return ;
    }
#endif
#ifdef EAI_MEMORY
    if (resolver_status == EAI_MEMORY)
    {
        ft_errno = SOCKET_RESOLVE_MEMORY;
        return ;
    }
#endif
#ifdef EAI_NONAME
    if (resolver_status == EAI_NONAME)
    {
        ft_errno = SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_NODATA
    if (resolver_status == EAI_NODATA)
    {
        ft_errno = SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_OVERFLOW
    if (resolver_status == EAI_OVERFLOW)
    {
        ft_errno = SOCKET_RESOLVE_OVERFLOW;
        return ;
    }
#endif
#ifdef EAI_SYSTEM
    if (resolver_status == EAI_SYSTEM)
    {
#ifdef _WIN32
        ft_errno = WSAGetLastError() + ERRNO_OFFSET;
#else
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = SOCKET_RESOLVE_FAIL;
#endif
        return ;
    }
#endif
    ft_errno = SOCKET_RESOLVE_FAILED;
    return ;
}

struct api_async_request
{
    char *ip;
    uint16_t port;
    char *method;
    char *path;
    json_group *payload;
    char *headers;
    int timeout;
    api_callback callback;
    void *user_data;
};

static void api_set_timeval(struct timeval *time_value, int timeout_ms)
{
    if (!time_value)
        return ;
    if (timeout_ms < 0)
        timeout_ms = 0;
    time_value->tv_sec = timeout_ms / 1000;
    time_value->tv_usec = (timeout_ms % 1000) * 1000;
    return ;
}

static void api_async_worker(api_async_request *data)
{
    int socket_fd = -1;
    struct addrinfo hints;
    struct addrinfo *address_results = ft_nullptr;
    struct addrinfo *address_info;
    ft_string request;
    ft_string body_string;
    ft_string response;
    char port_string[6];
    char *result_body = ft_nullptr;
    int status = -1;
    char buffer[1024];
    fd_set read_set;
    fd_set write_set;
    struct timeval tv;
    int timeout_ms;
    size_t total_sent;
    int resolver_status;

    if (!data || !data->ip || !data->method || !data->path)
    {
        ft_errno = FT_EINVAL;
        goto cleanup;
    }
    timeout_ms = data->timeout;
    ft_bzero(&hints, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    pf_snprintf(port_string, sizeof(port_string), "%u", data->port);
    resolver_status = getaddrinfo(data->ip, port_string, &hints, &address_results);
    if (resolver_status != 0)
    {
        api_request_set_resolve_error(resolver_status);
        goto cleanup;
    }
    address_info = address_results;
    while (address_info != ft_nullptr && socket_fd < 0)
    {
        int connect_attempt;
        int max_attempts;

        connect_attempt = 0;
        max_attempts = 5;
        while (connect_attempt < max_attempts && socket_fd < 0)
        {
            int candidate_fd;
            bool in_progress;

            candidate_fd = nw_socket(address_info->ai_family,
                                     address_info->ai_socktype,
                                     address_info->ai_protocol);
            if (candidate_fd < 0)
                break;
#ifdef _WIN32
            u_long mode = 1;
            ioctlsocket(candidate_fd, FIONBIO, &mode);
#else
            fcntl(candidate_fd, F_SETFL, O_NONBLOCK);
#endif
            if (nw_connect(candidate_fd, address_info->ai_addr,
                           static_cast<socklen_t>(address_info->ai_addrlen)) >= 0)
            {
                socket_fd = candidate_fd;
                ft_errno = ER_SUCCESS;
                break;
            }
#ifdef _WIN32
            int connect_error;

            connect_error = WSAGetLastError();
            in_progress = false;
            if (connect_error == WSAEINPROGRESS || connect_error == WSAEWOULDBLOCK)
                in_progress = true;
            if (connect_error == WSAECONNREFUSED)
            {
                FT_CLOSE_SOCKET(candidate_fd);
                connect_attempt += 1;
                pt_thread_sleep(100);
                continue;
            }
            if (!in_progress)
            {
                WSASetLastError(connect_error);
                FT_CLOSE_SOCKET(candidate_fd);
                break;
            }
#else
            in_progress = false;
            if (errno == EINPROGRESS)
                in_progress = true;
            if (errno == ECONNREFUSED)
            {
                FT_CLOSE_SOCKET(candidate_fd);
                connect_attempt += 1;
                pt_thread_sleep(100);
                continue;
            }
            if (!in_progress)
            {
                FT_CLOSE_SOCKET(candidate_fd);
                break;
            }
#endif
            FD_ZERO(&write_set);
            FD_SET(candidate_fd, &write_set);
            api_set_timeval(&tv, timeout_ms);
            if (select(candidate_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
            {
#ifdef _WIN32
                int select_error;

                select_error = WSAGetLastError();
                if (select_error != 0)
                    ft_errno = select_error + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
#else
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
#endif
                FT_CLOSE_SOCKET(candidate_fd);
                connect_attempt += 1;
                pt_thread_sleep(100);
                continue;
            }
            int socket_error;
#ifdef _WIN32
            int socket_error_length;

            socket_error_length = sizeof(socket_error);
            if (getsockopt(candidate_fd, SOL_SOCKET, SO_ERROR,
                    reinterpret_cast<char *>(&socket_error), &socket_error_length) < 0)
#else
            socklen_t socket_error_length;

            socket_error_length = sizeof(socket_error);
            if (getsockopt(candidate_fd, SOL_SOCKET, SO_ERROR,
                    &socket_error, &socket_error_length) < 0)
#endif
            {
#ifdef _WIN32
                int getsockopt_error;

                getsockopt_error = WSAGetLastError();
                if (getsockopt_error != 0)
                    ft_errno = getsockopt_error + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
#else
                if (errno != 0)
                    ft_errno = errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_CONNECT_FAILED;
#endif
                FT_CLOSE_SOCKET(candidate_fd);
                connect_attempt += 1;
                pt_thread_sleep(100);
                continue;
            }
#ifdef _WIN32
            if (socket_error != 0)
            {
                if (socket_error == WSAECONNREFUSED)
                {
                    FT_CLOSE_SOCKET(candidate_fd);
                    connect_attempt += 1;
                    pt_thread_sleep(100);
                    continue;
                }
                WSASetLastError(socket_error);
                FT_CLOSE_SOCKET(candidate_fd);
                break;
            }
#else
            if (socket_error != 0)
            {
                if (socket_error == ECONNREFUSED)
                {
                    FT_CLOSE_SOCKET(candidate_fd);
                    connect_attempt += 1;
                    pt_thread_sleep(100);
                    continue;
                }
                errno = socket_error;
                FT_CLOSE_SOCKET(candidate_fd);
                break;
            }
#endif
            socket_fd = candidate_fd;
            ft_errno = ER_SUCCESS;
        }
        if (socket_fd < 0)
            address_info = address_info->ai_next;
    }
    if (socket_fd < 0)
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        goto cleanup;
    }

    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    api_set_timeval(&tv, timeout_ms);
    if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        goto cleanup;
    }

    request = data->method;
    request += " ";
    request += data->path;
    request += " HTTP/1.1\r\nHost: ";
    request += data->ip;
    if (data->headers && data->headers[0])
    {
        request += "\r\n";
        request += data->headers;
    }
    if (data->payload)
    {
        char *temporary_string = json_write_to_string(data->payload);
        if (!temporary_string)
        {
            if (ft_errno == ER_SUCCESS)
                ft_errno = FT_EALLOC;
            goto cleanup;
        }
        body_string = temporary_string;
        cma_free(temporary_string);
        request += "\r\nContent-Type: application/json";
        if (!api_append_content_length_header(request, body_string.size()))
        {
            ft_errno = FT_EIO;
            goto cleanup;
        }
    }
    request += "\r\nConnection: close\r\n\r\n";
    if (data->payload)
        request += body_string.c_str();

    total_sent = 0;
    while (total_sent < request.size())
    {
        ssize_t bytes_sent = nw_send(socket_fd,
                                     request.c_str() + total_sent,
                                     request.size() - total_sent,
                                     0);
        if (bytes_sent < 0)
        {
#ifdef _WIN32
            int send_err = WSAGetLastError();
            if (send_err == WSAEWOULDBLOCK)
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN)
#endif
            {
                FD_ZERO(&write_set);
                FD_SET(socket_fd, &write_set);
                api_set_timeval(&tv, timeout_ms);
                if (select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv) <= 0)
                {
                    if (errno != 0)
                        ft_errno = errno + ERRNO_OFFSET;
                    else if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
                    goto cleanup;
                }
                continue;
            }
            goto cleanup;
        }
        total_sent += static_cast<size_t>(bytes_sent);
    }

    while (true)
    {
        FD_ZERO(&read_set);
        FD_SET(socket_fd, &read_set);
        api_set_timeval(&tv, timeout_ms);
        if (select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv) <= 0)
        {
            if (errno != 0)
                ft_errno = errno + ERRNO_OFFSET;
            else if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_RECEIVE_FAILED;
            break;
        }
        ssize_t bytes_received = nw_recv(socket_fd, buffer,
                                         sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
            break;
        buffer[bytes_received] = '\0';
        response += buffer;
    }

    if (response.size() > 0)
    {
        const char *space = ft_strchr(response.c_str(), ' ');
        if (space)
            status = ft_atoi(space + 1);
        const char *body = ft_strstr(response.c_str(), "\r\n\r\n");
        if (body)
        {
            body += 4;
            result_body = cma_strdup(body);
            if (!result_body && ft_errno == ER_SUCCESS)
                ft_errno = FT_EALLOC;
            if (result_body)
                ft_errno = ER_SUCCESS;
        }
    }

cleanup:
    if (data->callback)
        data->callback(result_body, status, data->user_data);
    if (socket_fd >= 0)
        FT_CLOSE_SOCKET(socket_fd);
    if (address_results)
        freeaddrinfo(address_results);
    if (data)
    {
        if (data->ip)
            cma_free(data->ip);
        if (data->method)
            cma_free(data->method);
        if (data->path)
            cma_free(data->path);
        if (data->headers)
            cma_free(data->headers);
        cma_free(data);
    }
    return ;
}

bool    api_request_string_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    if (!ip || !method || !path || !callback)
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    api_async_request *data = static_cast<api_async_request*>(cma_malloc(sizeof(api_async_request)));
    if (!data)
    {
        ft_errno = FT_EALLOC;
        return (false);
    }
    ft_bzero(data, sizeof(api_async_request));
    data->ip = cma_strdup(ip);
    data->method = cma_strdup(method);
    data->path = cma_strdup(path);
    if (headers)
        data->headers = cma_strdup(headers);
    data->port = port;
    data->payload = payload;
    data->timeout = timeout;
    data->callback = callback;
    data->user_data = user_data;
    if (!data->ip || !data->method || !data->path || (headers && !data->headers))
    {
        if (data->ip)
            cma_free(data->ip);
        if (data->method)
            cma_free(data->method);
        if (data->path)
            cma_free(data->path);
        if (data->headers)
            cma_free(data->headers);
        cma_free(data);
        ft_errno = FT_EALLOC;
        return (false);
    }
    ft_thread thread_worker(api_async_worker, data);
    thread_worker.detach();
    ft_errno = ER_SUCCESS;
    return (true);
}

struct api_json_async_data
{
    api_json_callback callback;
    void *user_data;
};

static void api_json_async_wrapper(char *body, int status, void *user_data)
{
    api_json_async_data *data;
    json_group *json_body;

    data = static_cast<api_json_async_data*>(user_data);
    json_body = ft_nullptr;
    if (body)
    {
        json_body = json_read_from_string(body);
        cma_free(body);
    }
    if (data && data->callback)
        data->callback(json_body, status, data->user_data);
    if (data)
        cma_free(data);
    return ;
}

bool    api_request_json_async(const char *ip, uint16_t port,
        const char *method, const char *path, api_json_callback callback,
        void *user_data, json_group *payload, const char *headers, int timeout)
{
    api_json_async_data *data;

    if (!ip || !method || !path || !callback)
    {
        ft_errno = FT_EINVAL;
        return (false);
    }
    data = static_cast<api_json_async_data*>(cma_malloc(sizeof(api_json_async_data)));
    if (!data)
    {
        ft_errno = FT_EALLOC;
        return (false);
    }
    data->callback = callback;
    data->user_data = user_data;
    if (!api_request_string_async(ip, port, method, path, api_json_async_wrapper,
            data, payload, headers, timeout))
    {
        cma_free(data);
        ft_errno = ft_errno == ER_SUCCESS ? FT_EALLOC : ft_errno;
        return (false);
    }
    ft_errno = ER_SUCCESS;
    return (true);
}
