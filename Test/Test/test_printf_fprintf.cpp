#include "../../Printf/printf.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstdarg>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#if defined(_WIN32) || defined(_WIN64)
# include <io.h>
#endif

static int create_pipe(int pipe_fds[2])
{
    if (pipe(pipe_fds) != 0)
        return (0);
    return (1);
}

static int close_pipe_end(int file_descriptor)
{
    if (close(file_descriptor) != 0)
        return (0);
    return (1);
}

static int read_pipe_into_buffer(int read_fd, char *buffer, size_t buffer_size, ssize_t *bytes_read)
{
    if (buffer == ft_nullptr || bytes_read == ft_nullptr)
        return (0);
    *bytes_read = read(read_fd, buffer, buffer_size);
    if (*bytes_read < 0)
        return (0);
    return (1);
}

static int call_ft_vfprintf(FILE *stream, const char *format, ...)
{
    va_list arguments;
    int result;

    va_start(arguments, format);
    result = ft_vfprintf(stream, format, arguments);
    va_end(arguments);
    return (result);
}

FT_TEST(test_ft_vfprintf_writes_output, "ft_vfprintf formats text into the provided stream")
{
    int pipe_fds[2];
    FILE *stream;
    ssize_t bytes_read;
    char buffer[64];
    char word[] = "done";

    FT_ASSERT(create_pipe(pipe_fds));
    stream = fdopen(pipe_fds[1], "w");
    if (stream == ft_nullptr)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    int printed = call_ft_vfprintf(stream, "Value:%d %s!", 42, word);
    FT_ASSERT_EQ(14, printed);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, fflush(stream));
    FT_ASSERT_EQ(0, fclose(stream));
    stream = static_cast<FILE *>(ft_nullptr);
    pipe_fds[1] = -1;
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Value:42 done!"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_vfprintf_null_arguments_return_error, "ft_vfprintf rejects null stream or format")
{
    int pipe_fds[2];
    FILE *stream;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, call_ft_vfprintf(static_cast<FILE *>(ft_nullptr), "noop"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT(create_pipe(pipe_fds));
    stream = fdopen(pipe_fds[1], "w");
    if (stream == ft_nullptr)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        return (0);
    }
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, call_ft_vfprintf(stream, static_cast<const char *>(ft_nullptr)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(0, fclose(stream));
    stream = static_cast<FILE *>(ft_nullptr);
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_vfprintf_write_failure_sets_errno, "ft_vfprintf propagates stream write failures")
{
    FILE *stream;

    stream = std::tmpfile();
    if (stream == ft_nullptr)
        return (0);
#if defined(_WIN32) || defined(_WIN64)
    int descriptor = _fileno(stream);
    if (descriptor != -1)
        _close(descriptor);
#else
    int descriptor = fileno(stream);
    if (descriptor != -1)
        close(descriptor);
#endif
    ft_errno = ER_SUCCESS;
    int printed = call_ft_vfprintf(stream, "%s", "fail");
    FT_ASSERT_EQ(-1, printed);
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, ft_errno);
    fclose(stream);
    return (1);
}

FT_TEST(test_ft_fprintf_writes_and_counts, "ft_fprintf forwards to ft_vfprintf and returns byte count")
{
    int pipe_fds[2];
    FILE *stream;
    ssize_t bytes_read;
    char buffer[64];
    char name[] = "world";

    FT_ASSERT(create_pipe(pipe_fds));
    stream = fdopen(pipe_fds[1], "w");
    if (stream == ft_nullptr)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    int printed = ft_fprintf(stream, "Hello %s", name);
    FT_ASSERT_EQ(11, printed);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, fflush(stream));
    FT_ASSERT_EQ(0, fclose(stream));
    stream = static_cast<FILE *>(ft_nullptr);
    pipe_fds[1] = -1;
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Hello world"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_ft_fprintf_null_arguments_return_error, "ft_fprintf rejects null stream or format")
{
    int pipe_fds[2];
    FILE *stream;
    typedef int (*t_ft_fprintf_plain)(FILE *, const char *, ...);
    t_ft_fprintf_plain plain_ft_fprintf;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_fprintf(static_cast<FILE *>(ft_nullptr), "noop"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT(create_pipe(pipe_fds));
    stream = fdopen(pipe_fds[1], "w");
    if (stream == ft_nullptr)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        return (0);
    }
    ft_errno = ER_SUCCESS;
    plain_ft_fprintf = ft_fprintf;
    FT_ASSERT_EQ(-1, plain_ft_fprintf(stream, static_cast<const char *>(ft_nullptr)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(0, fclose(stream));
    stream = static_cast<FILE *>(ft_nullptr);
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_pf_printf_writes_to_stdout, "pf_printf writes formatted output to STDOUT")
{
    int pipe_fds[2];
    int stdout_backup;
    ssize_t bytes_read;
    char buffer[64];
    char status[] = "done";

    FT_ASSERT(create_pipe(pipe_fds));
    stdout_backup = dup(STDOUT_FILENO);
    if (stdout_backup < 0)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        return (0);
    }
    if (dup2(pipe_fds[1], STDOUT_FILENO) < 0)
    {
        close_pipe_end(pipe_fds[0]);
        close_pipe_end(pipe_fds[1]);
        close(stdout_backup);
        return (0);
    }
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    int printed = pf_printf("Sum=%d %s", 7, status);
    FT_ASSERT_EQ(10, printed);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, fflush(stdout));
    FT_ASSERT(close_pipe_end(pipe_fds[1]));
    pipe_fds[1] = -1;
    FT_ASSERT(dup2(stdout_backup, STDOUT_FILENO) >= 0);
    close(stdout_backup);
    FT_ASSERT(read_pipe_into_buffer(pipe_fds[0], buffer, sizeof(buffer) - 1, &bytes_read));
    FT_ASSERT(bytes_read >= 0);
    buffer[static_cast<size_t>(bytes_read)] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "Sum=7 done"));
    FT_ASSERT(close_pipe_end(pipe_fds[0]));
    return (1);
}

FT_TEST(test_pf_printf_null_format_sets_errno, "pf_printf rejects null format strings")
{
    typedef int (*t_pf_printf_plain)(const char *, ...);
    t_pf_printf_plain plain_pf_printf;

    ft_errno = ER_SUCCESS;
    plain_pf_printf = pf_printf;
    FT_ASSERT_EQ(-1, plain_pf_printf(static_cast<const char *>(ft_nullptr)));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
