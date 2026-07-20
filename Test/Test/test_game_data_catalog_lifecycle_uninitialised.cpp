#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_data_catalog_get_item_definitions(game_data_catalog &value)
{
    (void)value.get_item_definitions();
    return ;
}

static void
game_data_catalog_get_item_definitions_const(game_data_catalog &value)
{
    const game_data_catalog &constant_value = value;

    (void)constant_value.get_item_definitions();
    return ;
}

static void game_data_catalog_get_recipes(game_data_catalog &value)
{
    (void)value.get_recipes();
    return ;
}

static void game_data_catalog_get_recipes_const(game_data_catalog &value)
{
    const game_data_catalog &constant_value = value;

    (void)constant_value.get_recipes();
    return ;
}

static void game_data_catalog_get_loadouts(game_data_catalog &value)
{
    (void)value.get_loadouts();
    return ;
}

static void game_data_catalog_get_loadouts_const(game_data_catalog &value)
{
    const game_data_catalog &constant_value = value;

    (void)constant_value.get_loadouts();
    return ;
}

static void game_data_catalog_register_item_definition(game_data_catalog &value)
{
    game_item_definition definition;

    (void)value.register_item_definition(definition);
    return ;
}

static void game_data_catalog_register_recipe(game_data_catalog &value)
{
    game_recipe_blueprint recipe;

    (void)value.register_recipe(recipe);
    return ;
}

static void game_data_catalog_register_loadout(game_data_catalog &value)
{
    game_loadout_blueprint loadout;

    (void)value.register_loadout(loadout);
    return ;
}

static void game_data_catalog_fetch_item_definition(game_data_catalog &value)
{
    game_item_definition definition;

    (void)value.fetch_item_definition(1, definition);
    return ;
}

static void game_data_catalog_fetch_recipe(game_data_catalog &value)
{
    game_recipe_blueprint recipe;

    (void)value.fetch_recipe(1, recipe);
    return ;
}

static void game_data_catalog_fetch_loadout(game_data_catalog &value)
{
    game_loadout_blueprint loadout;

    (void)value.fetch_loadout(1, loadout);
    return ;
}

static void game_data_catalog_enable_thread_safety(game_data_catalog &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_data_catalog_disable_thread_safety(game_data_catalog &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_data_catalog_is_thread_safe(game_data_catalog &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_data_catalog_get_error(game_data_catalog &value)
{
    (void)value.get_error();
    return ;
}

static void game_data_catalog_get_error_str(game_data_catalog &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_data_catalog_get_item_definitions_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_item_definitions));
    return (1);
}

FT_TEST(test_game_data_catalog_get_item_definitions_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_item_definitions_const));
    return (1);
}

FT_TEST(test_game_data_catalog_get_recipes_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_recipes));
    return (1);
}

FT_TEST(test_game_data_catalog_get_recipes_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_recipes_const));
    return (1);
}

FT_TEST(test_game_data_catalog_get_loadouts_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_loadouts));
    return (1);
}

FT_TEST(test_game_data_catalog_get_loadouts_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_loadouts_const));
    return (1);
}

FT_TEST(test_game_data_catalog_register_item_definition_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_register_item_definition));
    return (1);
}

FT_TEST(test_game_data_catalog_register_recipe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_register_recipe));
    return (1);
}

FT_TEST(test_game_data_catalog_register_loadout_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_register_loadout));
    return (1);
}

FT_TEST(test_game_data_catalog_fetch_item_definition_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_fetch_item_definition));
    return (1);
}

FT_TEST(test_game_data_catalog_fetch_recipe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_fetch_recipe));
    return (1);
}

FT_TEST(test_game_data_catalog_fetch_loadout_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_fetch_loadout));
    return (1);
}

FT_TEST(test_game_data_catalog_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_data_catalog_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_data_catalog_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_is_thread_safe));
    return (1);
}

FT_TEST(test_game_data_catalog_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_error));
    return (1);
}

FT_TEST(test_game_data_catalog_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        game_data_catalog_get_error_str));
    return (1);
}
