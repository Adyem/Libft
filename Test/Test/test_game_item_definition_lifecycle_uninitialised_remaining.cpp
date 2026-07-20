#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_item_definition_get_slot_requirement(game_item_definition &value)
{
    (void)value.get_slot_requirement();
    return ;
}

static void
game_item_definition_set_slot_requirement(game_item_definition &value)
{
    value.set_slot_requirement(1);
    return ;
}

static void
game_item_definition_enable_thread_safety(game_item_definition &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void
game_item_definition_disable_thread_safety(game_item_definition &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_item_definition_is_thread_safe(game_item_definition &value)
{
    (void)value.is_thread_safe();
    return ;
}

static void game_item_definition_get_error(game_item_definition &value)
{
    (void)value.get_error();
    return ;
}

static void game_item_definition_get_error_str(game_item_definition &value)
{
    (void)value.get_error_str();
    return ;
}

FT_TEST(test_game_item_definition_get_slot_requirement_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_slot_requirement));
    return (1);
}

FT_TEST(test_game_item_definition_set_slot_requirement_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_slot_requirement));
    return (1);
}

FT_TEST(test_game_item_definition_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_definition_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_item_definition_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_is_thread_safe));
    return (1);
}

FT_TEST(test_game_item_definition_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_error));
    return (1);
}

FT_TEST(test_game_item_definition_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_error_str));
    return (1);
}
