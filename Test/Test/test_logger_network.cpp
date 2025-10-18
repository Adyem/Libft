#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include <string>

struct mock_network_send_state
{
    std::string transmitted_data;
    size_t      chunk_size;
    int         call_count;
    int         failure_call_index;
    bool        failure_returns_zero;
};

static mock_network_send_state g_mock_network_send_state = {"", 0, 0, 0, false};

static s_network_sink *g_sink_to_remove = ft_nullptr;

static ssize_t mock_partial_send(int socket_fd, const void *buffer, size_t length, int flags)
{
    (void)socket_fd;
    (void)flags;

    g_mock_network_send_state.call_count++;
    if (g_mock_network_send_state.failure_call_index > 0
        && g_mock_network_send_state.call_count == g_mock_network_send_state.failure_call_index)
    {
        if (g_mock_network_send_state.failure_returns_zero)
            return (0);
        return (-1);
    }
    size_t allowed_chunk;

    allowed_chunk = g_mock_network_send_state.chunk_size;
    if (allowed_chunk == 0 || allowed_chunk > length)
        allowed_chunk = length;
    g_mock_network_send_state.transmitted_data.append(static_cast<const char *>(buffer), allowed_chunk);
    return (static_cast<ssize_t>(allowed_chunk));
}

static void reset_mock_network_send_state(void)
{
    g_mock_network_send_state.transmitted_data.clear();
    g_mock_network_send_state.chunk_size = 0;
    g_mock_network_send_state.call_count = 0;
    g_mock_network_send_state.failure_call_index = 0;
    g_mock_network_send_state.failure_returns_zero = false;
    g_sink_to_remove = ft_nullptr;
    return ;
}

static ssize_t mock_send_remove_sink(int socket_fd, const void *buffer, size_t length, int flags)
{
    (void)socket_fd;
    (void)buffer;
    (void)length;
    (void)flags;

    if (g_sink_to_remove)
    {
        s_network_sink *sink_to_delete;

        sink_to_delete = g_sink_to_remove;
        g_sink_to_remove = ft_nullptr;
        ft_log_remove_sink(ft_network_sink, sink_to_delete);
        delete sink_to_delete;
    }
    ft_errno = FT_ERR_SOCKET_SEND_FAILED;
    return (-1);
}

static ssize_t mock_successful_send(int socket_fd, const void *buffer, size_t length, int flags)
{
    (void)socket_fd;
    (void)buffer;
    (void)flags;

    ft_errno = ER_SUCCESS;
    return (static_cast<ssize_t>(length));
}

FT_TEST(test_logger_network_sink_handles_partial_writes, "logger network sink handles partial writes")
{
    s_network_sink sink;
    const char *log_message;
    size_t expected_length;
    int observed_errno;

    reset_mock_network_send_state();
    g_mock_network_send_state.chunk_size = 5;
    sink.socket_fd = 123;
    sink.send_function = mock_partial_send;
    log_message = "partial write message ensures loop execution";
    ft_errno = ER_SUCCESS;
    ft_network_sink(log_message, &sink);
    observed_errno = ft_errno;
    expected_length = ft_strlen(log_message);
    FT_ASSERT_EQ(ER_SUCCESS, observed_errno);
    FT_ASSERT_EQ(123, sink.socket_fd);
    FT_ASSERT(g_mock_network_send_state.call_count > 1);
    FT_ASSERT_EQ(expected_length, g_mock_network_send_state.transmitted_data.size());
    FT_ASSERT(g_mock_network_send_state.transmitted_data == log_message);
    return (1);
}

FT_TEST(test_logger_network_sink_handles_send_failure, "logger network sink handles send failure")
{
    s_network_sink sink;
    const char *log_message;
    std::string expected_prefix;
    size_t prefix_length;

    reset_mock_network_send_state();
    g_mock_network_send_state.chunk_size = 6;
    g_mock_network_send_state.failure_call_index = 2;
    sink.socket_fd = 321;
    sink.send_function = mock_partial_send;
    log_message = "network sink failure path";
    ft_errno = ER_SUCCESS;
    ft_network_sink(log_message, &sink);
    int failure_errno = ft_errno;
    prefix_length = g_mock_network_send_state.chunk_size;
    expected_prefix.assign(log_message, prefix_length);
    FT_ASSERT_EQ(FT_ERR_SOCKET_SEND_FAILED, failure_errno);
    FT_ASSERT_EQ(-1, sink.socket_fd);
    FT_ASSERT(sink.send_function == ft_nullptr);
    FT_ASSERT_EQ(prefix_length, g_mock_network_send_state.transmitted_data.size());
    FT_ASSERT(g_mock_network_send_state.transmitted_data == expected_prefix);
    return (1);
}

