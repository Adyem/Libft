#include "http_client.hpp"
#include "http2_client.hpp"
#include "socket_class.hpp"
#include "networking.hpp"
#include "ssl_wrapper.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <vector>
#include <openssl/x509v3.h>
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include "../Time/time.hpp"
#include "../Observability/observability_networking_metrics.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

struct http_stream_state
{
    ft_string           header_buffer;
    ft_string           headers;
    int                 status_code;
    bool                headers_ready;
    bool                keep_alive_allowed;
    bool                has_content_length;
    size_t              expected_body_length;
    size_t              received_body_length;
    bool                saw_chunked_encoding;
    bool                http_1_1;
    http_response_handler handler;
};

struct http_buffer_adapter_state
{
    ft_string   *response;
    bool        header_appended;
    size_t      body_bytes;
    int         status_code;
};

static http_buffer_adapter_state g_http_buffer_adapter_state = { NULL, false, 0, 0 };

struct http_client_connection_entry
{
    ft_string               host;
    ft_string               port;
    bool                    use_ssl;
    int                     socket_fd;
    SSL_CTX                 *ssl_context;
    SSL                     *ssl_connection;
    t_monotonic_time_point  last_used;
};

struct http_client_active_connection
{
    http_client_connection_entry   entry;
    bool                            from_pool;
    bool                            store_allowed;
};

static pt_mutex g_http_client_pool_mutex;
static std::vector<http_client_connection_entry> g_http_client_pool_entries;
static size_t g_http_client_pool_max_idle = 8;
static long long g_http_client_pool_idle_timeout_ms = 30000;
static size_t g_http_client_pool_acquire_calls = 0;
static size_t g_http_client_pool_reuse_hits = 0;
static size_t g_http_client_pool_acquire_misses = 0;

static void http_client_pool_reset_active(http_client_active_connection &connection)
{
    connection.entry.socket_fd = -1;
    connection.entry.ssl_context = NULL;
    connection.entry.ssl_connection = NULL;
    connection.entry.host.clear();
    connection.entry.port.clear();
    connection.entry.use_ssl = false;
    connection.from_pool = false;
    connection.store_allowed = true;
    return ;
}

static void http_client_pool_dispose_entry(http_client_connection_entry &entry)
{
    if (entry.ssl_connection != NULL)
    {
        SSL_shutdown(entry.ssl_connection);
        SSL_free(entry.ssl_connection);
        entry.ssl_connection = NULL;
    }
    if (entry.ssl_context != NULL)
    {
        SSL_CTX_free(entry.ssl_context);
        entry.ssl_context = NULL;
    }
    if (entry.socket_fd >= 0)
    {
        nw_close(entry.socket_fd);
        entry.socket_fd = -1;
    }
    entry.host.clear();
    entry.port.clear();
    entry.use_ssl = false;
    entry.last_used = time_monotonic_point_now();
    return ;
}

static void http_client_pool_prune_locked(t_monotonic_time_point now)
{
    size_t index;

    index = 0;
    while (index < g_http_client_pool_entries.size())
    {
        http_client_connection_entry &candidate = g_http_client_pool_entries[index];
        long long idle_time_ms;

        idle_time_ms = time_monotonic_point_diff_ms(candidate.last_used, now);
        if (idle_time_ms < 0)
            idle_time_ms = 0;
        if (idle_time_ms > g_http_client_pool_idle_timeout_ms)
        {
            http_client_pool_dispose_entry(candidate);
            g_http_client_pool_entries.erase(g_http_client_pool_entries.begin() + index);
            continue ;
        }
        index++;
    }
    return ;
}

static void http_client_pool_release_connection(http_client_active_connection &connection, bool allow_reuse)
{
    if (connection.entry.socket_fd < 0)
    {
        http_client_pool_reset_active(connection);
        return ;
    }
    if (allow_reuse == false || connection.store_allowed == false)
    {
        http_client_pool_dispose_entry(connection.entry);
        http_client_pool_reset_active(connection);
        return ;
    }
    int lock_error;
    t_monotonic_time_point now;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        http_client_pool_dispose_entry(connection.entry);
        http_client_pool_reset_active(connection);
        return ;
    }
    now = time_monotonic_point_now();
    http_client_pool_prune_locked(now);
    if (g_http_client_pool_entries.size() >= g_http_client_pool_max_idle)
    {
        (void)g_http_client_pool_mutex.unlock();
        http_client_pool_dispose_entry(connection.entry);
        http_client_pool_reset_active(connection);
        return ;
    }
    connection.entry.last_used = now;
    g_http_client_pool_entries.push_back(connection.entry);
    (void)g_http_client_pool_mutex.unlock();
    http_client_pool_reset_active(connection);
    return ;
}

