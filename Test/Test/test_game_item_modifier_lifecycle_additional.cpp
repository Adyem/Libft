#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void item_modifier_initialize_twice(game_item_modifier &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void
item_modifier_copy_initialize_uninitialised(game_item_modifier &value)
{
    game_item_modifier source;

    (void)value.initialize(source);
    return ;
}

static void
item_modifier_move_initialize_uninitialised(game_item_modifier &value)
{
    game_item_modifier source;

    (void)value.initialize(static_cast<game_item_modifier &&>(source));
    return ;
}

static void item_modifier_move_uninitialised(game_item_modifier &value)
{
    game_item_modifier source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_item_modifier_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        item_modifier_initialize_twice));
    return (1);
}

FT_TEST(test_game_item_modifier_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        item_modifier_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_item_modifier_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        item_modifier_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_item_modifier_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        item_modifier_move_uninitialised));
    return (1);
}

FT_TEST(test_game_item_modifier_initialize_default_values)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    FT_ASSERT_EQ(0, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_initialize_with_values)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 14));
    FT_ASSERT_EQ(8, value.get_id());
    FT_ASSERT_EQ(14, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_set_id_updates_value)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(5);
    FT_ASSERT_EQ(5, value.get_id());
    return (1);
}

FT_TEST(test_game_item_modifier_set_value_updates_value)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_value(17);
    FT_ASSERT_EQ(17, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_negative_id_reports_error)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(-1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_item_modifier_negative_value_reports_error)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_value(-1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_item_modifier_copy_after_mutation)
{
    game_item_modifier source;
    game_item_modifier destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(3, 21));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(3, destination.get_id());
    FT_ASSERT_EQ(21, destination.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_move_after_mutation)
{
    game_item_modifier source;
    game_item_modifier destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4, 22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(4, destination.get_id());
    FT_ASSERT_EQ(22, destination.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_self_move_is_safe)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(4, 22));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(4, value.get_id());
    return (1);
}

FT_TEST(test_game_item_modifier_thread_safety_enable_disable_cycle)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_error_string_after_success)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_item_modifier_destroy_resets_values)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(5, 25));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_id());
    FT_ASSERT_EQ(0, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_destroy_twice_is_safe)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_item_modifier_large_values_are_preserved)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1000, 2000));
    FT_ASSERT_EQ(1000, value.get_id());
    FT_ASSERT_EQ(2000, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_reinitialize_after_destroy)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(2, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(9, 8));
    FT_ASSERT_EQ(9, value.get_id());
    FT_ASSERT_EQ(8, value.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_error_after_valid_setters)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_id(12);
    value.set_value(13);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_item_modifier_disable_without_mutex_is_safe)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_enable_is_idempotent)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_item_modifier_copy_keeps_source_values)
{
    game_item_modifier source;
    game_item_modifier destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(6, 16));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(6, source.get_id());
    FT_ASSERT_EQ(16, source.get_value());
    return (1);
}

FT_TEST(test_game_item_modifier_move_self_preserves_value)
{
    game_item_modifier value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(7, 18));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(18, value.get_value());
    return (1);
}
