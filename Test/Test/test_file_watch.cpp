#include "../test_internal.hpp"
#include "../../File/file_watch.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void file_watch_noop_callback(const char *path, int event_type, void *user_data)
{
    (void)path;
    (void)event_type;
    (void)user_data;
    return ;
}

FT_TEST(test_file_watch_error_resets_after_success,
        "ft_file_watch validates input and starts successfully")
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(-1, file_watch.watch_directory(ft_nullptr, ft_nullptr,
                ft_nullptr));
    FT_ASSERT_EQ(0, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_stop_resets_errno_when_inactive,
    "ft_file_watch stop is safe when watcher is already stopped")
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_get_error_resets_errno,
    "ft_file_watch requires initialize before watch_directory")
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(-1, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(0, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_get_error_str_resets_errno,
    "ft_file_watch destroy rejects repeated destroy")
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.destroy());
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, file_watch.destroy());
    return (1);
}