static void http_client_pool_disable_store(http_client_active_connection &connection)
{
    connection.store_allowed = false;
    return ;
}

static int http_client_pool_acquire_connection(const char *host, const char *port,
    bool use_ssl, http_client_active_connection &connection, bool &reused)
{
    int lock_error;
    t_monotonic_time_point now;
    size_t index;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (1);
    now = time_monotonic_point_now();
    http_client_pool_prune_locked(now);
    g_http_client_pool_acquire_calls++;
    index = 0;
    while (index < g_http_client_pool_entries.size())
    {
        http_client_connection_entry &candidate = g_http_client_pool_entries[index];

        if (candidate.use_ssl == use_ssl && candidate.host == host && candidate.port == port)
        {
            connection.entry = candidate;
            connection.from_pool = true;
            connection.store_allowed = true;
            g_http_client_pool_entries.erase(g_http_client_pool_entries.begin() + index);
            g_http_client_pool_reuse_hits++;
            reused = true;
            (void)g_http_client_pool_mutex.unlock();
            return (0);
        }
        index++;
    }
    g_http_client_pool_acquire_misses++;
    (void)g_http_client_pool_mutex.unlock();
    connection.entry.host = host;
    connection.entry.port = port;
    connection.entry.use_ssl = use_ssl;
    connection.entry.socket_fd = -1;
    connection.entry.ssl_context = NULL;
    connection.entry.ssl_connection = NULL;
    connection.entry.last_used = now;
    connection.from_pool = false;
    connection.store_allowed = true;
    reused = false;
    return (0);
}

static void http_client_trim_string(ft_string &value)
{
    size_t start_index;
    size_t end_index;

    start_index = 0;
    while (start_index < value.size()
        && ft_isspace(static_cast<unsigned char>(value[start_index])) != 0)
        start_index++;
    if (start_index > 0)
        value.erase(0, start_index);
    end_index = value.size();
    while (end_index > 0
        && ft_isspace(static_cast<unsigned char>(value[end_index - 1])) != 0)
        end_index--;
    if (end_index < value.size())
        value.erase(end_index, value.size() - end_index);
    return ;
}

static void http_client_to_lower(ft_string &value)
{
    size_t index;
    char *mutable_data;

    index = 0;
    mutable_data = value.data();
    if (mutable_data == NULL)
        return ;
    while (index < value.size())
    {
        char character;

        character = mutable_data[index];
        if (character >= 'A' && character <= 'Z')
            mutable_data[index] = static_cast<char>(character + 32);
        index++;
    }
    return ;
}

