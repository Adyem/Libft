#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_item_modifier_get_id(game_item_modifier &value)
{
    (void)value.get_id();
    return ;
}

static void game_item_modifier_set_id(game_item_modifier &value)
{
    value.set_id(1);
    return ;
}

static void game_item_modifier_get_value(game_item_modifier &value)
{
    (void)value.get_value();
    return ;
}

static void game_item_modifier_set_value(game_item_modifier &value)
{
    value.set_value(1);
    return ;
}

static void game_item_modifier_enable_thread_safety(game_item_modifier &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_item_modifier_disable_thread_safety(game_item_modifier &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_item_modifier_is_thread_safe(game_item_modifier &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_item_modifier_get_error(game_item_modifier &value)
{
    (void)value.get_error();
    return ;
}

static void game_item_modifier_get_error_str(game_item_modifier &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_item_modifier_get_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_get_id));
    return (1);
}

FT_TEST(test_game_item_modifier_set_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_set_id));
    return (1);
}

FT_TEST(test_game_item_modifier_get_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_get_value));
    return (1);
}

FT_TEST(test_game_item_modifier_set_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_set_value));
    return (1);
}

FT_TEST(test_game_item_modifier_enable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_modifier_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_modifier_is_thread_safe_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_is_thread_safe));
    return (1);
}

FT_TEST(test_game_item_modifier_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_get_error));
    return (1);
}

FT_TEST(test_game_item_modifier_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_modifier>(
                        game_item_modifier_get_error_str));
    return (1);
}
