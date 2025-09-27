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
    return ;
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
    FT_ASSERT_EQ(SOCKET_SEND_FAILED, failure_errno);
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