static void http_client_parse_header_metadata(http_stream_state &state)
{
    const char  *header_data;
    size_t      header_limit;
    size_t      offset;
    ft_string   status_prefix;

    state.keep_alive_allowed = false;
    state.has_content_length = false;
    state.expected_body_length = 0;
    state.received_body_length = 0;
    state.saw_chunked_encoding = false;
    state.http_1_1 = false;
    header_limit = state.headers.size();
    header_data = state.headers.c_str();
    if (header_limit >= 8)
    {
        status_prefix.assign(header_data, 8);
        if (status_prefix == "HTTP/1.1")
        {
            state.http_1_1 = true;
            state.keep_alive_allowed = true;
        }
    }
    offset = 0;
    while (offset < header_limit)
    {
        size_t line_length;
        const char *line_start;
        size_t colon_index;
        ft_string header_name;
        ft_string header_value;
        ft_string lowered_value;

        line_start = header_data + offset;
        line_length = 0;
        while (offset + line_length + 1 < header_limit)
        {
            if (line_start[line_length] == '\r'
                && line_start[line_length + 1] == '\n')
                break;
            line_length++;
        }
        if (offset + line_length + 1 >= header_limit)
            break;
        offset += line_length + 2;
        if (line_length == 0)
            continue ;
        colon_index = 0;
        while (colon_index < line_length && line_start[colon_index] != ':')
            colon_index++;
        if (colon_index >= line_length)
            continue ;
        header_name.assign(line_start, colon_index);
        header_value.assign(line_start + colon_index + 1, line_length - colon_index - 1);
        http_client_trim_string(header_name);
        http_client_trim_string(header_value);
        http_client_to_lower(header_name);
        if (header_name == "connection")
        {
            lowered_value = header_value;
            http_client_to_lower(lowered_value);
            if (lowered_value.find("close") != ft_string::npos)
                state.keep_alive_allowed = false;
            else if (lowered_value.find("keep-alive") != ft_string::npos)
                state.keep_alive_allowed = true;
            continue ;
        }
        if (header_name == "content-length")
        {
            size_t digit_index;
            unsigned long long parsed_length;

            digit_index = 0;
            parsed_length = 0;
            while (digit_index < header_value.size()
                && ft_isdigit(static_cast<unsigned char>(header_value[digit_index])) != 0)
            {
                parsed_length = (parsed_length * 10)
                    + static_cast<unsigned long long>(header_value[digit_index] - '0');
                digit_index++;
            }
            state.has_content_length = true;
            state.expected_body_length = static_cast<size_t>(parsed_length);
            continue ;
        }
        if (header_name == "transfer-encoding")
        {
            lowered_value = header_value;
            http_client_to_lower(lowered_value);
            if (lowered_value.find("chunked") != ft_string::npos)
            {
                state.saw_chunked_encoding = true;
                state.keep_alive_allowed = false;
            }
        }
    }
    if (state.has_content_length == false)
        state.keep_alive_allowed = false;
    if (state.saw_chunked_encoding != false)
        state.keep_alive_allowed = false;
    return ;
}

void http_client_pool_flush(void)
{
    int lock_error;
    size_t index;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    index = 0;
    while (index < g_http_client_pool_entries.size())
    {
        http_client_pool_dispose_entry(g_http_client_pool_entries[index]);
        index++;
    }
    g_http_client_pool_entries.clear();
    (void)g_http_client_pool_mutex.unlock();
    return ;
}

void http_client_pool_set_max_idle(size_t max_idle)
{
    int lock_error;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    g_http_client_pool_max_idle = max_idle;
    if (g_http_client_pool_entries.size() > g_http_client_pool_max_idle)
    {
        size_t index;

        index = g_http_client_pool_max_idle;
        while (index < g_http_client_pool_entries.size())
        {
            http_client_pool_dispose_entry(g_http_client_pool_entries[index]);
            index++;
        }
        g_http_client_pool_entries.resize(g_http_client_pool_max_idle);
    }
    (void)g_http_client_pool_mutex.unlock();
    return ;
}

size_t http_client_pool_get_idle_count(void)
{
    int lock_error;
    size_t idle_count;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    idle_count = g_http_client_pool_entries.size();
    (void)g_http_client_pool_mutex.unlock();
    return (idle_count);
}

void http_client_pool_debug_reset_counters(void)
{
    int lock_error;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    g_http_client_pool_acquire_calls = 0;
    g_http_client_pool_reuse_hits = 0;
    g_http_client_pool_acquire_misses = 0;
    (void)g_http_client_pool_mutex.unlock();
    return ;
}

size_t http_client_pool_debug_get_reuse_count(void)
{
    int lock_error;
    size_t reuse_count;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    reuse_count = g_http_client_pool_reuse_hits;
    (void)g_http_client_pool_mutex.unlock();
    return (reuse_count);
}

size_t http_client_pool_debug_get_miss_count(void)
{
    int lock_error;
    size_t miss_count;

    lock_error = g_http_client_pool_mutex.lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    miss_count = g_http_client_pool_acquire_misses;
    (void)g_http_client_pool_mutex.unlock();
    return (miss_count);
}

static int http_client_wait_for_socket_ready(int socket_fd, bool wait_for_write)
{
    int poll_descriptor;
    int poll_result;

    if (socket_fd < 0)
    {
        return (-1);
    }
    poll_descriptor = socket_fd;
    if (wait_for_write != false)
        poll_result = nw_poll(NULL, 0, &poll_descriptor, 1, 1000);
    else
        poll_result = nw_poll(&poll_descriptor, 1, NULL, 0, 1000);
    if (poll_result < 0)
    {
#ifdef _WIN32
#else
#endif
        return (-1);
    }
    if (poll_result == 0)
        return (1);
    return (0);
}

