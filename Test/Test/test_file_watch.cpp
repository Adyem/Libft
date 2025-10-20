#include "../../File/file_watch.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static void file_watch_noop_callback(const char *path, int event_type, void *user_data)
{
    (void)path;
    (void)event_type;
    (void)user_data;
    return ;
}

FT_TEST(test_file_watch_error_resets_after_success, "ft_file_watch resets error state after successful watch")
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(-1, file_watch.watch_directory(ft_nullptr, ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_watch.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(0, file_watch.watch_directory(".", &file_watch_noop_callback, ft_nullptr));
    FT_ASSERT_EQ(ER_SUCCESS, file_watch.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_stop_preserves_errno_when_inactive,
    "ft_file_watch stop preserves errno when watcher already stopped")
{
    ft_file_watch file_watch;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    file_watch.stop();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_file_watch_get_error_preserves_entry_errno,
    "ft_file_watch get_error keeps incoming errno intact")
{
    ft_file_watch file_watch;
    int error_value;

    ft_errno = FT_ERR_SOCKET_ACCEPT_FAILED;
    error_value = file_watch.get_error();
    FT_ASSERT_EQ(ER_SUCCESS, error_value);
    FT_ASSERT_EQ(FT_ERR_SOCKET_ACCEPT_FAILED, ft_errno);
    return (1);
}

FT_TEST(test_file_watch_get_error_str_preserves_entry_errno,
    "ft_file_watch get_error_str keeps incoming errno intact")
{
    ft_file_watch file_watch;
    const char *error_string;

    ft_errno = FT_ERR_SOCKET_CONNECT_FAILED;
    error_string = file_watch.get_error_str();
    FT_ASSERT(error_string != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SOCKET_CONNECT_FAILED, ft_errno);
    return (1);
}
