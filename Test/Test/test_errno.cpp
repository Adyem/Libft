#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <cerrno>
#include <cstring>
#include <thread>

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