static int http_client_initialize_ssl(int socket_fd, const char *host, SSL_CTX **ssl_context, SSL **ssl_connection)
{
    SSL_CTX *local_context;
    SSL *local_connection;

    if (ssl_context == NULL || ssl_connection == NULL)
    {
        return (-1);
    }
    *ssl_context = NULL;
    *ssl_connection = NULL;
    SSL_library_init();
    local_context = SSL_CTX_new(TLS_client_method());
    if (local_context == NULL)
    {
        return (-1);
    }
    SSL_CTX_set_verify(local_context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(local_context) != 1)
    {
        SSL_CTX_free(local_context);
        return (-1);
    }
    local_connection = SSL_new(local_context);
    if (local_connection == NULL)
    {
        SSL_CTX_free(local_context);
        return (-1);
    }
    bool selected_http2;
    int alpn_error;

    if (!http2_select_alpn_protocol(local_connection, selected_http2, alpn_error))
    (void)selected_http2;
    (void)alpn_error;
    if (host != NULL && host[0] != '\0')
    {
        long control_result;

        control_result = SSL_ctrl(local_connection, SSL_CTRL_SET_TLSEXT_HOSTNAME,
            TLSEXT_NAMETYPE_host_name, const_cast<char *>(host));
        if (control_result != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        X509_VERIFY_PARAM *verify_params;

        verify_params = SSL_get0_param(local_connection);
        if (verify_params == NULL)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            return (-1);
        }
        X509_VERIFY_PARAM_set_hostflags(verify_params, 0);
        if (X509_VERIFY_PARAM_set1_host(verify_params, host, 0) != 1)
        {
            SSL_free(local_connection);
            SSL_CTX_free(local_context);
            return (-1);
        }
#endif
    }
    if (SSL_set_fd(local_connection, socket_fd) != 1)
    {
        SSL_free(local_connection);
        SSL_CTX_free(local_context);
        return (-1);
    }
    *ssl_context = local_context;
    *ssl_connection = local_connection;
    return (0);
}

static int http_client_establish_connection(const char *host, const char *port_string,
    bool use_ssl, http_client_active_connection &connection)
{
    struct addrinfo address_hints;
    struct addrinfo *address_info;
    struct addrinfo *current_info;
    int socket_fd;
    int result;
    int resolver_status;

    if (connection.entry.socket_fd >= 0)
        return (0);
    ft_memset(&address_hints, 0, sizeof(address_hints));
    address_hints.ai_family = AF_UNSPEC;
    address_hints.ai_socktype = SOCK_STREAM;
    resolver_status = getaddrinfo(host, port_string, &address_hints, &address_info);
    if (resolver_status != 0)
    {
        networking_dns_set_error(resolver_status);
        http_client_pool_disable_store(connection);
        return (-1);
    }
    socket_fd = -1;
    result = -1;
    current_info = address_info;
    while (current_info != NULL)
    {
        socket_fd = nw_socket(current_info->ai_family, current_info->ai_socktype, current_info->ai_protocol);
        if (socket_fd >= 0)
        {
            result = nw_connect(socket_fd, current_info->ai_addr, current_info->ai_addrlen);
            if (result >= 0)
                break;
            nw_close(socket_fd);
            socket_fd = -1;
        }
        else
        {
        }
        current_info = current_info->ai_next;
    }
    freeaddrinfo(address_info);
    if (socket_fd < 0 || result < 0)
    {
        http_client_pool_disable_store(connection);
        return (-1);
    }
    connection.entry.socket_fd = socket_fd;
    connection.entry.use_ssl = use_ssl;
    if (use_ssl != false)
    {
        if (http_client_initialize_ssl(socket_fd, host, &connection.entry.ssl_context,
            &connection.entry.ssl_connection) != 0)
        {
            nw_close(socket_fd);
            connection.entry.socket_fd = -1;
            http_client_pool_disable_store(connection);
            return (-1);
        }
        result = SSL_connect(connection.entry.ssl_connection);
        if (result != 1)
        {
            SSL_free(connection.entry.ssl_connection);
            SSL_CTX_free(connection.entry.ssl_context);
            connection.entry.ssl_connection = NULL;
            connection.entry.ssl_context = NULL;
            nw_close(socket_fd);
            connection.entry.socket_fd = -1;
            http_client_pool_disable_store(connection);
            return (-1);
        }
#if OPENSSL_VERSION_NUMBER >= 0x10002000L
        if (SSL_get_verify_result(connection.entry.ssl_connection) != X509_V_OK)
        {
            SSL_shutdown(connection.entry.ssl_connection);
            SSL_free(connection.entry.ssl_connection);
            SSL_CTX_free(connection.entry.ssl_context);
            connection.entry.ssl_connection = NULL;
            connection.entry.ssl_context = NULL;
            nw_close(socket_fd);
            connection.entry.socket_fd = -1;
            http_client_pool_disable_store(connection);
            return (-1);
        }
#endif
    }
    return (0);
}

