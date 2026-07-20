#include "../../Modules/Game/game_inventory.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void inventory_initialize_twice(game_inventory &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void inventory_copy_initialize_uninitialised(game_inventory &value)
{
    game_inventory source;

    (void)value.initialize(source);
    return ;
}

static void inventory_move_initialize_uninitialised(game_inventory &value)
{
    game_inventory source;

    (void)value.initialize(static_cast<game_inventory &&>(source));
    return ;
}

static void inventory_move_uninitialised(game_inventory &value)
{
    game_inventory source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_inventory_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        inventory_initialize_twice));
    return (1);
}

FT_TEST(test_game_inventory_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        inventory_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_inventory_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        inventory_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_inventory_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_inventory>(
                        inventory_move_uninitialised));
    return (1);
}

FT_TEST(test_game_inventory_initialize_default_values)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_capacity());
    FT_ASSERT_EQ(0, value.get_used());
    FT_ASSERT_EQ(0, value.get_weight_limit());
    FT_ASSERT_EQ(0, value.get_current_weight());
    FT_ASSERT_EQ(FT_FALSE, value.is_full());
    return (1);
}

FT_TEST(test_game_inventory_initialize_with_values)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 100));
    FT_ASSERT_EQ(8, value.get_capacity());
    FT_ASSERT_EQ(100, value.get_weight_limit());
    FT_ASSERT_EQ(0, value.get_used());
    return (1);
}

FT_TEST(test_game_inventory_resize_updates_capacity)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.resize(12);
    FT_ASSERT_EQ(12, value.get_capacity());
    return (1);
}

FT_TEST(test_game_inventory_set_used_slots_updates_full_state)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(4, 0));
    value.set_used_slots(4);
    FT_ASSERT_EQ(4, value.get_used());
    FT_ASSERT_EQ(FT_TRUE, value.is_full());
    return (1);
}

FT_TEST(test_game_inventory_set_weight_limit_updates_full_state)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_weight_limit(20);
    value.set_current_weight(20);
    FT_ASSERT_EQ(20, value.get_weight_limit());
    FT_ASSERT_EQ(20, value.get_current_weight());
    FT_ASSERT_EQ(FT_TRUE, value.is_full());
    return (1);
}

FT_TEST(test_game_inventory_set_current_weight_below_limit_is_not_full)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_weight_limit(20);
    value.set_current_weight(19);
    FT_ASSERT_EQ(FT_FALSE, value.is_full());
    return (1);
}

FT_TEST(test_game_inventory_empty_count_item_is_zero)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.count_item(4));
    FT_ASSERT_EQ(FT_FALSE, value.has_item(4));
    return (1);
}

FT_TEST(test_game_inventory_empty_count_rarity_is_zero)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.count_rarity(2));
    FT_ASSERT_EQ(FT_FALSE, value.has_rarity(2));
    return (1);
}

FT_TEST(test_game_inventory_add_null_item_reports_error)
{
    game_inventory value;
    ft_sharedptr<game_item> item;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.add_item(item));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, value.get_error());
    return (1);
}

FT_TEST(test_game_inventory_remove_missing_item_is_safe)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.remove_item(42);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_inventory_get_items_mutable_initialised)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_items().size());
    return (1);
}

FT_TEST(test_game_inventory_get_items_const_initialised)
{
    game_inventory value;
    const game_inventory &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, const_value.get_items().size());
    return (1);
}

FT_TEST(test_game_inventory_thread_safety_enable_disable_cycle)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_inventory_lock_unlock_initialised)
{
    game_inventory value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_inventory_destroy_twice_is_safe)
{
    game_inventory value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
