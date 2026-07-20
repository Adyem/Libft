#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_loadout_blueprint_get_loadout_id(game_loadout_blueprint &value)
{
    (void)value.get_loadout_id();
    return ;
}

static void game_loadout_blueprint_set_loadout_id(game_loadout_blueprint &value)
{
    value.set_loadout_id(1);
    return ;
}

static void game_loadout_blueprint_get_entries(game_loadout_blueprint &value)
{
    (void)value.get_entries();
    return ;
}

static void
game_loadout_blueprint_get_entries_const(game_loadout_blueprint &value)
{
    const game_loadout_blueprint &constant_value = value;

    (void)constant_value.get_entries();
    return ;
}

static void game_loadout_blueprint_set_entries(game_loadout_blueprint &value)
{
    ft_vector<game_loadout_entry> entries;

    value.set_entries(entries);
    return ;
}

static void
game_loadout_blueprint_enable_thread_safety(game_loadout_blueprint &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void
game_loadout_blueprint_disable_thread_safety(game_loadout_blueprint &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_loadout_blueprint_is_thread_safe(game_loadout_blueprint &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_loadout_blueprint_get_error(game_loadout_blueprint &value)
{
    (void)value.get_error();
    return ;
}

static void game_loadout_blueprint_get_error_str(game_loadout_blueprint &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_loadout_blueprint_get_loadout_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_get_loadout_id));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_set_loadout_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_set_loadout_id));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_get_entries_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_get_entries));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_get_entries_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_get_entries_const));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_set_entries_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_set_entries));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_is_thread_safe));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_get_error));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_get_error_str));
    return (1);
}
