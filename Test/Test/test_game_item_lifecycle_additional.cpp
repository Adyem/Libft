#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void item_initialize_twice(game_item &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void item_copy_initialize_uninitialised(game_item &value)
{
    game_item source;

    (void)value.initialize(source);
    return ;
}

static void item_move_initialize_uninitialised(game_item &value)
{
    game_item source;

    (void)value.initialize(static_cast<game_item &&>(source));
    return ;
}

static void item_move_uninitialised(game_item &value)
{
    game_item source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_item_initialize_twice_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(item_initialize_twice));
    return (1);
}

FT_TEST(test_game_item_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item>(
                        item_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_item_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item>(
                        item_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_item_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_item>(item_move_uninitialised));
    return (1);
}

FT_TEST(test_game_item_initialize_default_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_max_stack());
    FT_ASSERT_EQ(0, value.get_stack_size());
    FT_ASSERT_EQ(0, value.get_item_id());
    FT_ASSERT_EQ(1, value.get_width());
    FT_ASSERT_EQ(1, value.get_height());
    FT_ASSERT_EQ(0, value.get_rarity());
    return (1);
}

FT_TEST(test_game_item_stack_add_and_subtract)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_max_stack(10);
    value.set_stack_size(5);
    value.add_to_stack(3);
    value.sub_from_stack(2);
    FT_ASSERT_EQ(6, value.get_stack_size());
    return (1);
}

FT_TEST(test_game_item_set_dimensions)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_width(3);
    value.set_height(4);
    FT_ASSERT_EQ(3, value.get_width());
    FT_ASSERT_EQ(4, value.get_height());
    return (1);
}

FT_TEST(test_game_item_set_identity_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_item_id(12);
    value.set_rarity(5);
    value.set_max_stack(64);
    FT_ASSERT_EQ(12, value.get_item_id());
    FT_ASSERT_EQ(5, value.get_rarity());
    FT_ASSERT_EQ(64, value.get_max_stack());
    return (1);
}

FT_TEST(test_game_item_modifier1_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier1_id(3);
    value.set_modifier1_value(9);
    FT_ASSERT_EQ(3, value.get_modifier1_id());
    FT_ASSERT_EQ(9, value.get_modifier1_value());
    return (1);
}

FT_TEST(test_game_item_modifier2_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier2_id(4);
    value.set_modifier2_value(10);
    FT_ASSERT_EQ(4, value.get_modifier2_id());
    FT_ASSERT_EQ(10, value.get_modifier2_value());
    return (1);
}

FT_TEST(test_game_item_modifier3_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier3_id(5);
    value.set_modifier3_value(11);
    FT_ASSERT_EQ(5, value.get_modifier3_id());
    FT_ASSERT_EQ(11, value.get_modifier3_value());
    return (1);
}

FT_TEST(test_game_item_modifier4_values)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier4_id(6);
    value.set_modifier4_value(12);
    FT_ASSERT_EQ(6, value.get_modifier4_id());
    FT_ASSERT_EQ(12, value.get_modifier4_value());
    return (1);
}

FT_TEST(test_game_item_get_modifier_copy)
{
    game_item value;
    game_item_modifier modifier;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, modifier.initialize(7, 13));
    value.set_modifier1(modifier);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_modifier1(modifier));
    FT_ASSERT_EQ(7, modifier.get_id());
    FT_ASSERT_EQ(13, modifier.get_value());
    return (1);
}

FT_TEST(test_game_item_thread_safety_enable_disable_cycle)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_error_after_success)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_item_id(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_item_destroy_twice_is_safe)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_item_set_stack_size_zero_is_preserved)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_stack_size(0);
    FT_ASSERT_EQ(0, value.get_stack_size());
    return (1);
}

FT_TEST(test_game_item_sub_stack_to_zero)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_stack_size(6);
    value.sub_from_stack(6);
    FT_ASSERT_EQ(0, value.get_stack_size());
    return (1);
}

FT_TEST(test_game_item_all_modifier_slots_start_empty)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_modifier1_id());
    FT_ASSERT_EQ(0, value.get_modifier2_id());
    FT_ASSERT_EQ(0, value.get_modifier3_id());
    FT_ASSERT_EQ(0, value.get_modifier4_id());
    return (1);
}

FT_TEST(test_game_item_destroy_reinitializes_subobjects)
{
    game_item value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_modifier1_id(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_modifier1_id());
    return (1);
}
