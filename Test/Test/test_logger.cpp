#include "../../Logger/logger.hpp"
#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../Networking/networking.hpp"
#include "../../Errno/errno.hpp"
#include <unistd.h>

static size_t g_network_sink_partial_call_count = 0;
static size_t g_network_sink_partial_total_bytes = 0;
static ssize_t logger_network_partial_send_stub(int socket_fd, const void *buffer,
                                               size_t length, int flags)
{
    size_t chunk_size;

    (void)socket_fd;
    (void)buffer;
    (void)flags;
    g_network_sink_partial_call_count++;
    chunk_size = length;
    if (g_network_sink_partial_call_count == 1 && length > 4)
        chunk_size = 4;
    else if (g_network_sink_partial_call_count == 2 && length > 2)
        chunk_size = 2;
    g_network_sink_partial_total_bytes += chunk_size;
    return (static_cast<ssize_t>(chunk_size));
}

FT_TEST(test_logger_color_toggle, "logger color toggle")
{
    ft_log_set_color(false);
    FT_ASSERT_EQ(false, ft_log_get_color());
    ft_log_set_color(true);
    FT_ASSERT_EQ(true, ft_log_get_color());
    return (1);
}

FT_TEST(test_logger_json_sink, "logger json sink")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[256];

    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    ft_log_info("json check");
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(buffer[0] == '{');
    FT_ASSERT(ft_strstr(buffer, "\"level\":\"INFO\"") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_fd);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}

FT_TEST(test_logger_network_sink_retries_partial, "logger network sink retries partial send")
{
    s_network_sink network_sink;
    const char    *message;
    size_t         expected_length;

    g_network_sink_partial_call_count = 0;
    g_network_sink_partial_total_bytes = 0;
    network_sink.socket_fd = 10;
    message = "network sink partial message";
    expected_length = ft_strlen(message);
    ft_errno = ER_SUCCESS;
    nw_set_send_stub(&logger_network_partial_send_stub);
    ft_network_sink(message, &network_sink);
    nw_set_send_stub(NULL);
    FT_ASSERT_EQ(expected_length, g_network_sink_partial_total_bytes);
    FT_ASSERT_EQ(static_cast<size_t>(3), g_network_sink_partial_call_count);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_logger_network_sink_handles_error, "logger network sink handles send error")
{
    s_network_sink network_sink;

    network_sink.socket_fd = -1;
    ft_errno = ER_SUCCESS;
    ft_network_sink("network sink failure", &network_sink);
    FT_ASSERT_EQ(SOCKET_SEND_FAILED, ft_errno);
    ft_errno = ER_SUCCESS;
    return (1);
}

