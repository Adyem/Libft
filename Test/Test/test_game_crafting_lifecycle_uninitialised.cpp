#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_crafting_get_recipes(game_crafting &value)
{
    (void)value.get_recipes();
    return ;
}

static void game_crafting_get_recipes_const(game_crafting &value)
{
    const game_crafting &constant_value = value;

    (void)constant_value.get_recipes();
    return ;
}

static void game_crafting_register_recipe(game_crafting &value)
{
    ft_vector<game_crafting_ingredient> ingredients;

    (void)value.register_recipe(
        1, static_cast<ft_vector<game_crafting_ingredient> &&>(ingredients));
    return ;
}

static void game_crafting_craft_item(game_crafting &value)
{
    game_inventory inventory;
    ft_sharedptr<game_item> result;

    (void)value.craft_item(inventory, 1, result);
    return ;
}

static void game_crafting_get_error(game_crafting &value)
{
    (void)value.get_error();
    return ;
}

static void game_crafting_get_error_str(game_crafting &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_crafting_enable_thread_safety(game_crafting &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_crafting_disable_thread_safety(game_crafting &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_crafting_is_thread_safe(game_crafting &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_crafting_get_recipes_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_get_recipes));
    return (1);
}

FT_TEST(test_game_crafting_get_recipes_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_get_recipes_const));
    return (1);
}

FT_TEST(test_game_crafting_register_recipe_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_register_recipe));
    return (1);
}

FT_TEST(test_game_crafting_craft_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_craft_item));
    return (1);
}

FT_TEST(test_game_crafting_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_get_error));
    return (1);
}

FT_TEST(test_game_crafting_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_get_error_str));
    return (1);
}

FT_TEST(test_game_crafting_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_crafting_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_crafting_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_crafting>(
                        game_crafting_is_thread_safe));
    return (1);
}
