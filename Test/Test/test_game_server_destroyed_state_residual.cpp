#include "../../Modules/Game/game_server.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void server_destroyed_disable_thread_safety(game_server &value)
{
    (void)value.disable_thread_safety();
    return ;
}

FT_TEST(test_game_server_destroyed_disable_thread_safety_aborts_again)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_server_destroyed_is_thread_safe_is_false_again)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_server_destroyed_get_error_str_is_valid_again)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_server_destroyed_state_can_reinitialize_again)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_server_destroyed_cleanup_is_idempotent_again)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_server_destroyed_destructor_after_cleanup_is_safe_again)
{
    game_server *value;

    value = new game_server();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}

FT_TEST(test_game_server_destroyed_cleanup_preserves_error_success)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_server_destroyed_thread_safety_restarts)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_server_destroyed_run_once_after_reinitialize_is_safe)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.run_once();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, value.get_error());
    return (1);
}

static void server_destroyed_disable_after_repeated_cleanup(game_server &value)
{
    (void)value.disable_thread_safety();
    return ;
}

FT_TEST(test_game_server_destroyed_disable_after_repeated_cleanup_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_server>(
                        server_destroyed_disable_after_repeated_cleanup));
    return (1);
}

FT_TEST(test_game_server_destroyed_error_string_remains_available)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}
