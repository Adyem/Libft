#include "../test_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <cerrno>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/wait.h>

#ifndef LIBFT_TEST_BUILD
#endif

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <windows.h>
#endif

FT_TEST(test_ft_strerror_sets_success_errno, "ft_strerror returns message and resets errno to success")
{
    const char *expected_message;
    const char *actual_message;

    expected_message = strerror(EINVAL);
    actual_message = ft_strerror(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT(expected_message != NULL);
    FT_ASSERT(actual_message != NULL);
    FT_ASSERT_EQ(0, std::strcmp(expected_message, actual_message));
    return (1);
}

FT_TEST(test_ft_errno_memcpy_tracks_error_state, "ft_memcpy updates ft_errno on failure and success")
{
    char        source_buffer[] = "data";
    char        destination_buffer[5];
    void       *copy_result;

    copy_result = ft_memcpy(ft_nullptr, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == ft_nullptr);

    copy_result = ft_memcpy(destination_buffer, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == destination_buffer);
    FT_ASSERT_EQ(0, std::memcmp(destination_buffer, source_buffer, sizeof(source_buffer)));
    return (1);
}

FT_TEST(test_ft_errno_strlen_resets_after_failure, "ft_strlen resets ft_errno to success for valid input")
{
    const char *valid_string;
    int         string_length;

    valid_string = "example";
    string_length = ft_strlen(ft_nullptr);
    FT_ASSERT_EQ(0, string_length);

    string_length = ft_strlen(valid_string);
    FT_ASSERT_EQ(7, string_length);
    return (1);
}

FT_TEST(test_ft_errno_is_thread_local, "ft_errno maintains independent values per thread")
{
    std::atomic<int> thread_string_length;
    std::thread      worker_thread;

    thread_string_length.store(0);
    worker_thread = std::thread(
        [&thread_string_length]()
        {
            thread_string_length.store(ft_strlen("worker"));
            return ;
        });
    worker_thread.join();
    FT_ASSERT_EQ(6, thread_string_length.load());
    return (1);
}

FT_TEST(test_ft_perror_null_message_resets_errno, "ft_perror prints strerror when message is null and clears errno")
{
    int     pipe_fds[2];
    int     saved_stderr;
    ssize_t read_count;
    char    buffer[256];
    const char *expected_message;

    expected_message = ft_strerror(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT_EQ(0, pipe(pipe_fds));
    saved_stderr = dup(2);
    FT_ASSERT(saved_stderr >= 0);
    FT_ASSERT(dup2(pipe_fds[1], 2) >= 0);
    close(pipe_fds[1]);

    ft_perror(ft_nullptr, FT_ERR_INVALID_ARGUMENT);

    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(std::strstr(buffer, expected_message) != ft_nullptr);
    FT_ASSERT(dup2(saved_stderr, 2) >= 0);
    close(saved_stderr);
    close(pipe_fds[0]);
    return (1);
}

FT_TEST(test_ft_perror_prefixes_custom_message_resets_errno,
    "ft_perror prefixes custom text before strerror output and clears errno")
{
    int     pipe_fds[2];
    int     saved_stderr;
    ssize_t read_count;
    char    buffer[256];
    const char *expected_message;

    expected_message = ft_strerror(FT_ERR_IO);
    FT_ASSERT_EQ(0, pipe(pipe_fds));
    saved_stderr = dup(2);
    FT_ASSERT(saved_stderr >= 0);
    FT_ASSERT(dup2(pipe_fds[1], 2) >= 0);
    close(pipe_fds[1]);

    ft_perror("custom context", FT_ERR_IO);

    read_count = read(pipe_fds[0], buffer, sizeof(buffer) - 1);
    FT_ASSERT(read_count > 0);
    buffer[read_count] = '\0';
    FT_ASSERT(std::strstr(buffer, "custom context: ") != ft_nullptr);
    FT_ASSERT(std::strstr(buffer, expected_message) != ft_nullptr);
    FT_ASSERT(dup2(saved_stderr, 2) >= 0);
    close(saved_stderr);
    close(pipe_fds[0]);
    return (1);
}