int http_client_send_plain_request(int socket_fd, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;

    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_send(socket_fd, buffer + total_sent, length - total_sent, 0);
        if (send_result < 0)
        {
            int wait_result;

#ifdef _WIN32
            int last_error;

            last_error = WSAGetLastError();
            if (last_error == WSAEINTR)
                continue ;
            if (last_error == WSAEWOULDBLOCK)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
#else
            if (errno == EINTR)
                continue ;
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
#endif
            return (-1);
        }
        if (send_result == 0)
        {
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_socket_after_send(socket_fd) != 0)
        return (-1);
    return (0);
}

int http_client_send_ssl_request(SSL *ssl_connection, const char *buffer, size_t length)
{
    size_t total_sent;
    ssize_t send_result;
    int socket_fd;

    socket_fd = -1;
    if (ssl_connection != NULL)
        socket_fd = SSL_get_fd(ssl_connection);
    total_sent = 0;
    while (total_sent < length)
    {
        send_result = nw_ssl_write(ssl_connection, buffer + total_sent, length - total_sent);
        if (send_result <= 0)
        {
            int ssl_error;
            int wait_result;

            ssl_error = SSL_get_error(ssl_connection, static_cast<int>(send_result));
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
            {
                bool wait_for_write;

                wait_for_write = (ssl_error == SSL_ERROR_WANT_WRITE);
                wait_result = http_client_wait_for_socket_ready(socket_fd, wait_for_write);
                if (wait_result < 0)
                    return (-1);
                continue ;
            }
            if (ssl_error == SSL_ERROR_SYSCALL)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (last_error == WSAEINTR)
                    continue ;
                if (last_error == WSAEWOULDBLOCK)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error != 0)
                {
                    return (-1);
                }
#else
                if (errno == EINTR)
                    continue ;
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    wait_result = http_client_wait_for_socket_ready(socket_fd, true);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (errno != 0)
                {
                    return (-1);
                }
#endif
            }
            return (-1);
        }
        total_sent += static_cast<size_t>(send_result);
    }
    if (networking_check_ssl_after_send(ssl_connection) != 0)
        return (-1);
    return (0);
}

static void http_client_stream_state_init(http_stream_state &state, http_response_handler handler)
{
    state.header_buffer.clear();
    state.headers.clear();
    state.status_code = 0;
    state.headers_ready = false;
    state.keep_alive_allowed = false;
    state.has_content_length = false;
    state.expected_body_length = 0;
    state.received_body_length = 0;
    state.saw_chunked_encoding = false;
    state.http_1_1 = false;
    state.handler = handler;
    return ;
}

static int http_client_parse_status(const ft_string &headers)
{
    const char  *header_cstr;
    size_t      index;

    header_cstr = headers.c_str();
    index = 0;
    while (header_cstr[index] != '\0' && header_cstr[index] != ' ')
        index++;
    while (header_cstr[index] == ' ')
        index++;
    if (header_cstr[index] == '\0')
        return (0);
    return (ft_atoi(header_cstr + index));
}

