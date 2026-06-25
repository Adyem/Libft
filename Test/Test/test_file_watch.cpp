#include "../test_internal.hpp"
#include "../../Modules/File/file_watch.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/File/file_watch_event.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static void file_watch_noop_callback(const char *path, int event_type, void *user_data)
{
    (void)path;
    (void)event_type;
    (void)user_data;
    return ;
}

FT_TEST(test_file_watch_error_resets_after_success)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, file_watch.watch_directory(ft_nullptr, static_cast<file_watch_legacy_callback>(ft_nullptr),
                ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_stop_resets_errno_when_inactive)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_get_error_resets_errno)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.watch_directory(".",
                &file_watch_noop_callback, ft_nullptr));
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_get_error_str_resets_errno)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.destroy());
    return (1);
}

static void file_watch_typed_noop_callback(const char *path,
    file_watch_event_type event_type, void *user_data)
{
    (void)path;
    (void)event_type;
    (void)user_data;
    return ;
}

FT_TEST(test_file_watch_typed_callback_overload_starts)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.watch_directory(".",
            &file_watch_typed_noop_callback, ft_nullptr));
    file_watch.stop();
    return (1);
}

FT_TEST(test_file_watch_debounce_configuration_round_trips)
{
    ft_file_watch file_watch;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.initialize());
    FT_ASSERT_EQ(static_cast<uint32_t>(0), file_watch.get_debounce_milliseconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.set_debounce_milliseconds(25));
    FT_ASSERT_EQ(static_cast<uint32_t>(25), file_watch.get_debounce_milliseconds());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, file_watch.destroy());
    return (1);
}
