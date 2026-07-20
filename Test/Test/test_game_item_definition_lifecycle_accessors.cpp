#include "../../Modules/Game/game_data_catalog.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_item_definition_get_item_id(game_item_definition &value)
{
    (void)value.get_item_id();
    return ;
}
static void game_item_definition_set_item_id(game_item_definition &value)
{
    value.set_item_id(1);
    return ;
}
static void game_item_definition_get_rarity(game_item_definition &value)
{
    (void)value.get_rarity();
    return ;
}
static void game_item_definition_set_rarity(game_item_definition &value)
{
    value.set_rarity(1);
    return ;
}
static void game_item_definition_get_max_stack(game_item_definition &value)
{
    (void)value.get_max_stack();
    return ;
}
static void game_item_definition_set_max_stack(game_item_definition &value)
{
    value.set_max_stack(1);
    return ;
}
static void game_item_definition_get_width(game_item_definition &value)
{
    (void)value.get_width();
    return ;
}
static void game_item_definition_set_width(game_item_definition &value)
{
    value.set_width(1);
    return ;
}
static void game_item_definition_get_height(game_item_definition &value)
{
    (void)value.get_height();
    return ;
}
static void game_item_definition_set_height(game_item_definition &value)
{
    value.set_height(1);
    return ;
}

FT_TEST(test_game_item_definition_get_item_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_item_id));
    return (1);
}
FT_TEST(test_game_item_definition_set_item_id_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_item_id));
    return (1);
}
FT_TEST(test_game_item_definition_get_rarity_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_rarity));
    return (1);
}
FT_TEST(test_game_item_definition_set_rarity_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_rarity));
    return (1);
}
FT_TEST(test_game_item_definition_get_max_stack_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_max_stack));
    return (1);
}
FT_TEST(test_game_item_definition_set_max_stack_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_max_stack));
    return (1);
}
FT_TEST(test_game_item_definition_get_width_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_width));
    return (1);
}
FT_TEST(test_game_item_definition_set_width_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_width));
    return (1);
}
FT_TEST(test_game_item_definition_get_height_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_get_height));
    return (1);
}
FT_TEST(test_game_item_definition_set_height_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_item_definition>(
                        game_item_definition_set_height));
    return (1);
}
