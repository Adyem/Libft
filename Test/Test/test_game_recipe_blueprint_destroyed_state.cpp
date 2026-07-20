#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void recipe_destroyed_get_recipe_id(game_recipe_blueprint &value)
{
    (void)value.get_recipe_id();
    return ;
}

static void recipe_destroyed_set_recipe_id(game_recipe_blueprint &value)
{
    value.set_recipe_id(3);
    return ;
}

static void recipe_destroyed_get_result_item_id(game_recipe_blueprint &value)
{
    (void)value.get_result_item_id();
    return ;
}

static void recipe_destroyed_set_result_item_id(game_recipe_blueprint &value)
{
    value.set_result_item_id(4);
    return ;
}

static void recipe_destroyed_get_ingredients(game_recipe_blueprint &value)
{
    (void)value.get_ingredients();
    return ;
}

static void recipe_destroyed_set_ingredients(game_recipe_blueprint &value)
{
    ft_vector<game_crafting_ingredient> ingredients;

    (void)ingredients.initialize();
    value.set_ingredients(ingredients);
    return ;
}

static void recipe_destroyed_enable_thread_safety(game_recipe_blueprint &value)
{
    (void)value.enable_thread_safety();
    return ;
}

FT_TEST(test_game_recipe_destroyed_get_recipe_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_get_recipe_id));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_set_recipe_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_set_recipe_id));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_get_result_item_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_get_result_item_id));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_set_result_item_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_set_result_item_id));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_get_ingredients_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_get_ingredients));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_set_ingredients_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_set_ingredients));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_recipe_blueprint>(
                        recipe_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_recipe_destroyed_get_error_is_valid)
{
    game_recipe_blueprint value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_recipe_destroyed_get_error_str_is_valid)
{
    game_recipe_blueprint value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}
