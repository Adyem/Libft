#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_loadout_entry_copy_initialize_uninitialised(game_loadout_entry &value)
{
    game_loadout_entry source;

    (void)value.initialize(source);
    return ;
}

static void
game_loadout_entry_move_initialize_uninitialised(game_loadout_entry &value)
{
    game_loadout_entry source;

    (void)value.initialize(static_cast<game_loadout_entry &&>(source));
    return ;
}

static void game_loadout_entry_move_uninitialised(game_loadout_entry &value)
{
    game_loadout_entry source;

    (void)value.move(source);
    return ;
}

static void game_loadout_entry_destroy_uninitialised(game_loadout_entry &value)
{
    (void)value.destroy();
    return ;
}

static void game_recipe_blueprint_copy_initialize_uninitialised(
    game_recipe_blueprint &value)
{
    game_recipe_blueprint source;

    (void)value.initialize(source);
    return ;
}

static void game_recipe_blueprint_move_initialize_uninitialised(
    game_recipe_blueprint &value)
{
    game_recipe_blueprint source;

    (void)value.initialize(static_cast<game_recipe_blueprint &&>(source));
    return ;
}

static void
game_recipe_blueprint_move_uninitialised(game_recipe_blueprint &value)
{
    game_recipe_blueprint source;

    (void)value.move(source);
    return ;
}

static void
game_recipe_blueprint_destroy_uninitialised(game_recipe_blueprint &value)
{
    (void)value.destroy();
    return ;
}

static void game_loadout_blueprint_copy_initialize_uninitialised(
    game_loadout_blueprint &value)
{
    game_loadout_blueprint source;

    (void)value.initialize(source);
    return ;
}

static void game_loadout_blueprint_move_initialize_uninitialised(
    game_loadout_blueprint &value)
{
    game_loadout_blueprint source;

    (void)value.initialize(static_cast<game_loadout_blueprint &&>(source));
    return ;
}

static void
game_loadout_blueprint_move_uninitialised(game_loadout_blueprint &value)
{
    game_loadout_blueprint source;

    (void)value.move(source);
    return ;
}

static void
game_loadout_blueprint_destroy_uninitialised(game_loadout_blueprint &value)
{
    (void)value.destroy();
    return ;
}

FT_TEST(test_game_loadout_entry_copy_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_entry_move_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_entry_move_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_move_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_entry_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_destroy_uninitialised));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_copy_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_move_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_move_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_move_uninitialised));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_destroy_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_copy_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_move_initialize_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_move_from_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_move_uninitialised));
    return (1);
}

FT_TEST(test_game_loadout_blueprint_destroy_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_blueprint>(
                        game_loadout_blueprint_destroy_uninitialised));
    return (1);
}
