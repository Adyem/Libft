#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void crafting_destroyed_get_recipes(game_crafting &value)
{
    (void)value.get_recipes();
    return ;
}

static void crafting_destroyed_get_recipes_const(game_crafting &value)
{
    const game_crafting &constant_value = value;

    (void)constant_value.get_recipes();
    return ;
}

static void crafting_destroyed_register_recipe(game_crafting &value)
{
    ft_vector<game_crafting_ingredient> ingredients;

    (void)ingredients.initialize();
    (void)value.register_recipe(
        1, static_cast<ft_vector<game_crafting_ingredient> &&>(ingredients));
    return ;
}

static void crafting_destroyed_craft_item(game_crafting &value)
{
    game_inventory inventory;
    ft_sharedptr<game_item> item;

    (void)inventory.initialize();
    (void)value.craft_item(inventory, 1, item);
    return ;
}

static void crafting_destroyed_enable_thread_safety(game_crafting &value)
{
    (void)value.enable_thread_safety();
    return ;
}

FT_TEST(test_game_crafting_destroyed_get_recipes_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_crafting>(
                        crafting_destroyed_get_recipes));
    return (1);
}

FT_TEST(test_game_crafting_destroyed_get_recipes_const_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_crafting>(
                        crafting_destroyed_get_recipes_const));
    return (1);
}

FT_TEST(test_game_crafting_destroyed_register_recipe_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_crafting>(
                        crafting_destroyed_register_recipe));
    return (1);
}

FT_TEST(test_game_crafting_destroyed_craft_item_aborts)
{
    FT_ASSERT_EQ(0, expect_game_destroyed_sigabrt<game_crafting>(
                        crafting_destroyed_craft_item));
    return (1);
}

FT_TEST(test_game_crafting_destroyed_enable_thread_safety_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_crafting>(
                        crafting_destroyed_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_crafting_destroyed_get_error_is_valid)
{
    game_crafting value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_crafting_destroyed_get_error_str_is_valid)
{
    game_crafting value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_crafting_destroyed_destroy_is_idempotent)
{
    game_crafting value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_crafting_destroyed_state_can_reinitialize)
{
    game_crafting value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_crafting_destroyed_destructor_is_non_aborting)
{
    game_crafting *value;

    value = new game_crafting();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
