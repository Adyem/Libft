#include "../../Logger/logger.hpp"
#include "../../Logger/logger_internal.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
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

FT_TEST(test_logger_set_file_missing_directory, "ft_log_set_file returns errno for missing directory")
{
    const char *directory_path;
    const char *file_path;
    int result;

    directory_path = "/tmp/libft_logger_missing_dir";
    file_path = "/tmp/libft_logger_missing_dir/log.txt";
    (void)rmdir(directory_path);
    errno = 0;
    ft_errno = ER_SUCCESS;
    result = ft_log_set_file(file_path, 1024);
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(ENOENT + ERRNO_OFFSET, ft_errno);
    ft_errno = ER_SUCCESS;
    return (1);
}

FT_TEST(test_logger_rotate_fstat_failure_sets_errno, "ft_log_rotate reports fstat failure")
{
    s_file_sink sink;

    sink.fd = -1;
    sink.path = ft_string("/tmp/libft_logger_invalid_fd");
    FT_ASSERT_EQ(ER_SUCCESS, sink.path.get_error());
    sink.max_size = 1;
    errno = 0;
    ft_errno = ER_SUCCESS;
    ft_log_rotate(&sink);
    FT_ASSERT_EQ(EBADF + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_logger_rotate_success_clears_errno, "ft_log_rotate clears errno after successful rotation")
{
    char        template_path[] = "/tmp/libft_logger_rotate_XXXXXX";
    int         temp_fd;
    ssize_t     write_result;
    s_file_sink sink;
    ft_string   rotated_path;

    temp_fd = mkstemp(template_path);
    FT_ASSERT(temp_fd >= 0);
    write_result = write(temp_fd, "rotation-test", 13);
    FT_ASSERT_EQ(13, write_result);
    sink.fd = temp_fd;
    sink.path = ft_string(template_path);
    FT_ASSERT_EQ(ER_SUCCESS, sink.path.get_error());
    sink.max_size = 4;
    ft_errno = FT_EINVAL;
    errno = 0;
    ft_log_rotate(&sink);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(sink.fd >= 0);
    rotated_path = sink.path + ".1";
    FT_ASSERT_EQ(ER_SUCCESS, rotated_path.get_error());
    FT_ASSERT_EQ(0, access(rotated_path.c_str(), F_OK));
    close(sink.fd);
    unlink(template_path);
    unlink(rotated_path.c_str());
    return (1);
}

FT_TEST(test_ft_log_set_level_updates_global_threshold, "ft_log_set_level updates g_level and clears errno")
{
    t_log_level previous_level;
    int         previous_errno_value;

    previous_level = g_level;
    previous_errno_value = ft_errno;
    ft_errno = FT_EINVAL;
    ft_log_set_level(LOG_LEVEL_ERROR);
    FT_ASSERT_EQ(LOG_LEVEL_ERROR, g_level);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    g_level = previous_level;
    ft_errno = previous_errno_value;
    return (1);
}

FT_TEST(test_ft_log_alloc_logging_without_global_logger, "ft_log_set_alloc_logging handles missing global logger")
{
    ft_logger *previous_logger;
    int         previous_errno_value;

    previous_logger = g_logger;
    FT_ASSERT(previous_logger == ft_nullptr);
    g_logger = ft_nullptr;
    previous_errno_value = ft_errno;
    ft_errno = FT_EINVAL;
    ft_log_set_alloc_logging(true);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(false, ft_log_get_alloc_logging());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    g_logger = previous_logger;
    ft_errno = previous_errno_value;
    return (1);
}

FT_TEST(test_ft_log_global_helpers_forward_to_logger, "ft_log_* helpers forward to the active logger instance")
{
    ft_logger  *previous_logger;
    int         previous_errno_value;

    previous_logger = g_logger;
    FT_ASSERT(previous_logger == ft_nullptr);
    previous_errno_value = ft_errno;
    {
        ft_logger logger_instance;

        logger_instance.set_global();
        ft_errno = FT_EINVAL;
        ft_log_set_alloc_logging(true);
        FT_ASSERT_EQ(true, logger_instance.get_alloc_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_EINVAL;
        FT_ASSERT_EQ(true, ft_log_get_alloc_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_EINVAL;
        ft_log_set_api_logging(true);
        FT_ASSERT_EQ(true, logger_instance.get_api_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_errno = FT_EINVAL;
        FT_ASSERT_EQ(true, ft_log_get_api_logging());
        FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

        ft_log_set_alloc_logging(false);
        FT_ASSERT_EQ(false, logger_instance.get_alloc_logging());
        ft_log_set_api_logging(false);
        FT_ASSERT_EQ(false, logger_instance.get_api_logging());

        g_logger = previous_logger;
    }
    ft_errno = previous_errno_value;
    return (1);
}