static int http_client_stream_handle_header(http_stream_state &state)
{
    const char  *terminator;
    size_t      header_length;
    size_t      body_length;

    terminator = ft_strstr(state.header_buffer.c_str(), "\r\n\r\n");
    if (terminator == NULL)
        return (0);
    header_length = static_cast<size_t>(terminator - state.header_buffer.c_str()) + 4;
    state.headers = state.header_buffer;
    if (state.headers.size() > header_length)
        state.headers.erase(header_length, state.headers.size() - header_length);
    state.status_code = http_client_parse_status(state.headers);
    http_client_parse_header_metadata(state);
    body_length = state.header_buffer.size() - header_length;
    if (body_length > 0 && state.handler != NULL)
    {
        const char *body_ptr;
        size_t deliver_length;

        body_ptr = state.header_buffer.c_str() + header_length;
        deliver_length = body_length;
        if (state.has_content_length != false)
        {
            size_t remaining_bytes;

            if (state.expected_body_length > state.received_body_length)
                remaining_bytes = state.expected_body_length - state.received_body_length;
            else
                remaining_bytes = 0;
            if (deliver_length > remaining_bytes)
            {
                deliver_length = remaining_bytes;
                state.keep_alive_allowed = false;
            }
            state.received_body_length += deliver_length;
        }
        if (deliver_length > 0)
            state.handler(state.status_code, state.headers, body_ptr, deliver_length, false);
        if (deliver_length < body_length)
            state.keep_alive_allowed = false;
    }
    state.header_buffer.clear();
    state.headers_ready = true;
    return (1);
}

static int http_client_receive_stream(http_client_active_connection &connection,
    http_response_handler handler, bool &allow_keep_alive)
{
    char                buffer[1024];
    ssize_t             bytes_received;
    http_stream_state   state;
    int                 socket_fd;
    SSL                 *ssl_connection;
    bool                use_ssl;

    allow_keep_alive = false;
    if (handler == NULL)
    {
        return (-1);
    }
    http_client_stream_state_init(state, handler);
    socket_fd = connection.entry.socket_fd;
    ssl_connection = connection.entry.ssl_connection;
    use_ssl = connection.entry.use_ssl;
    while (1)
    {
        if (use_ssl != false)
            bytes_received = nw_ssl_read(ssl_connection, buffer, sizeof(buffer) - 1);
        else
            bytes_received = nw_recv(socket_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            if (state.headers_ready == false)
            {
                state.header_buffer.append(buffer, static_cast<size_t>(bytes_received));
                if (http_client_stream_handle_header(state) != 0)
                {
                    if (state.has_content_length != false
                        && state.received_body_length >= state.expected_body_length)
                        break;
                    continue ;
                }
            }
            else
            {
                size_t deliver_size;

                deliver_size = static_cast<size_t>(bytes_received);
                if (state.has_content_length != false)
                {
                    size_t remaining_bytes;

                    if (state.expected_body_length > state.received_body_length)
                        remaining_bytes = state.expected_body_length - state.received_body_length;
                    else
                        remaining_bytes = 0;
                    if (deliver_size > remaining_bytes)
                    {
                        deliver_size = remaining_bytes;
                        state.keep_alive_allowed = false;
                    }
                    state.received_body_length += deliver_size;
                }
                if (deliver_size > 0)
                    state.handler(state.status_code, state.headers, buffer, deliver_size, false);
            }
            if (state.has_content_length != false
                && state.received_body_length >= state.expected_body_length)
                break;
        }
        else
        {
            if (use_ssl != false)
            {
                int ssl_error;

                ssl_error = SSL_get_error(ssl_connection, static_cast<int>(bytes_received));
                if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE)
                {
                    int wait_result;
                    bool wait_for_write;

                    wait_for_write = (ssl_error == SSL_ERROR_WANT_WRITE);
                    wait_result = http_client_wait_for_socket_ready(socket_fd, wait_for_write);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (ssl_error == SSL_ERROR_ZERO_RETURN)
                {
                    state.keep_alive_allowed = false;
                    break;
                }
                if (ssl_error == SSL_ERROR_SYSCALL)
                {
#ifdef _WIN32
                    int last_error;

                    last_error = WSAGetLastError();
                    if (last_error == WSAEINTR)
                        continue ;
                    if (last_error == WSAEWOULDBLOCK)
                    {
                        int wait_result;

                        wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                        if (wait_result < 0)
                            return (-1);
                        continue ;
                    }
#else
                    int last_error;

                    last_error = errno;
                    if (last_error == EINTR)
                        continue ;
                    if (last_error == EWOULDBLOCK || last_error == EAGAIN)
                    {
                        int wait_result;

                        wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                        if (wait_result < 0)
                            return (-1);
                        continue ;
                    }
#endif
                    state.keep_alive_allowed = false;
                    return (-1);
                }
                state.keep_alive_allowed = false;
                return (-1);
            }
            if (bytes_received < 0)
            {
#ifdef _WIN32
                int last_error;

                last_error = WSAGetLastError();
                if (last_error == WSAEINTR)
                    continue ;
                if (last_error == WSAEWOULDBLOCK)
                {
                    int wait_result;

                    wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error == WSAECONNRESET)
                    break;
#else
                int last_error;

                last_error = errno;
                if (last_error == EINTR)
                    continue ;
                if (last_error == EWOULDBLOCK || last_error == EAGAIN)
                {
                    int wait_result;

                    wait_result = http_client_wait_for_socket_ready(socket_fd, false);
                    if (wait_result < 0)
                        return (-1);
                    continue ;
                }
                if (last_error == ECONNRESET)
                    break;
#endif
                state.keep_alive_allowed = false;
                return (-1);
            }
            state.keep_alive_allowed = false;
            break;
        }
    }
    if (state.headers_ready == false && state.header_buffer.empty() == false)
    {
        const char *body_ptr;

        body_ptr = state.header_buffer.c_str();
        state.handler(state.status_code, state.headers, body_ptr,
            state.header_buffer.size(), false);
    }
    state.handler(state.status_code, state.headers, "", 0, true);
    if (state.headers_ready != false && state.keep_alive_allowed != false
        && state.has_content_length != false
        && state.received_body_length >= state.expected_body_length)
        allow_keep_alive = true;
    else
        allow_keep_alive = false;
    return (0);
}

