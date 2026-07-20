#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void buff_initialize_twice(game_buff &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

FT_TEST(test_game_buff_initialize_twice_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(buff_initialize_twice));
    return (1);
}

FT_TEST(test_game_buff_initialize_succeeds_with_zero_values)
{
    game_buff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    FT_ASSERT_EQ(0, value.get_duration());
    return (1);
}

FT_TEST(test_game_buff_destroy_twice_is_safe)
{
    game_buff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_buff_reinitialize_after_destroy)
{
    game_buff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    return (1);
}

FT_TEST(test_game_buff_self_move_is_safe)
{
    game_buff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier4(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(9, value.get_modifier4());
    return (1);
}

FT_TEST(test_game_buff_thread_safety_cycle)
{
    game_buff value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}
