#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <cerrno>
#include <cstring>
#include <string>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

static bool capture_ft_exit_output(const char *message, int errno_value, int exit_code,
        std::string &output, int &child_status)
{
    int pipe_fds[2];
    pid_t child_process_id;
    char buffer[256];
    ssize_t bytes_read;

    output.clear();
    child_status = -1;
    if (pipe(pipe_fds) != 0)
        return (false);
    child_process_id = fork();
    if (child_process_id < 0)
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return (false);
    }
    if (child_process_id == 0)
    {
        close(pipe_fds[0]);
        if (dup2(pipe_fds[1], 2) < 0)
            _exit(127);
        close(pipe_fds[1]);
        ft_errno = errno_value;
        ft_exit(message, exit_code);
        _exit(127);
    }
    close(pipe_fds[1]);
    while (true)
    {
        bytes_read = read(pipe_fds[0], buffer, sizeof(buffer));
        if (bytes_read > 0)
        {
            output.append(buffer, static_cast<size_t>(bytes_read));
        }
        else if (bytes_read == 0)
        {
            break ;
        }
        else if (errno == EINTR)
        {
            continue ;
        }
        else
        {
            close(pipe_fds[0]);
            waitpid(child_process_id, &child_status, 0);
            return (false);
        }
    }
    close(pipe_fds[0]);
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (false);
    return (true);
}

FT_TEST(test_ft_strerror_errno_message, "ft_strerror returns standard errno message")
{
    const char *expected_message;
    const char *actual_message;
    int         previous_errno;

    expected_message = strerror(EINVAL);
    previous_errno = FT_EINVAL;
    ft_errno = previous_errno;
    actual_message = ft_strerror(EINVAL + ERRNO_OFFSET);
    FT_ASSERT(expected_message != NULL);
    FT_ASSERT(actual_message != NULL);
    FT_ASSERT_EQ(0, std::strcmp(expected_message, actual_message));
    FT_ASSERT_EQ(previous_errno, ft_errno);
    return (1);
}

