#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_loadout_entry_get_slot(game_loadout_entry &value)
{
    (void)value.get_slot();
    return ;
}

static void game_loadout_entry_set_slot(game_loadout_entry &value)
{
    value.set_slot(1);
    return ;
}

static void game_loadout_entry_get_item_id(game_loadout_entry &value)
{
    (void)value.get_item_id();
    return ;
}

static void game_loadout_entry_set_item_id(game_loadout_entry &value)
{
    value.set_item_id(1);
    return ;
}

static void game_loadout_entry_get_quantity(game_loadout_entry &value)
{
    (void)value.get_quantity();
    return ;
}

static void game_loadout_entry_set_quantity(game_loadout_entry &value)
{
    value.set_quantity(1);
    return ;
}

static void game_loadout_entry_enable_thread_safety(game_loadout_entry &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_loadout_entry_disable_thread_safety(game_loadout_entry &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_loadout_entry_is_thread_safe(game_loadout_entry &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_loadout_entry_get_error(game_loadout_entry &value)
{
    (void)value.get_error();
    return ;
}

static void game_loadout_entry_get_error_str(game_loadout_entry &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_recipe_blueprint_get_recipe_id(game_recipe_blueprint &value)
{
    (void)value.get_recipe_id();
    return ;
}

static void game_recipe_blueprint_set_recipe_id(game_recipe_blueprint &value)
{
    value.set_recipe_id(1);
    return ;
}

static void
game_recipe_blueprint_get_result_item_id(game_recipe_blueprint &value)
{
    (void)value.get_result_item_id();
    return ;
}

static void
game_recipe_blueprint_set_result_item_id(game_recipe_blueprint &value)
{
    value.set_result_item_id(1);
    return ;
}

static void game_recipe_blueprint_get_ingredients(game_recipe_blueprint &value)
{
    (void)value.get_ingredients();
    return ;
}

static void
game_recipe_blueprint_get_ingredients_const(game_recipe_blueprint &value)
{
    const game_recipe_blueprint &constant_value = value;

    (void)constant_value.get_ingredients();
    return ;
}

static void game_recipe_blueprint_set_ingredients(game_recipe_blueprint &value)
{
    ft_vector<game_crafting_ingredient> ingredients;

    value.set_ingredients(ingredients);
    return ;
}

static void game_recipe_blueprint_get_error(game_recipe_blueprint &value)
{
    (void)value.get_error();
    return ;
}

FT_TEST(test_game_loadout_entry_get_slot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_get_slot));
    return (1);
}

FT_TEST(test_game_loadout_entry_set_slot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_set_slot));
    return (1);
}

FT_TEST(test_game_loadout_entry_get_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_get_item_id));
    return (1);
}

FT_TEST(test_game_loadout_entry_set_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_set_item_id));
    return (1);
}

FT_TEST(test_game_loadout_entry_get_quantity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_get_quantity));
    return (1);
}

FT_TEST(test_game_loadout_entry_set_quantity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_set_quantity));
    return (1);
}

FT_TEST(test_game_loadout_entry_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_loadout_entry_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_loadout_entry_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_is_thread_safe));
    return (1);
}

FT_TEST(test_game_loadout_entry_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_get_error));
    return (1);
}

FT_TEST(test_game_loadout_entry_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_loadout_entry>(
                        game_loadout_entry_get_error_str));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_recipe_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_get_recipe_id));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_set_recipe_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_set_recipe_id));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_result_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_get_result_item_id));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_set_result_item_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_set_result_item_id));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_ingredients_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_get_ingredients));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_ingredients_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_get_ingredients_const));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_set_ingredients_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_set_ingredients));
    return (1);
}

FT_TEST(test_game_recipe_blueprint_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_recipe_blueprint>(
                        game_recipe_blueprint_get_error));
    return (1);
}