static void http_client_buffering_adapter(int status_code, const ft_string &headers,
    const char *body_chunk, size_t chunk_size, bool finished)
{
    if (g_http_buffer_adapter_state.response == NULL)
        return ;
    if (status_code != 0)
        g_http_buffer_adapter_state.status_code = status_code;
    if (g_http_buffer_adapter_state.header_appended == false && headers.empty() == false)
    {
        g_http_buffer_adapter_state.response->append(headers);
        g_http_buffer_adapter_state.header_appended = true;
    }
    if (chunk_size > 0)
    {
        g_http_buffer_adapter_state.response->append(body_chunk);
        g_http_buffer_adapter_state.body_bytes += chunk_size;
    }
    if (finished != false && status_code != 0)
        g_http_buffer_adapter_state.status_code = status_code;
    return ;
}

static void http_client_reset_buffer_adapter_state(void)
{
    g_http_buffer_adapter_state.response = NULL;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    return ;
}

static int http_client_finish_with_metrics(const char *method, const char *host,
    const char *path, size_t request_bytes, int result,
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
    error_code = FT_ERR_SUCCESS;
    if (result != 0)
        error_code = FT_ERR_IO;
    sample.labels.component = "http_client";
    sample.labels.operation = method;
    sample.labels.target = host;
    sample.labels.resource = path;
    sample.duration_ms = duration_ms;
    sample.request_bytes = request_bytes;
    sample.response_bytes = g_http_buffer_adapter_state.body_bytes;
    sample.status_code = g_http_buffer_adapter_state.status_code;
    sample.error_code = error_code;
    if (error_code == FT_ERR_SUCCESS)
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
    http_client_reset_buffer_adapter_state();
    return (result);
}

