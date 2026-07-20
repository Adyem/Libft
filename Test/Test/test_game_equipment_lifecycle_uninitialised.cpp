#include "../../Modules/Game/game_equipment.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_equipment_equip(game_equipment &value)
{
    (void)value.equip(0, ft_sharedptr<game_item>());
    return ;
}

static void game_equipment_unequip(game_equipment &value)
{
    value.unequip(0);
    return ;
}

static void game_equipment_get_item(game_equipment &value)
{
    (void)value.get_item(0);
    return ;
}

static void game_equipment_get_error(game_equipment &value)
{
    (void)value.get_error();
    return ;
}

static void game_equipment_get_error_str(game_equipment &value)
{
    (void)value.get_error_str();
    return ;
}

static void game_equipment_enable_thread_safety(game_equipment &value)
{
    (void)value.enable_thread_safety();
    return ;
}

static void game_equipment_disable_thread_safety(game_equipment &value)
{
    (void)value.disable_thread_safety();
    return ;
}

static void game_equipment_is_thread_safe(game_equipment &value)
{
    (void)value.is_thread_safe();
    return ;
}

FT_TEST(test_game_equipment_equip_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_equip));
    return (1);
}

FT_TEST(test_game_equipment_unequip_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_unequip));
    return (1);
}

FT_TEST(test_game_equipment_get_item_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_get_item));
    return (1);
}

FT_TEST(test_game_equipment_get_error_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_get_error));
    return (1);
}

FT_TEST(test_game_equipment_get_error_str_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_get_error_str));
    return (1);
}

FT_TEST(test_game_equipment_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_enable_thread_safety));
    return (1);
}

FT_TEST(test_game_equipment_disable_thread_safety_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_disable_thread_safety));
    return (1);
}

FT_TEST(test_game_equipment_is_thread_safe_uninitialised_aborts)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_equipment>(
        game_equipment_is_thread_safe));
    return (1);
}
