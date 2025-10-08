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
