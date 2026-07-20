#include "../../Modules/Game/game_inventory.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_inventory_get_items_const(game_inventory &value)
{
    const game_inventory &constant_value = value;

    (void)constant_value.get_items();
    return ;
}

static void game_inventory_add_item(game_inventory &value)
{
    ft_sharedptr<game_item> item;

    (void)value.add_item(item);
    return ;
}

static void game_inventory_remove_item(game_inventory &value)
{
    value.remove_item(0);
    return ;
}

static void game_inventory_count_item(game_inventory &value)
{
    (void)value.count_item(1);
    return ;
}

static void game_inventory_has_item(game_inventory &value)
{
    (void)value.has_item(1);
    return ;
}

static void game_inventory_count_rarity(game_inventory &value)
{
    (void)value.count_rarity(1);
    return ;
}

static void game_inventory_has_rarity(game_inventory &value)
{
    (void)value.has_rarity(1);
    return ;
}

static void game_inventory_get_error(game_inventory &value)
{
    (void)value.get_error();
    return ;
}

static void game_inventory_get_error_str(game_inventory &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_inventory_get_items_const_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_items_const));
    return (1);
}

FT_TEST(test_game_inventory_add_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_add_item));
    return (1);
}

FT_TEST(test_game_inventory_remove_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_remove_item));
    return (1);
}

FT_TEST(test_game_inventory_count_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_count_item));
    return (1);
}

FT_TEST(test_game_inventory_has_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_has_item));
    return (1);
}

FT_TEST(test_game_inventory_count_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_count_rarity));
    return (1);
}

FT_TEST(test_game_inventory_has_rarity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_has_rarity));
    return (1);
}

FT_TEST(test_game_inventory_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_error));
    return (1);
}

FT_TEST(test_game_inventory_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_error_str));
    return (1);
}
