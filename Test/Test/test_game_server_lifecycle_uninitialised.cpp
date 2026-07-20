#include "../../Modules/Game/game_server.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_server_set_join_callback(game_server &value)
{
    value.set_join_callback(ft_nullptr);
    return ;
}

static void game_server_set_leave_callback(game_server &value)
{
    value.set_leave_callback(ft_nullptr);
    return ;
}

static void game_server_start(game_server &value)
{
    (void)value.start(ft_nullptr, 0U);
    return ;
}

static void game_server_run_once(game_server &value)
{
    value.run_once();
    return ;
}

static void game_server_get_error(game_server &value)
{
    (void)value.get_error();
    return ;
}

static void game_server_get_error_str(game_server &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_server_enable_thread_safety(game_server &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_server_disable_thread_safety(game_server &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_server_is_thread_safe(game_server &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_server_set_join_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_set_join_callback));
    return (1);
}

FT_TEST(test_game_server_set_leave_callback_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_set_leave_callback));
    return (1);
}

FT_TEST(test_game_server_start_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_server>(game_server_start));
    return (1);
}

FT_TEST(test_game_server_run_once_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_server>(game_server_run_once));
    return (1);
}

FT_TEST(test_game_server_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_server>(game_server_get_error));
    return (1);
}

FT_TEST(test_game_server_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_get_error_str));
    return (1);
}

FT_TEST(test_game_server_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_server_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_server_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_server>(
                        game_server_is_thread_safe));
    return (1);
}