FT_TEST(test_logger_network_sink_handles_zero_byte_send, "logger network sink handles zero byte send")
{
    s_network_sink sink;
    const char *log_message;
    size_t transmitted_length;
    size_t message_length;
    int observed_errno;

    reset_mock_network_send_state();
    g_mock_network_send_state.chunk_size = 7;
    g_mock_network_send_state.failure_call_index = 2;
    g_mock_network_send_state.failure_returns_zero = true;
    sink.socket_fd = 555;
    sink.send_function = mock_partial_send;
    log_message = "network sink zero send failure";
    ft_errno = ER_SUCCESS;
    ft_network_sink(log_message, &sink);
    observed_errno = ft_errno;
    transmitted_length = g_mock_network_send_state.transmitted_data.size();
    message_length = ft_strlen(log_message);
    FT_ASSERT(observed_errno != ER_SUCCESS);
    FT_ASSERT_EQ(-1, sink.socket_fd);
    FT_ASSERT(sink.send_function == ft_nullptr);
    FT_ASSERT(transmitted_length > 0);
    FT_ASSERT(transmitted_length < message_length);
    return (1);
}

FT_TEST(test_logger_remote_health_probe_records_success, "remote health probe reports reachable sinks")
{
    s_network_sink *sink;
    s_log_remote_health status;
    size_t status_count;

    reset_mock_network_send_state();
    g_mock_network_send_state.chunk_size = 8;
    sink = new s_network_sink();
    FT_ASSERT(sink != ft_nullptr);
    sink->socket_fd = 777;
    sink->send_function = mock_partial_send;
    sink->host = "127.0.0.1";
    FT_ASSERT_EQ(ER_SUCCESS, sink->host.get_error());
    sink->port = 5555;
    sink->use_tcp = true;
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, sink));
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    status_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(&status, 1, &status_count));
    FT_ASSERT_EQ(static_cast<size_t>(1), status_count);
    FT_ASSERT(status.reachable);
    FT_ASSERT_EQ(ER_SUCCESS, status.last_error);
    FT_ASSERT(status.host != ft_nullptr);
    FT_ASSERT(ft_strcmp(status.host, "127.0.0.1") == 0);
    FT_ASSERT_EQ(5555, status.port);
    FT_ASSERT(status.use_tcp);
    FT_ASSERT(status.last_check != 0);
    ft_log_remove_sink(ft_network_sink, sink);
    delete sink;
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    status_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(ft_nullptr, 0, &status_count));
    FT_ASSERT_EQ(static_cast<size_t>(0), status_count);
    return (1);
}

FT_TEST(test_logger_remote_health_probe_records_failure, "remote health probe surfaces send errors")
{
    s_network_sink *sink;
    s_log_remote_health status;
    size_t status_count;

    reset_mock_network_send_state();
    g_mock_network_send_state.chunk_size = 6;
    g_mock_network_send_state.failure_call_index = 1;
    sink = new s_network_sink();
    FT_ASSERT(sink != ft_nullptr);
    sink->socket_fd = 888;
    sink->send_function = mock_partial_send;
    sink->host = "10.0.0.5";
    FT_ASSERT_EQ(ER_SUCCESS, sink->host.get_error());
    sink->port = 6000;
    sink->use_tcp = false;
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, sink));
    int probe_result;
    probe_result = ft_log_probe_remote_health();
    FT_ASSERT_EQ(-1, probe_result);
    FT_ASSERT_EQ(FT_ERR_SOCKET_SEND_FAILED, ft_errno);
    status_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(&status, 1, &status_count));
    FT_ASSERT_EQ(static_cast<size_t>(1), status_count);
    FT_ASSERT(!status.reachable);
    FT_ASSERT_EQ(FT_ERR_SOCKET_SEND_FAILED, status.last_error);
    FT_ASSERT(status.last_check != 0);
    FT_ASSERT(status.host != ft_nullptr);
    FT_ASSERT(ft_strcmp(status.host, "10.0.0.5") == 0);
    FT_ASSERT_EQ(6000, status.port);
    FT_ASSERT(!status.use_tcp);
    ft_log_remove_sink(ft_network_sink, sink);
    delete sink;
    return (1);
}