FT_TEST(test_ft_errno_memcpy_tracks_error_state, "ft_memcpy updates ft_errno on failure and success")
{
    char        source_buffer[] = "data";
    char        destination_buffer[5];
    void       *copy_result;

    ft_errno = ER_SUCCESS;
    copy_result = ft_memcpy(ft_nullptr, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    copy_result = ft_memcpy(destination_buffer, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == destination_buffer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, std::memcmp(destination_buffer, source_buffer, sizeof(source_buffer)));
    return (1);
}

FT_TEST(test_ft_errno_strlen_resets_after_failure, "ft_strlen resets ft_errno to success for valid input")
{
    const char *valid_string;
    int         string_length;

    valid_string = "example";
    ft_errno = ER_SUCCESS;
    string_length = ft_strlen(ft_nullptr);
    FT_ASSERT_EQ(0, string_length);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    string_length = ft_strlen(valid_string);
    FT_ASSERT_EQ(7, string_length);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_errno_is_thread_local, "ft_errno maintains independent values per thread")
{
    std::atomic<int> thread_errno_value;
    std::thread      worker_thread;

    thread_errno_value.store(ER_SUCCESS);
    ft_errno = FT_EINVAL;
    worker_thread = std::thread(
        [&thread_errno_value]()
        {
            ft_errno = ER_SUCCESS;
            ft_errno = FT_ERANGE;
            thread_errno_value.store(ft_errno);
            return ;
        });
    worker_thread.join();
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(FT_ERANGE, thread_errno_value.load());
    return (1);
}

FT_TEST(test_ft_perror_null_message_outputs_errno, "ft_perror prints strerror when message is null")
{
    int     pipe_fds[2];
    int     saved_stderr;
    ssize_t read_count;
    char    buffer[256];
    int     original_errno_value;
    const char *expected_message;

    original_errno_value = ft_errno;
    expected_message = ft_strerror(FT_EINVAL);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(0, pipe(pipe_fds));
    saved_stderr = dup(2);
    FT_ASSERT(saved_stderr >= 0);
    FT_ASSERT(dup2(pipe_fds[1], 2) >= 0);
    close(pipe_fds[1]);

    ft_perror(ft_nullptr);

    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(std::strstr(buffer, expected_message) != ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    FT_ASSERT(dup2(saved_stderr, 2) >= 0);
    close(saved_stderr);
    close(pipe_fds[0]);
    ft_errno = original_errno_value;
    return (1);
}

FT_TEST(test_ft_perror_prefixes_custom_message, "ft_perror prefixes custom text before strerror output")
{
    int     pipe_fds[2];
    int     saved_stderr;
    ssize_t read_count;
    char    buffer[256];
    int     original_errno_value;
    const char *expected_message;

    original_errno_value = ft_errno;
    expected_message = ft_strerror(FT_EIO);
    ft_errno = FT_EIO;
    FT_ASSERT_EQ(0, pipe(pipe_fds));
    saved_stderr = dup(2);
    FT_ASSERT(saved_stderr >= 0);
    FT_ASSERT(dup2(pipe_fds[1], 2) >= 0);
    close(pipe_fds[1]);

    ft_perror("custom context");

    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(std::strstr(buffer, "custom context: ") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, expected_message) != ft_nullptr);
    FT_ASSERT_EQ(FT_EIO, ft_errno);

    FT_ASSERT(dup2(saved_stderr, 2) >= 0);
    close(saved_stderr);
    close(pipe_fds[0]);
    ft_errno = original_errno_value;
    return (1);
}

FT_TEST(test_ft_exit_message_without_errno, "ft_exit prints message without strerror when errno indicates success")
{
    std::string captured_output;
    int         child_status;
    int         original_errno_value;

    original_errno_value = ft_errno;
    FT_ASSERT(capture_ft_exit_output("shutting down", ER_SUCCESS, 23, captured_output, child_status));
    FT_ASSERT(WIFEXITED(child_status));
    FT_ASSERT_EQ(23, WEXITSTATUS(child_status));
    FT_ASSERT_EQ(std::string("shutting down\n"), captured_output);
    FT_ASSERT_EQ(original_errno_value, ft_errno);
    return (1);
}

FT_TEST(test_ft_exit_message_with_errno, "ft_exit appends strerror details when errno set")
{
    std::string captured_output;
    int         child_status;
    const char *expected_error_message;
    int         original_errno_value;

    original_errno_value = ft_errno;
    expected_error_message = ft_strerror(FT_EIO);
    FT_ASSERT(capture_ft_exit_output("fatal failure", FT_EIO, 90, captured_output, child_status));
    FT_ASSERT(WIFEXITED(child_status));
    FT_ASSERT_EQ(90, WEXITSTATUS(child_status));
    FT_ASSERT(!captured_output.empty());
    FT_ASSERT(captured_output.find("fatal failure: ") != std::string::npos);
    FT_ASSERT(captured_output.find(expected_error_message) != std::string::npos);
    FT_ASSERT_EQ('\n', captured_output.back());
    FT_ASSERT_EQ(original_errno_value, ft_errno);
    return (1);
}

FT_TEST(test_ft_exit_without_message_outputs_errno, "ft_exit prints strerror when no message provided")
{
    std::string captured_output;
    int         child_status;
    const char *expected_error_message;
    std::string expected_output;
    int         original_errno_value;

    original_errno_value = ft_errno;
    expected_error_message = ft_strerror(FT_EINVAL);
    expected_output = expected_error_message;
    expected_output.push_back('\n');
    FT_ASSERT(capture_ft_exit_output(ft_nullptr, FT_EINVAL, 7, captured_output, child_status));
    FT_ASSERT(WIFEXITED(child_status));
    FT_ASSERT_EQ(7, WEXITSTATUS(child_status));
    FT_ASSERT_EQ(expected_output, captured_output);
    FT_ASSERT_EQ(original_errno_value, ft_errno);
    return (1);
}
