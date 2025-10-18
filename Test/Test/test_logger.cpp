#include "../../Logger/logger.hpp"
#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/system_utils.hpp"
#include <unistd.h>

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

FT_TEST(test_logger_structured_logging, "structured logging emits structured payloads")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[512];
    s_log_field fields[3];

    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    fields[0].key = "user";
    fields[0].value = "alice";
    fields[1].key = "ip";
    fields[1].value = "127.0.0.1";
    fields[2].key = "attempts";
    fields[2].value = ft_nullptr;
    ft_log_info_structured("login", fields, 3);
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "\"message\":{\"message\":\"login\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\"user\":\"alice\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\"ip\":\"127.0.0.1\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\"attempts\":null") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_fd);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}

FT_TEST(test_logger_context_prefixes_plain_logs, "context guard prefixes plain logs")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[512];
    s_log_field context_fields[3];

    ft_log_set_color(false);
    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    context_fields[0].key = "request_id";
    context_fields[0].value = "abc123";
    context_fields[1].key = "customer";
    context_fields[1].value = "premium";
    context_fields[2].key = "trace";
    context_fields[2].value = ft_nullptr;
    {
        ft_log_context_guard guard(context_fields, 3);

        FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
        ft_log_info("processing order");
    }
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "request_id=abc123") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "customer=premium") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "trace]") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_fd);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}

FT_TEST(test_logger_context_enriches_structured_logs, "context guard augments structured payloads")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[512];
    s_log_field base_fields[1];
    s_log_field context_fields[2];

    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    base_fields[0].key = "operation";
    base_fields[0].value = "sync";
    context_fields[0].key = "request_id";
    context_fields[0].value = "ctx-42";
    context_fields[1].key = "attempt";
    context_fields[1].value = ft_nullptr;
    {
        ft_log_context_guard guard(context_fields, 2);

        FT_ASSERT_EQ(ER_SUCCESS, guard.get_error());
        ft_log_info_structured("sync", base_fields, 1);
    }
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "\\\"operation\\\":\\\"sync\\\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\\\"request_id\\\":\\\"ctx-42\\\"") != ft_nullptr);
    FT_ASSERT(ft_strstr(buffer, "\\\"attempt\\\":null") != ft_nullptr);
    ft_log_remove_sink(ft_json_sink, &write_fd);
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}

FT_TEST(test_logger_redaction_masks_plain_logs, "redaction helpers replace sensitive substrings")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[512];

    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    ft_log_clear_redactions();
    FT_ASSERT_EQ(0, ft_log_add_redaction("token=abcd"));
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    ft_log_info("issuing %s", "token=abcd");
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "[REDACTED]") != ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(buffer, "token=abcd"));
    ft_log_remove_sink(ft_json_sink, &write_fd);
    ft_log_clear_redactions();
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}

FT_TEST(test_logger_redaction_masks_structured_fields, "structured payloads honor redaction helpers")
{
    int pipe_fds[2];
    int write_fd;
    ssize_t read_count;
    char buffer[512];
    s_log_field fields[2];

    FT_ASSERT_EQ(0, pipe(pipe_fds));
    write_fd = pipe_fds[1];
    ft_log_clear_redactions();
    FT_ASSERT_EQ(0, ft_log_add_redaction_with_replacement("secret-token", "***"));
    FT_ASSERT_EQ(0, ft_log_add_sink(ft_json_sink, &write_fd));
    fields[0].key = "session";
    fields[0].value = "secret-token";
    fields[1].key = "status";
    fields[1].value = "ok";
    ft_log_info_structured("login", fields, 2);
    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(ft_strstr(buffer, "***") != ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, ft_strstr(buffer, "secret-token"));
    ft_log_remove_sink(ft_json_sink, &write_fd);
    ft_log_clear_redactions();
    close(pipe_fds[0]);
    close(pipe_fds[1]);
    return (1);
}
