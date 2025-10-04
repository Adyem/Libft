#include "api.hpp"
#include "api_internal.hpp"
#include "../Networking/socket_class.hpp"
#include "../CPP_class/class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Printf/printf.hpp"
#include "../PThread/thread.hpp"
#include "../Time/time.hpp"
#include <atomic>
#include <errno.h>
#include <limits.h>
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

static std::atomic<size_t> g_api_async_last_request_size(0);
static std::atomic<size_t> g_api_async_last_bytes_sent(0);
static std::atomic<size_t> g_api_async_last_bytes_received(0);
static std::atomic<int> g_api_async_last_send_state(0);
static std::atomic<int> g_api_async_last_send_timeout(0);
static std::atomic<int> g_api_async_last_receive_state(0);
static std::atomic<int> g_api_async_last_receive_timeout(0);

static bool api_request_string_ends_with_crlf(const ft_string &value)
{
    size_t length;
    const char *character_pointer;

    length = value.size();
    if (length < 2)
        return (false);
    character_pointer = value.at(length - 2);
    if (character_pointer == ft_nullptr)
        return (false);
    if (*character_pointer != '\r')
        return (false);
    character_pointer = value.at(length - 1);
    if (character_pointer == ft_nullptr)
        return (false);
    if (*character_pointer != '\n')
        return (false);
    return (true);
}

static void api_request_trim_header_block(ft_string &headers)
{
    size_t length;
    const char *character_pointer;

    length = headers.size();
    while (length > 0)
    {
        character_pointer = headers.at(length - 1);
        if (character_pointer == ft_nullptr)
            break;
        if (*character_pointer == '\r' || *character_pointer == '\n')
        {
            headers.erase(length - 1, 1);
            length -= 1;
            continue;
        }
        break;
    }
    length = headers.size();
    while (length > 0)
    {
        character_pointer = headers.at(0);
        if (character_pointer == ft_nullptr)
            break;
        if (*character_pointer == '\r' || *character_pointer == '\n')
        {
            headers.erase(0, 1);
            length -= 1;
            continue;
        }
        break;
    }
    return ;
}

static void api_request_append_header_block(ft_string &request, const ft_string &block)
{
    if (block.empty())
        return ;
    if (!api_request_string_ends_with_crlf(request))
        request += "\r\n";
    request += block;
    return ;
}

static void api_request_append_header_line(ft_string &request, const char *line)
{
    if (!api_request_string_ends_with_crlf(request))
        request += "\r\n";
    request += line;
    return ;
}

static void api_request_ensure_header_termination(ft_string &request)
{
    size_t length;
    const char *first_pointer;
    const char *second_pointer;
    const char *third_pointer;
    const char *fourth_pointer;

    length = request.size();
    if (length >= 4)
    {
        first_pointer = request.at(length - 4);
        second_pointer = request.at(length - 3);
        third_pointer = request.at(length - 2);
        fourth_pointer = request.at(length - 1);
        if (first_pointer != ft_nullptr && second_pointer != ft_nullptr &&
                third_pointer != ft_nullptr && fourth_pointer != ft_nullptr)
        {
            if (*first_pointer == '\r' && *second_pointer == '\n' &&
                    *third_pointer == '\r' && *fourth_pointer == '\n')
                return ;
        }
    }
    if (!api_request_string_ends_with_crlf(request))
        request += "\r\n";
    request += "\r\n";
    return ;
}

static void api_request_reset_async_debug_counters(void)
{
    g_api_async_last_request_size.store(0);
    g_api_async_last_bytes_sent.store(0);
    g_api_async_last_bytes_received.store(0);
    g_api_async_last_send_state.store(0);
    g_api_async_last_send_timeout.store(0);
    g_api_async_last_receive_state.store(0);
    g_api_async_last_receive_timeout.store(0);
    return ;
}

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