int http_get_stream(const char *host, const char *path, http_response_handler handler,
    bool use_ssl, const char *custom_port)
{
    const char *port_string;
    ft_string request;
    http_client_active_connection connection;
    bool allow_keep_alive;
    int attempt;

    if (handler == NULL)
    {
        return (-1);
    }
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    http_client_pool_reset_active(connection);
    attempt = 0;
    while (attempt < 2)
    {
        bool reused_connection;
        int send_result;
        int receive_result;

        if (http_client_pool_acquire_connection(host, port_string, use_ssl,
            connection, reused_connection) != 0)
            return (-1);
        if (http_client_establish_connection(host, port_string, use_ssl, connection) != 0)
        {
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (-1);
        }
        request.clear();
        request.append("GET ");
        request.append(path);
        request.append(" HTTP/1.1\r\nHost: ");
        request.append(host);
        request.append("\r\nConnection: keep-alive\r\n\r\n");
        if (use_ssl != false)
            send_result = http_client_send_ssl_request(connection.entry.ssl_connection,
                request.c_str(), request.size());
        else
            send_result = http_client_send_plain_request(connection.entry.socket_fd,
                request.c_str(), request.size());
        if (send_result != 0)
        {
            http_client_pool_disable_store(connection);
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (-1);
        }
        receive_result = http_client_receive_stream(connection, handler, allow_keep_alive);
        if (receive_result != 0)
        {
            http_client_pool_disable_store(connection);
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (-1);
        }
        http_client_pool_release_connection(connection, allow_keep_alive);
        return (0);
    }
    return (-1);
}

int http_get(const char *host, const char *path, ft_string &response, bool use_ssl, const char *custom_port)
{
    t_monotonic_time_point start_time;
    int result;

    start_time = time_monotonic_point_now();
    response.clear();
    g_http_buffer_adapter_state.response = &response;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    result = http_get_stream(host, path, http_client_buffering_adapter, use_ssl, custom_port);
    return (http_client_finish_with_metrics("GET", host, path, 0, result, start_time));
}

int http_post(const char *host, const char *path, const ft_string &body, ft_string &response, bool use_ssl, const char *custom_port)
{
    t_monotonic_time_point start_time;
    const char *port_string;
    ft_string request;
    char length_string[32];
    http_client_active_connection connection;
    bool allow_keep_alive;
    int attempt;
    size_t body_length;

    start_time = time_monotonic_point_now();
    response.clear();
    g_http_buffer_adapter_state.response = &response;
    g_http_buffer_adapter_state.header_appended = false;
    g_http_buffer_adapter_state.body_bytes = 0;
    g_http_buffer_adapter_state.status_code = 0;
    if (custom_port != NULL && custom_port[0] != '\0')
        port_string = custom_port;
    else if (use_ssl)
        port_string = "443";
    else
        port_string = "80";
    http_client_pool_reset_active(connection);
    body_length = body.size();
    std::snprintf(length_string, sizeof(length_string), "%zu", body_length);
    attempt = 0;
    while (attempt < 2)
    {
        bool reused_connection;
        int send_result;
        int receive_result;

        if (http_client_pool_acquire_connection(host, port_string, use_ssl,
            connection, reused_connection) != 0)
            return (http_client_finish_with_metrics("POST", host, path, body_length, -1, start_time));
        if (http_client_establish_connection(host, port_string, use_ssl, connection) != 0)
        {
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (http_client_finish_with_metrics("POST", host, path, body_length, -1, start_time));
        }
        request.clear();
        request.append("POST ");
        request.append(path);
        request.append(" HTTP/1.1\r\nHost: ");
        request.append(host);
        request.append("\r\nContent-Length: ");
        request.append(length_string);
        request.append("\r\nConnection: keep-alive\r\n\r\n");
        request.append(body);
        if (use_ssl != false)
            send_result = http_client_send_ssl_request(connection.entry.ssl_connection,
                request.c_str(), request.size());
        else
            send_result = http_client_send_plain_request(connection.entry.socket_fd,
                request.c_str(), request.size());
        if (send_result != 0)
        {
            http_client_pool_disable_store(connection);
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (http_client_finish_with_metrics("POST", host, path, body_length, -1, start_time));
        }
        receive_result = http_client_receive_stream(connection, http_client_buffering_adapter,
            allow_keep_alive);
        if (receive_result != 0)
        {
            http_client_pool_disable_store(connection);
            http_client_pool_release_connection(connection, false);
            if (reused_connection != false)
            {
                attempt++;
                continue ;
            }
            return (http_client_finish_with_metrics("POST", host, path, body_length, -1, start_time));
        }
        http_client_pool_release_connection(connection, allow_keep_alive);
        return (http_client_finish_with_metrics("POST", host, path, body_length, 0, start_time));
    }
    return (http_client_finish_with_metrics("POST", host, path, body_length, -1, start_time));
}


#endif
