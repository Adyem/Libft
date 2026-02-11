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

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <windows.h>
#endif

FT_TEST(test_ft_strerror_sets_success_errno, "ft_strerror returns message and resets errno to success")
{
    const char *expected_message;
    const char *actual_message;
    int         previous_errno;

    expected_message = strerror(EINVAL);
    previous_errno = FT_ERR_MUTEX_ALREADY_LOCKED;
    ft_errno = previous_errno;
    actual_message = ft_strerror(FT_ERR_INVALID_ARGUMENT);
    FT_ASSERT(expected_message != NULL);
    FT_ASSERT(actual_message != NULL);
    FT_ASSERT_EQ(0, std::strcmp(expected_message, actual_message));
    FT_ASSERT_NE(previous_errno, ft_errno);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_errno_memcpy_tracks_error_state, "ft_memcpy updates ft_errno on failure and success")
{
    char        source_buffer[] = "data";
    char        destination_buffer[5];
    void       *copy_result;

    ft_errno = FT_ERR_SUCCESSS;
    copy_result = ft_memcpy(ft_nullptr, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    copy_result = ft_memcpy(destination_buffer, source_buffer, sizeof(source_buffer));
    FT_ASSERT(copy_result == destination_buffer);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(0, std::memcmp(destination_buffer, source_buffer, sizeof(source_buffer)));
    return (1);
}

FT_TEST(test_ft_errno_strlen_resets_after_failure, "ft_strlen resets ft_errno to success for valid input")
{
    const char *valid_string;
    int         string_length;

    valid_string = "example";
    ft_errno = FT_ERR_SUCCESSS;
    string_length = ft_strlen(ft_nullptr);
    FT_ASSERT_EQ(0, string_length);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    string_length = ft_strlen(valid_string);
    FT_ASSERT_EQ(7, string_length);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_errno_is_thread_local, "ft_errno maintains independent values per thread")
{
    std::atomic<int> thread_errno_value;
    std::thread      worker_thread;

    thread_errno_value.store(FT_ERR_SUCCESSS);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    worker_thread = std::thread(
        [&thread_errno_value]()
        {
            ft_errno = FT_ERR_SUCCESSS;
            ft_errno = FT_ERR_OUT_OF_RANGE;
            thread_errno_value.store(ft_errno);
            return ;
        });
    worker_thread.join();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, thread_errno_value.load());
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
    ft_errno = FT_ERR_INVALID_ARGUMENT;
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
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

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
    ft_errno = FT_ERR_IO;
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
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);

    FT_ASSERT(dup2(saved_stderr, 2) >= 0);
    close(saved_stderr);
    close(pipe_fds[0]);
    return (1);
}

FT_TEST(test_ft_map_system_error_normalizes_common_codes,
    "ft_map_system_error maps common operating system errors")
{
    int mapped_error;

    mapped_error = ft_map_system_error(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mapped_error);
#if defined(_WIN32) || defined(_WIN64)
    mapped_error = ft_map_system_error(WSAEWOULDBLOCK);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, mapped_error);
    mapped_error = ft_map_system_error(WSAECONNRESET);
    FT_ASSERT_EQ(FT_ERR_IO, mapped_error);
    mapped_error = ft_map_system_error(ERROR_FILE_NOT_FOUND);
    FT_ASSERT_EQ(FT_ERR_IO, mapped_error);
    mapped_error = ft_map_system_error(ERROR_INVALID_HANDLE);
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, mapped_error);
    mapped_error = ft_map_system_error(ERROR_ALREADY_EXISTS);
    FT_ASSERT_EQ(FT_ERR_ALREADY_EXISTS, mapped_error);
#else
    mapped_error = ft_map_system_error(EINTR);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, mapped_error);
    mapped_error = ft_map_system_error(EAGAIN);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, mapped_error);
    mapped_error = ft_map_system_error(ENOENT);
    FT_ASSERT_EQ(FT_ERR_IO, mapped_error);
    mapped_error = ft_map_system_error(EACCES);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
    mapped_error = ft_map_system_error(EINVAL);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