static bool api_prepare_timeout(struct timeval *time_value, bool has_deadline,
        t_monotonic_time_point deadline_point, int fallback_timeout)
{
    if (!time_value)
        return (false);
    if (has_deadline)
    {
        t_monotonic_time_point now_point;
        long long remaining_ms;

        now_point = time_monotonic_point_now();
        remaining_ms = time_monotonic_point_diff_ms(now_point, deadline_point);
        if (remaining_ms <= 0)
            return (false);
        if (remaining_ms > INT_MAX)
            remaining_ms = INT_MAX;
        api_set_timeval(time_value, static_cast<int>(remaining_ms));
        return (true);
    }
    api_set_timeval(time_value, fallback_timeout);
    return (true);
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
    int send_timeout_ms;
    int receive_timeout_ms;
    size_t total_sent;
    int send_retry_attempts;
    int resolver_status;
    bool has_send_deadline;
    bool has_receive_deadline;
    t_monotonic_time_point send_deadline;
    t_monotonic_time_point receive_deadline;
    bool restore_non_blocking;
#ifndef _WIN32
    int original_flags;
#else
    u_long blocking_mode;
#endif

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

    restore_non_blocking = false;
#ifdef _WIN32
    blocking_mode = 0;
    if (ioctlsocket(socket_fd, FIONBIO, &blocking_mode) == 0)
        restore_non_blocking = true;
#else
    int current_flags;

    current_flags = fcntl(socket_fd, F_GETFL, 0);
    if (current_flags >= 0 && (current_flags & O_NONBLOCK))
    {
        if (fcntl(socket_fd, F_SETFL, current_flags & ~O_NONBLOCK) == 0)
        {
            restore_non_blocking = true;
            original_flags = current_flags;
        }
    }
#endif

    api_request_reset_async_debug_counters();
    request = data->method;
    request += " ";
    request += data->path;
    request += " HTTP/1.1\r\nHost: ";
    request += data->ip;
    if (data->headers && data->headers[0])
    {
        ft_string headers_string;

        headers_string = data->headers;
        api_request_trim_header_block(headers_string);
        if (!headers_string.empty())
            api_request_append_header_block(request, headers_string);
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
    api_request_append_header_line(request, "Connection: close");
    api_request_ensure_header_termination(request);
    if (data->payload)
        request += body_string.c_str();

    g_api_async_last_request_size.store(request.size());

#if defined(SO_SNDBUF)
    if (request.size() > 0)
    {
        int desired_buffer;

        if (request.size() > static_cast<size_t>(INT_MAX))
            desired_buffer = INT_MAX;
        else
            desired_buffer = static_cast<int>(request.size());
        setsockopt(socket_fd, SOL_SOCKET, SO_SNDBUF,
                   reinterpret_cast<const char*>(&desired_buffer),
                   sizeof(desired_buffer));
    }
#endif
#if defined(TCP_NODELAY)
    {
        int tcp_no_delay;

        tcp_no_delay = 1;
        setsockopt(socket_fd, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&tcp_no_delay),
                   sizeof(tcp_no_delay));
    }
#endif

    send_timeout_ms = timeout_ms;
    if (send_timeout_ms > 0)
    {
        size_t request_size;
        size_t request_kib;

        request_size = request.size();
        request_kib = request_size / 1024;
        if ((request_size % 1024) != 0)
            request_kib += 1;
        if (request_kib > 0)
        {
            if (request_kib > static_cast<size_t>(INT_MAX))
                send_timeout_ms = INT_MAX;
            else if (static_cast<int>(request_kib) > send_timeout_ms)
                send_timeout_ms = static_cast<int>(request_kib);
        }
    }
    if (send_timeout_ms < 10000)
        send_timeout_ms = 10000;
    g_api_async_last_send_timeout.store(send_timeout_ms);
    has_send_deadline = false;
    if (send_timeout_ms > 0)
    {
        send_deadline = time_monotonic_point_now();
        send_deadline = time_monotonic_point_add_ms(send_deadline, send_timeout_ms);
        has_send_deadline = true;
    }
    FD_ZERO(&write_set);
    FD_SET(socket_fd, &write_set);
    g_api_async_last_send_state.store(1);
    if (!api_prepare_timeout(&tv, has_send_deadline, send_deadline, send_timeout_ms))
    {
        if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_SEND_FAILED;
        g_api_async_last_send_state.store(2);
        goto cleanup;
    }
    int initial_select_result;

    initial_select_result = select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv);
    if (initial_select_result <= 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else if (ft_errno == ER_SUCCESS)
            ft_errno = SOCKET_CONNECT_FAILED;
        if (initial_select_result == 0)
            g_api_async_last_send_state.store(2);
        else
            g_api_async_last_send_state.store(3);
        goto cleanup;
    }
    g_api_async_last_send_state.store(4);
    if (has_send_deadline)
    {
        send_deadline = time_monotonic_point_now();
        send_deadline = time_monotonic_point_add_ms(send_deadline, send_timeout_ms);
    }
    total_sent = 0;
    send_retry_attempts = 0;
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
                g_api_async_last_send_state.store(6);
                FD_ZERO(&write_set);
                FD_SET(socket_fd, &write_set);
                if (!api_prepare_timeout(&tv, has_send_deadline, send_deadline, send_timeout_ms))
                {
                    if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
                    g_api_async_last_send_state.store(8);
                    goto cleanup;
                }
                int select_result;

                select_result = select(socket_fd + 1, ft_nullptr, &write_set, ft_nullptr, &tv);
                if (select_result <= 0)
                {
#ifdef _WIN32
                    int select_error;

                    select_error = WSAGetLastError();
                    if (select_result < 0 && select_error == WSAEINTR)
                    {
                        g_api_async_last_send_state.store(6);
                        continue;
                    }
                    if (select_result < 0)
                    {
                        if (select_error != 0)
                            ft_errno = select_error + ERRNO_OFFSET;
                        else if (ft_errno == ER_SUCCESS)
                            ft_errno = SOCKET_SEND_FAILED;
                    }
                    else if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
#else
                    int select_errno;

                    select_errno = errno;
                    if (select_result < 0 && (select_errno == EINTR || select_errno == EAGAIN))
                    {
                        errno = 0;
                        g_api_async_last_send_state.store(6);
                        continue;
                    }
                    if (select_result < 0)
                    {
                        if (select_errno != 0)
                            ft_errno = select_errno + ERRNO_OFFSET;
                        else if (ft_errno == ER_SUCCESS)
                            ft_errno = SOCKET_SEND_FAILED;
                    }
                    else if (ft_errno == ER_SUCCESS)
                        ft_errno = SOCKET_SEND_FAILED;
#endif
                    if (select_result == 0)
                        g_api_async_last_send_state.store(2);
                    else
                        g_api_async_last_send_state.store(8);
                    goto cleanup;
                }
                send_retry_attempts = 0;
                g_api_async_last_send_state.store(4);
                continue;
            }
