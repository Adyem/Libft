#include "../../Modules/Game/game_inventory.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_inventory_get_items(game_inventory &value)
{
    (void)value.get_items();
    return ;
}

static void game_inventory_get_capacity(game_inventory &value)
{
    (void)value.get_capacity();
    return ;
}

static void game_inventory_resize(game_inventory &value)
{
    value.resize(1U);
    return ;
}

static void game_inventory_get_used(game_inventory &value)
{
    (void)value.get_used();
    return ;
}

static void game_inventory_set_used_slots(game_inventory &value)
{
    value.set_used_slots(1U);
    return ;
}

static void game_inventory_is_full(game_inventory &value)
{
    (void)value.is_full();
    return ;
}

static void game_inventory_get_weight_limit(game_inventory &value)
{
    (void)value.get_weight_limit();
    return ;
}

static void game_inventory_set_weight_limit(game_inventory &value)
{
    value.set_weight_limit(1);
    return ;
}

static void game_inventory_get_current_weight(game_inventory &value)
{
    (void)value.get_current_weight();
    return ;
}

static void game_inventory_set_current_weight(game_inventory &value)
{
    value.set_current_weight(1);
    return ;
}

FT_TEST(test_game_inventory_get_items_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_items));
    return (1);
}

FT_TEST(test_game_inventory_get_capacity_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_capacity));
    return (1);
}

FT_TEST(test_game_inventory_resize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_resize));
    return (1);
}

FT_TEST(test_game_inventory_get_used_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_used));
    return (1);
}

FT_TEST(test_game_inventory_set_used_slots_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_set_used_slots));
    return (1);
}

FT_TEST(test_game_inventory_is_full_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_is_full));
    return (1);
}

FT_TEST(test_game_inventory_get_weight_limit_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_weight_limit));
    return (1);
}

FT_TEST(test_game_inventory_set_weight_limit_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_set_weight_limit));
    return (1);
}

FT_TEST(test_game_inventory_get_current_weight_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_get_current_weight));
    return (1);
}

FT_TEST(test_game_inventory_set_current_weight_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        game_inventory_set_current_weight));
    return (1);
}
