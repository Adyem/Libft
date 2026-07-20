#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_price_definition_additional_get_base_value(game_price_definition &value)
{
    (void)value.get_base_value();
    return ;
}

static void
game_price_definition_additional_set_base_value(game_price_definition &value)
{
    value.set_base_value(10);
    return ;
}

static void
game_price_definition_additional_get_minimum_value(game_price_definition &value)
{
    (void)value.get_minimum_value();
    return ;
}

static void
game_price_definition_additional_set_minimum_value(game_price_definition &value)
{
    value.set_minimum_value(10);
    return ;
}

static void
game_price_definition_additional_get_maximum_value(game_price_definition &value)
{
    (void)value.get_maximum_value();
    return ;
}

static void
game_price_definition_additional_set_maximum_value(game_price_definition &value)
{
    value.set_maximum_value(10);
    return ;
}

static void game_price_definition_additional_disable_thread_safety(
    game_price_definition &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void
game_price_definition_additional_unlock(game_price_definition &value)
{
    value.unlock(FT_FALSE);
    return ;
}

static void
game_price_definition_additional_get_error(game_price_definition &value)
{
    (void)value.get_error();
    return ;
}

static void
game_price_definition_additional_get_error_str(game_price_definition &value)
{
    (void)value.get_error_str();
    return ;
}

static void
game_price_definition_additional_initialize_twice(game_price_definition &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void game_price_definition_additional_copy_initialize_uninitialised(
    game_price_definition &value)
{
    game_price_definition source;

    (void)value.initialize(source);
    return ;
}

static void game_price_definition_additional_move_initialize_uninitialised(
    game_price_definition &value)
{
    game_price_definition source;

    (void)value.initialize(static_cast<game_price_definition &&>(source));
    return ;
}

static void game_price_definition_additional_move_uninitialised(
    game_price_definition &value)
{
    game_price_definition source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_price_definition_get_base_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_get_base_value));
    return (1);
}

FT_TEST(test_game_price_definition_set_base_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_set_base_value));
    return (1);
}

FT_TEST(test_game_price_definition_get_minimum_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_get_minimum_value));
    return (1);
}

FT_TEST(test_game_price_definition_set_minimum_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_set_minimum_value));
    return (1);
}

FT_TEST(test_game_price_definition_get_maximum_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_get_maximum_value));
    return (1);
}

FT_TEST(test_game_price_definition_set_maximum_value_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_set_maximum_value));
    return (1);
}

FT_TEST(test_game_price_definition_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1,
                 expect_game_lifecycle_sigabrt<game_price_definition>(
                     game_price_definition_additional_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_price_definition_unlock_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_unlock));
    return (1);
}

FT_TEST(test_game_price_definition_is_thread_safe_uninitialised_is_safe)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_price_definition_destroy_uninitialised_is_safe)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_price_definition_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_get_error));
    return (1);
}

FT_TEST(test_game_price_definition_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_get_error_str));
    return (1);
}

FT_TEST(test_game_price_definition_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_initialize_twice));
    return (1);
}

FT_TEST(test_game_price_definition_copy_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_price_definition>(
               game_price_definition_additional_copy_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_price_definition_move_initialize_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_price_definition>(
               game_price_definition_additional_move_initialize_uninitialised));
    return (1);
}

FT_TEST(test_game_price_definition_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_price_definition>(
                        game_price_definition_additional_move_uninitialised));
    return (1);
}

FT_TEST(test_game_price_definition_initialize_with_values_succeeds)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(4, 2, 500, 300, 800));
    FT_ASSERT_EQ(4, value.get_item_id());
    FT_ASSERT_EQ(2, value.get_rarity());
    FT_ASSERT_EQ(500, value.get_base_value());
    FT_ASSERT_EQ(300, value.get_minimum_value());
    FT_ASSERT_EQ(800, value.get_maximum_value());
    return (1);
}

FT_TEST(test_game_price_definition_initialize_default_values_succeeds)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(0, value.get_item_id());
    FT_ASSERT_EQ(0, value.get_rarity());
    FT_ASSERT_EQ(0, value.get_base_value());
    FT_ASSERT_EQ(0, value.get_minimum_value());
    FT_ASSERT_EQ(0, value.get_maximum_value());
    return (1);
}

FT_TEST(test_game_price_definition_get_rarity_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 3, 700, 400, 900));
    FT_ASSERT_EQ(3, value.get_rarity());
    return (1);
}

FT_TEST(test_game_price_definition_get_base_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 3, 700, 400, 900));
    FT_ASSERT_EQ(700, value.get_base_value());
    return (1);
}

FT_TEST(test_game_price_definition_get_minimum_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 3, 700, 400, 900));
    FT_ASSERT_EQ(400, value.get_minimum_value());
    return (1);
}

FT_TEST(test_game_price_definition_get_maximum_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(8, 3, 700, 400, 900));
    FT_ASSERT_EQ(900, value.get_maximum_value());
    return (1);
}

FT_TEST(test_game_price_definition_set_item_id_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_item_id(12);
    FT_ASSERT_EQ(12, value.get_item_id());
    return (1);
}

FT_TEST(test_game_price_definition_set_rarity_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_rarity(4);
    FT_ASSERT_EQ(4, value.get_rarity());
    return (1);
}

FT_TEST(test_game_price_definition_set_base_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_base_value(900);
    FT_ASSERT_EQ(900, value.get_base_value());
    return (1);
}

FT_TEST(test_game_price_definition_set_minimum_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_minimum_value(350);
    FT_ASSERT_EQ(350, value.get_minimum_value());
    return (1);
}

FT_TEST(test_game_price_definition_set_maximum_value_initialised)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_maximum_value(1200);
    FT_ASSERT_EQ(1200, value.get_maximum_value());
    return (1);
}

FT_TEST(test_game_price_definition_thread_safety_enable_disable_cycle)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_price_definition_lock_unlock_initialised)
{
    game_price_definition value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_price_definition_destroy_twice_is_safe)
{
    game_price_definition value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}
