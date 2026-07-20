#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_upgrade_get_id(game_upgrade &value)
{
    (void)value.get_id();
    return ;
}
static void game_upgrade_set_id(game_upgrade &value)
{
    value.set_id(1);
    return ;
}
static void game_upgrade_set_current_level(game_upgrade &value)
{
    value.set_current_level(1);
    return ;
}
static void game_upgrade_add_level(game_upgrade &value)
{
    value.add_level(1);
    return ;
}
static void game_upgrade_sub_level(game_upgrade &value)
{
    value.sub_level(1);
    return ;
}
static void game_upgrade_set_max_level(game_upgrade &value)
{
    value.set_max_level(1);
    return ;
}
static void game_upgrade_get_modifier1(game_upgrade &value)
{
    (void)value.get_modifier1();
    return ;
}
static void game_upgrade_set_modifier1(game_upgrade &value)
{
    value.set_modifier1(1);
    return ;
}
static void game_upgrade_get_modifier2(game_upgrade &value)
{
    (void)value.get_modifier2();
    return ;
}
static void game_upgrade_set_modifier2(game_upgrade &value)
{
    value.set_modifier2(1);
    return ;
}

FT_TEST(test_new_game_upgrade_get_id_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_upgrade>(game_upgrade_get_id));
    return (1);
}
FT_TEST(test_new_game_upgrade_set_id_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_upgrade>(game_upgrade_set_id));
    return (1);
}
FT_TEST(test_new_game_upgrade_set_current_level_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_set_current_level));
    return (1);
}
FT_TEST(test_new_game_upgrade_add_level_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_upgrade>(game_upgrade_add_level));
    return (1);
}
FT_TEST(test_new_game_upgrade_sub_level_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_upgrade>(game_upgrade_sub_level));
    return (1);
}
FT_TEST(test_new_game_upgrade_set_max_level_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_set_max_level));
    return (1);
}
FT_TEST(test_new_game_upgrade_get_modifier1_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_get_modifier1));
    return (1);
}
FT_TEST(test_new_game_upgrade_set_modifier1_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_set_modifier1));
    return (1);
}
FT_TEST(test_new_game_upgrade_get_modifier2_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_get_modifier2));
    return (1);
}
FT_TEST(test_new_game_upgrade_set_modifier2_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_upgrade>(
                        game_upgrade_set_modifier2));
    return (1);
}
