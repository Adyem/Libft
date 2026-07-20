#include "../../Modules/Game/game_server.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void server_destroyed_set_join_callback(game_server &value)
{
    value.set_join_callback(ft_nullptr);
    return ;
}

static void server_destroyed_disable_thread_safety(game_server &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void server_destroyed_set_leave_callback(game_server &value)
{
    value.set_leave_callback(ft_nullptr);
    return ;
}

static void server_destroyed_enable_thread_safety(game_server &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void server_destroyed_start(game_server &value)
{
    (void)value.start("127.0.0.1", 0);
    return ;
}

static void server_destroyed_run_once(game_server &value)
{
    value.run_once();
    return ;
}

FT_TEST(test_game_server_destroyed_set_join_callback_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_set_join_callback));
    return (1);
}

FT_TEST(test_game_server_destroyed_set_leave_callback_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_set_leave_callback));
    return (1);
}

FT_TEST(test_game_server_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_server_destroyed_start_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_destroyed_sigabrt<game_server>(server_destroyed_start));
    return (1);
}

FT_TEST(test_game_server_destroyed_run_once_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_run_once));
    return (1);
}

FT_TEST(test_game_server_destroyed_get_error_is_valid)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_server_destroyed_get_error_str_is_valid)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_server_destroyed_disable_thread_safety_is_safe)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_server_destroyed_is_thread_safe_is_safe)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_server_destroyed_destructor_is_non_aborting)
{
    game_server *value;

    value = new game_server();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
