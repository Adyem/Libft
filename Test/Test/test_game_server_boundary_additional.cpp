#include "../../Modules/Game/game_server.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

static void game_server_join_callback(int32_t client_id)
{
    (void)client_id;
    return ;
}

static void game_server_leave_callback(int32_t client_id)
{
    (void)client_id;
    return ;
}

FT_TEST(test_game_server_null_join_callback_is_accepted)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_join_callback(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_server_null_leave_callback_is_accepted)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_leave_callback(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_server_callbacks_can_be_replaced)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_join_callback(game_server_join_callback);
    value.set_leave_callback(game_server_leave_callback);
    value.set_join_callback(ft_nullptr);
    value.set_leave_callback(ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_server_run_once_before_start_is_safe)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.run_once();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, value.get_error());
    return (1);
}

FT_TEST(test_game_server_zero_port_can_bind_ephemeral_socket)
{
    game_server value;
    int32_t start_result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    start_result = value.start("127.0.0.1", 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, start_result);
    return (1);
}

FT_TEST(test_game_server_invalid_ip_is_reported)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, value.start("256.256.256.256", 1));
    return (1);
}

FT_TEST(test_game_server_thread_safety_cycle)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_server_repeated_run_once_is_safe)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.run_once();
    value.run_once();
    value.run_once();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, value.get_error());
    return (1);
}

FT_TEST(test_game_server_destroy_twice_is_safe)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_server_reinitialize_after_destroy)
{
    game_server value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}