#ifdef _WIN32
            if (send_err == WSAEINTR)
            {
                g_api_async_last_send_state.store(6);
                continue;
            }
#else
            if (errno == EINTR)
            {
                errno = 0;
                g_api_async_last_send_state.store(6);
                continue;
            }
#endif
            if (send_retry_attempts < 10)
            {
                send_retry_attempts += 1;
                pt_thread_sleep(10);
                g_api_async_last_send_state.store(7);
                continue;
            }
#ifdef _WIN32
            if (send_err != 0)
                ft_errno = send_err + ERRNO_OFFSET;
            else if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_SEND_FAILED;
#else
            if (errno != 0)
                ft_errno = errno + ERRNO_OFFSET;
            else if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_SEND_FAILED;
#endif
            goto cleanup;
        }
        total_sent += static_cast<size_t>(bytes_sent);
        g_api_async_last_bytes_sent.store(total_sent);
        g_api_async_last_send_state.store(5);
        send_retry_attempts = 0;
        if (has_send_deadline)
        {
            send_deadline = time_monotonic_point_now();
            send_deadline = time_monotonic_point_add_ms(send_deadline, send_timeout_ms);
        }
    }
    if (restore_non_blocking)
    {
#ifdef _WIN32
        blocking_mode = 1;
        ioctlsocket(socket_fd, FIONBIO, &blocking_mode);
#else
        fcntl(socket_fd, F_SETFL, original_flags);
#endif
    }
    g_api_async_last_send_state.store(9);

    receive_timeout_ms = timeout_ms;
    if (receive_timeout_ms < send_timeout_ms)
        receive_timeout_ms = send_timeout_ms;
    if (request.size() > 0)
    {
        size_t request_kib;
        int extra_timeout;

        request_kib = request.size() / 1024;
        if ((request.size() % 1024) != 0)
            request_kib += 1;
        extra_timeout = 0;
        if (request_kib > 0)
        {
            if (request_kib > static_cast<size_t>(INT_MAX))
                extra_timeout = INT_MAX;
            else
                extra_timeout = static_cast<int>(request_kib);
        }
        if (extra_timeout > 0)
        {
            if (receive_timeout_ms > INT_MAX - extra_timeout)
                receive_timeout_ms = INT_MAX;
            else
                receive_timeout_ms += extra_timeout;
        }
    }
    if (receive_timeout_ms < 15000)
        receive_timeout_ms = 15000;
    has_receive_deadline = false;
    if (receive_timeout_ms > 0)
    {
        receive_deadline = time_monotonic_point_now();
        receive_deadline = time_monotonic_point_add_ms(receive_deadline, receive_timeout_ms);
        has_receive_deadline = true;
    }
    g_api_async_last_receive_timeout.store(receive_timeout_ms);
    size_t total_received;

    total_received = 0;
    while (true)
    {
        g_api_async_last_receive_state.store(7);
        FD_ZERO(&read_set);
        FD_SET(socket_fd, &read_set);
        if (!api_prepare_timeout(&tv, has_receive_deadline, receive_deadline, receive_timeout_ms))
        {
            if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_RECEIVE_FAILED;
            g_api_async_last_receive_state.store(6);
            break;
        }
        int select_result = select(socket_fd + 1, &read_set, ft_nullptr, ft_nullptr, &tv);
        if (select_result <= 0)
        {
#ifdef _WIN32
            int select_error;

            select_error = WSAGetLastError();
            if (select_result < 0 && select_error == WSAEINTR)
                continue;
            if (select_result < 0)
            {
                if (select_error != 0)
                    ft_errno = select_error + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_RECEIVE_FAILED;
                g_api_async_last_receive_state.store(2);
            }
            else if (ft_errno == ER_SUCCESS)
            {
                g_api_async_last_receive_state.store(1);
                ft_errno = SOCKET_RECEIVE_FAILED;
            }
#else
            int select_errno;

            select_errno = errno;
            if (select_result < 0 && (select_errno == EINTR || select_errno == EAGAIN))
            {
                errno = 0;
                continue;
            }
            if (select_result < 0)
            {
                if (select_errno != 0)
                    ft_errno = select_errno + ERRNO_OFFSET;
                else if (ft_errno == ER_SUCCESS)
                    ft_errno = SOCKET_RECEIVE_FAILED;
                g_api_async_last_receive_state.store(2);
            }
            else if (ft_errno == ER_SUCCESS)
            {
                g_api_async_last_receive_state.store(1);
                ft_errno = SOCKET_RECEIVE_FAILED;
            }
#endif
            break;
        }
        ssize_t bytes_received = nw_recv(socket_fd, buffer,
                                         sizeof(buffer) - 1, 0);
        if (bytes_received < 0)
        {
#ifdef _WIN32
            int recv_error;

            recv_error = WSAGetLastError();
            if (recv_error == WSAEWOULDBLOCK)
                continue;
            if (recv_error == WSAEINTR)
                continue;
            if (recv_error != 0)
                ft_errno = recv_error + ERRNO_OFFSET;
            else if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_RECEIVE_FAILED;
            g_api_async_last_receive_state.store(3);
#else
            int recv_errno;

            recv_errno = errno;
            if (recv_errno == EAGAIN || recv_errno == EWOULDBLOCK)
            {
                errno = 0;
                continue;
            }
            if (recv_errno == EINTR)
            {
                errno = 0;
                continue;
            }
            if (recv_errno != 0)
                ft_errno = recv_errno + ERRNO_OFFSET;
            else if (ft_errno == ER_SUCCESS)
                ft_errno = SOCKET_RECEIVE_FAILED;
            g_api_async_last_receive_state.store(3);
#endif
            break;
        }
        if (bytes_received == 0)
        {
            g_api_async_last_receive_state.store(4);
            break;
        }
        buffer[bytes_received] = '\0';
        response += buffer;
        total_received += static_cast<size_t>(bytes_received);
        g_api_async_last_bytes_received.store(total_received);
        g_api_async_last_receive_state.store(5);
        if (has_receive_deadline)
        {
            receive_deadline = time_monotonic_point_now();
            receive_deadline = time_monotonic_point_add_ms(receive_deadline, receive_timeout_ms);
        }
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

size_t  api_debug_get_last_async_request_size(void)
{
    return (g_api_async_last_request_size.load());
}

size_t  api_debug_get_last_async_bytes_sent(void)
{
    return (g_api_async_last_bytes_sent.load());
}

int     api_debug_get_last_async_send_state(void)
{
    return (g_api_async_last_send_state.load());
}

int     api_debug_get_last_async_send_timeout(void)
{
    return (g_api_async_last_send_timeout.load());
}

size_t  api_debug_get_last_async_bytes_received(void)
{
    return (g_api_async_last_bytes_received.load());
}

int     api_debug_get_last_async_receive_state(void)
{
    return (g_api_async_last_receive_state.load());
}

int     api_debug_get_last_async_receive_timeout(void)
{
    return (g_api_async_last_receive_timeout.load());
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