#if defined(EFAULT)
    mapped_error = ft_map_system_error(EFAULT);
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, mapped_error);
#endif
#if defined(E2BIG)
    mapped_error = ft_map_system_error(E2BIG);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, mapped_error);
#endif
    mapped_error = ft_map_system_error(EBADF);
    FT_ASSERT_EQ(FT_ERR_INVALID_HANDLE, mapped_error);
#if defined(ENODEV)
    mapped_error = ft_map_system_error(ENODEV);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, mapped_error);
#endif
#if defined(ENXIO)
    mapped_error = ft_map_system_error(ENXIO);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, mapped_error);
#endif
#if defined(ENAMETOOLONG)
    mapped_error = ft_map_system_error(ENAMETOOLONG);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, mapped_error);
#endif
#if defined(ENFILE)
    mapped_error = ft_map_system_error(ENFILE);
    FT_ASSERT_EQ(FT_ERR_FULL, mapped_error);
#endif
    mapped_error = ft_map_system_error(EMFILE);
    FT_ASSERT_EQ(FT_ERR_FULL, mapped_error);
    mapped_error = ft_map_system_error(ECONNREFUSED);
    FT_ASSERT_EQ(FT_ERR_SOCKET_CONNECT_FAILED, mapped_error);
#if defined(ENOTSUP)
    mapped_error = ft_map_system_error(ENOTSUP);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
#endif
#if defined(EBADMSG)
    mapped_error = ft_map_system_error(EBADMSG);
    FT_ASSERT_EQ(FT_ERR_IO, mapped_error);
#endif
    mapped_error = ft_map_system_error(ENOSPC);
    FT_ASSERT_EQ(FT_ERR_FULL, mapped_error);
#if defined(EOVERFLOW)
    mapped_error = ft_map_system_error(EOVERFLOW);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, mapped_error);
#endif
    mapped_error = ft_map_system_error(ERANGE);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, mapped_error);
#if defined(ECANCELED)
    mapped_error = ft_map_system_error(ECANCELED);
    FT_ASSERT_EQ(FT_ERR_TERMINATED, mapped_error);
#endif
#if defined(ENOSYS)
    mapped_error = ft_map_system_error(ENOSYS);
    FT_ASSERT_EQ(FT_ERR_UNSUPPORTED_TYPE, mapped_error);
#endif
#if defined(ENOTEMPTY)
    mapped_error = ft_map_system_error(ENOTEMPTY);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
#endif
#if defined(EROFS)
    mapped_error = ft_map_system_error(EROFS);
    FT_ASSERT_EQ(FT_ERR_INVALID_OPERATION, mapped_error);
#endif
#if defined(EOWNERDEAD)
    mapped_error = ft_map_system_error(EOWNERDEAD);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, mapped_error);
#endif
#endif
    mapped_error = ft_map_system_error(12345);
    FT_ASSERT_EQ(12345 + ERRNO_OFFSET, mapped_error);
    return (1);
}

FT_TEST(test_ft_set_errno_from_system_error_updates_global,
    "ft_set_errno_from_system_error stores normalized result in ft_errno")
{
    int mapped_error;

    ft_errno = FT_ERR_SUCCESSS;
#if defined(_WIN32) || defined(_WIN64)
    mapped_error = ft_set_errno_from_system_error(ERROR_DISK_FULL);
    FT_ASSERT_EQ(FT_ERR_FULL, mapped_error);
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
#else
    mapped_error = ft_set_errno_from_system_error(ENOSPC);
    FT_ASSERT_EQ(FT_ERR_FULL, mapped_error);
    FT_ASSERT_EQ(FT_ERR_FULL, ft_errno);
#endif
    mapped_error = ft_set_errno_from_system_error(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, mapped_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
