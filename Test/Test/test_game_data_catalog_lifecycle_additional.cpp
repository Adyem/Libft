#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void catalog_initialize_twice(game_data_catalog &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void catalog_copy_initialize_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.initialize(source);
    return ;
}

static void catalog_move_initialize_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.initialize(static_cast<game_data_catalog &&>(source));
    return ;
}

static void catalog_move_uninitialised(game_data_catalog &value)
{
    game_data_catalog source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_data_catalog_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        catalog_initialize_twice));
    return (1);
}

FT_TEST(test_game_data_catalog_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        catalog_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        catalog_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_data_catalog>(
                        catalog_move_uninitialised));
    return (1);
}

FT_TEST(test_game_data_catalog_default_maps_are_empty)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_item_definitions().size());
    FT_ASSERT_EQ(0, value.get_recipes().size());
    FT_ASSERT_EQ(0, value.get_loadouts().size());
    return (1);
}

FT_TEST(test_game_data_catalog_const_item_definitions_are_empty)
{
    game_data_catalog value;
    const game_data_catalog &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_item_definitions().size());
    return (1);
}

FT_TEST(test_game_data_catalog_const_recipes_are_empty)
{
    game_data_catalog value;
    const game_data_catalog &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_recipes().size());
    return (1);
}

FT_TEST(test_game_data_catalog_const_loadouts_are_empty)
{
    game_data_catalog value;
    const game_data_catalog &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_loadouts().size());
    return (1);
}

FT_TEST(test_game_data_catalog_thread_safety_enable_disable_cycle)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_data_catalog_error_after_success)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_data_catalog_destroy_twice_is_safe)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_data_catalog_reinitialize_after_destroy)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_recipes().size());
    return (1);
}

FT_TEST(test_game_data_catalog_self_move_is_safe)
{
    game_data_catalog value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(0, value.get_loadouts().size());
    return (1);
}

FT_TEST(test_game_data_catalog_empty_fetch_returns_not_found)
{
    game_data_catalog value;
    game_item_definition definition;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, definition.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.fetch_item_definition(1, definition));
    return (1);
}

FT_TEST(test_game_data_catalog_empty_recipe_fetch_returns_not_found)
{
    game_data_catalog value;
    game_recipe_blueprint recipe;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, recipe.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.fetch_recipe(1, recipe));
    return (1);
}

FT_TEST(test_game_data_catalog_empty_loadout_fetch_returns_not_found)
{
    game_data_catalog value;
    game_loadout_blueprint loadout;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, loadout.initialize());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, value.fetch_loadout(1, loadout));
    return (1);
}