FT_TEST(test_logger_remote_health_probe_handles_removed_sink, "remote health probe tolerates sink removal during send")
{
    s_network_sink *sink;
    s_log_remote_health status;
    size_t status_count;

    reset_mock_network_send_state();
    sink = new s_network_sink();
    FT_ASSERT(sink != ft_nullptr);
    sink->socket_fd = 4321;
    sink->send_function = mock_send_remove_sink;
    sink->host = "removed.example.com";
    FT_ASSERT_EQ(ER_SUCCESS, sink->host.get_error());
    sink->port = 9001;
    sink->use_tcp = true;
    g_sink_to_remove = sink;
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, sink));
    FT_ASSERT_EQ(-1, ft_log_probe_remote_health());
    FT_ASSERT_EQ(FT_ERR_SOCKET_SEND_FAILED, ft_errno);
    status_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(&status, 1, &status_count));
    FT_ASSERT_EQ(static_cast<size_t>(0), status_count);
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_logger_remote_health_probe_keeps_other_sinks_intact, "remote health probe keeps other sinks intact after removal")
{
    s_network_sink *removed_sink;
    s_network_sink *healthy_sink;
    s_log_remote_health status;
    size_t status_count;

    reset_mock_network_send_state();
    removed_sink = new s_network_sink();
    healthy_sink = new s_network_sink();
    FT_ASSERT(removed_sink != ft_nullptr);
    FT_ASSERT(healthy_sink != ft_nullptr);
    removed_sink->socket_fd = 1111;
    removed_sink->send_function = mock_send_remove_sink;
    removed_sink->host = "failing.example.com";
    FT_ASSERT_EQ(ER_SUCCESS, removed_sink->host.get_error());
    removed_sink->port = 7777;
    removed_sink->use_tcp = true;
    healthy_sink->socket_fd = 2222;
    healthy_sink->send_function = mock_successful_send;
    healthy_sink->host = "healthy.example.com";
    FT_ASSERT_EQ(ER_SUCCESS, healthy_sink->host.get_error());
    healthy_sink->port = 8888;
    healthy_sink->use_tcp = false;
    g_sink_to_remove = removed_sink;
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, removed_sink));
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, healthy_sink));
    FT_ASSERT_EQ(-1, ft_log_probe_remote_health());
    FT_ASSERT_EQ(FT_ERR_SOCKET_SEND_FAILED, ft_errno);
    status_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(&status, 1, &status_count));
    FT_ASSERT_EQ(static_cast<size_t>(1), status_count);
    FT_ASSERT(status.host != ft_nullptr);
    FT_ASSERT(ft_strcmp(status.host, "healthy.example.com") == 0);
    FT_ASSERT_EQ(8888, status.port);
    FT_ASSERT(!status.use_tcp);
    FT_ASSERT(status.last_check != 0);
    FT_ASSERT(status.reachable);
    FT_ASSERT_EQ(ER_SUCCESS, status.last_error);
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_remove_sink(ft_network_sink, healthy_sink);
    delete healthy_sink;
    return (1);
}

FT_TEST(test_logger_remote_health_enable_toggle, "remote health enable toggles without sinks")
{
    ft_log_enable_remote_health(false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_logger_remote_health_interval_updates, "remote health interval updates while running")
{
    ft_log_enable_remote_health(false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_set_remote_health_interval(0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_set_remote_health_interval(5);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_set_remote_health_interval(1);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_log_enable_remote_health(false);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_logger_remote_health_get_requires_capacity, "remote health get enforces capacity limits")
{
    s_network_sink *first_sink;
    s_network_sink *second_sink;
    s_log_remote_health statuses[1];
    size_t status_count;
    size_t peek_count;

    reset_mock_network_send_state();
    first_sink = new s_network_sink();
    FT_ASSERT(first_sink != ft_nullptr);
    first_sink->socket_fd = 3333;
    first_sink->send_function = mock_successful_send;
    first_sink->host = "capacity-one.example";
    FT_ASSERT_EQ(ER_SUCCESS, first_sink->host.get_error());
    first_sink->port = 1000;
    first_sink->use_tcp = true;
    second_sink = new s_network_sink();
    FT_ASSERT(second_sink != ft_nullptr);
    second_sink->socket_fd = 4444;
    second_sink->send_function = mock_successful_send;
    second_sink->host = "capacity-two.example";
    FT_ASSERT_EQ(ER_SUCCESS, second_sink->host.get_error());
    second_sink->port = 2000;
    second_sink->use_tcp = false;
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, first_sink));
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_network_sink, second_sink));
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    status_count = 0;
    statuses[0].host = "retain";
    statuses[0].port = 9999;
    statuses[0].use_tcp = true;
    statuses[0].reachable = true;
    statuses[0].last_check = 123;
    statuses[0].last_error = ER_SUCCESS;
    int get_result;

    get_result = ft_log_get_remote_health(statuses, 1, &status_count);
    FT_ASSERT_EQ(-1, get_result);
    FT_ASSERT(status_count >= static_cast<size_t>(2));
    FT_ASSERT(statuses[0].host != ft_nullptr);
    FT_ASSERT(ft_strcmp(statuses[0].host, "retain") == 0);
    FT_ASSERT_EQ(static_cast<unsigned short>(9999), statuses[0].port);
    FT_ASSERT(statuses[0].use_tcp);
    FT_ASSERT(statuses[0].reachable);
    peek_count = 0;
    FT_ASSERT_EQ(0, ft_log_get_remote_health(ft_nullptr, 0, &peek_count));
    FT_ASSERT_EQ(static_cast<size_t>(2), peek_count);
    ft_log_remove_sink(ft_network_sink, first_sink);
    ft_log_remove_sink(ft_network_sink, second_sink);
    delete first_sink;
    delete second_sink;
    FT_ASSERT_EQ(0, ft_log_probe_remote_health());
    return (1);
}

