#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void buff_destroyed_get_id(game_buff &value)
{
    (void)value.get_id();
    return ;
}

static void buff_destroyed_set_id(game_buff &value)
{
    value.set_id(3);
    return ;
}

static void buff_destroyed_get_duration(game_buff &value)
{
    (void)value.get_duration();
    return ;
}

static void buff_destroyed_set_duration(game_buff &value)
{
    value.set_duration(4);
    return ;
}

static void buff_destroyed_add_duration(game_buff &value)
{
    value.add_duration(1);
    return ;
}

static void buff_destroyed_sub_duration(game_buff &value)
{
    value.sub_duration(1);
    return ;
}

static void buff_destroyed_get_modifier1(game_buff &value)
{
    (void)value.get_modifier1();
    return ;
}

static void buff_destroyed_set_modifier1(game_buff &value)
{
    value.set_modifier1(2);
    return ;
}

static void buff_destroyed_enable_thread_safety(game_buff &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void buff_destroyed_lock(game_buff &value)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    (void)value.lock(&lock_acquired);
    return ;
}

FT_TEST(test_game_buff_destroyed_get_id_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_destroyed_sigabrt<game_buff>(buff_destroyed_get_id));
    return (1);
}

FT_TEST(test_game_buff_destroyed_set_id_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_destroyed_sigabrt<game_buff>(buff_destroyed_set_id));
    return (1);
}

FT_TEST(test_game_buff_destroyed_get_duration_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_get_duration));
    return (1);
}

FT_TEST(test_game_buff_destroyed_set_duration_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_set_duration));
    return (1);
}

FT_TEST(test_game_buff_destroyed_add_duration_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_add_duration));
    return (1);
}

FT_TEST(test_game_buff_destroyed_sub_duration_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_sub_duration));
    return (1);
}

FT_TEST(test_game_buff_destroyed_get_modifier1_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_get_modifier1));
    return (1);
}

FT_TEST(test_game_buff_destroyed_set_modifier1_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_set_modifier1));
    return (1);
}

FT_TEST(test_game_buff_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_buff>(
                        buff_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_buff_destroyed_lock_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_destroyed_sigabrt<game_buff>(buff_destroyed_lock));
    return (1);
}
