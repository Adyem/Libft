#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void upgrade_initialize_twice(game_upgrade &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void upgrade_copy_initialize_uninitialised(game_upgrade &value)
{
    game_upgrade source;

    (void)value.initialize(source);
    return ;
}

static void upgrade_move_initialize_uninitialised(game_upgrade &value)
{
    game_upgrade source;

    (void)value.initialize(static_cast<game_upgrade &&>(source));
    return ;
}

static void upgrade_move_uninitialised(game_upgrade &value)
{
    game_upgrade source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_upgrade_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        upgrade_initialize_twice));
    return (1);
}

FT_TEST(test_game_upgrade_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        upgrade_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_upgrade_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        upgrade_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_upgrade_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        upgrade_move_uninitialised));
    return (1);
}

FT_TEST(test_game_upgrade_initialize_default_values)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    FT_ASSERT_EQ(0, value.get_current_level());
    FT_ASSERT_EQ(0, value.get_max_level());
    FT_ASSERT_EQ(0, value.get_modifier1());
    FT_ASSERT_EQ(0, value.get_modifier4());
    return (1);
}

FT_TEST(test_game_upgrade_set_id_and_levels)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(4);
    value.set_current_level(2);
    value.set_max_level(5);
    FT_ASSERT_EQ(4, value.get_id());
    FT_ASSERT_EQ(2, value.get_current_level());
    FT_ASSERT_EQ(5, value.get_max_level());
    return (1);
}

FT_TEST(test_game_upgrade_add_and_sub_levels)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_current_level(3);
    value.add_level(4);
    value.sub_level(2);
    FT_ASSERT_EQ(5, value.get_current_level());
    return (1);
}

FT_TEST(test_game_upgrade_modifier1_values)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier1(1);
    value.add_modifier1(2);
    value.sub_modifier1(1);
    FT_ASSERT_EQ(2, value.get_modifier1());
    return (1);
}

FT_TEST(test_game_upgrade_modifier2_values)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier2(2);
    value.add_modifier2(3);
    value.sub_modifier2(1);
    FT_ASSERT_EQ(4, value.get_modifier2());
    return (1);
}

FT_TEST(test_game_upgrade_modifier3_values)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier3(3);
    value.add_modifier3(4);
    value.sub_modifier3(1);
    FT_ASSERT_EQ(6, value.get_modifier3());
    return (1);
}

FT_TEST(test_game_upgrade_modifier4_values)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier4(4);
    value.add_modifier4(5);
    value.sub_modifier4(1);
    FT_ASSERT_EQ(8, value.get_modifier4());
    return (1);
}

FT_TEST(test_game_upgrade_thread_safety_enable_disable_cycle)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_upgrade_lock_unlock_initialised)
{
    game_upgrade value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_upgrade_error_after_success)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_current_level(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_upgrade_destroy_twice_is_safe)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_upgrade_reinitialize_after_destroy)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    return (1);
}

FT_TEST(test_game_upgrade_self_move_is_safe)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(10, value.get_id());
    return (1);
}

FT_TEST(test_game_upgrade_all_modifiers_can_be_updated)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier1(1);
    value.set_modifier2(2);
    value.set_modifier3(3);
    value.set_modifier4(4);
    FT_ASSERT_EQ(1, value.get_modifier1());
    FT_ASSERT_EQ(2, value.get_modifier2());
    FT_ASSERT_EQ(3, value.get_modifier3());
    FT_ASSERT_EQ(4, value.get_modifier4());
    return (1);
}

FT_TEST(test_game_upgrade_zero_level_subtraction_is_preserved)
{
    game_upgrade value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_current_level(0);
    value.sub_level(0);
    FT_ASSERT_EQ(0, value.get_current_level());
    return (1);
}

FT_TEST(test_game_upgrade_copy_keeps_values)
{
    game_upgrade source;
    game_upgrade destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_id(4);
    source.set_current_level(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(4, destination.get_id());
    FT_ASSERT_EQ(6, destination.get_current_level());
    return (1);
}
