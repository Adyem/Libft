#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_item_get_max_stack(game_item &value)
{
    (void)value.get_max_stack();
    return ;
}

static void game_item_set_max_stack(game_item &value)
{
    value.set_max_stack(1);
    return ;
}

static void game_item_get_stack_size(game_item &value)
{
    (void)value.get_stack_size();
    return ;
}

static void game_item_set_stack_size(game_item &value)
{
    value.set_stack_size(1);
    return ;
}

static void game_item_add_to_stack(game_item &value)
{
    value.add_to_stack(1);
    return ;
}

static void game_item_sub_from_stack(game_item &value)
{
    value.sub_from_stack(1);
    return ;
}

static void game_item_get_item_id(game_item &value)
{
    (void)value.get_item_id();
    return ;
}

static void game_item_set_item_id(game_item &value)
{
    value.set_item_id(1);
    return ;
}

static void game_item_get_width(game_item &value)
{
    (void)value.get_width();
    return ;
}

static void game_item_set_width(game_item &value)
{
    value.set_width(1);
    return ;
}

static void game_item_get_height(game_item &value)
{
    (void)value.get_height();
    return ;
}

static void game_item_set_height(game_item &value)
{
    value.set_height(1);
    return ;
}

static void game_item_get_rarity(game_item &value)
{
    (void)value.get_rarity();
    return ;
}

static void game_item_set_rarity(game_item &value)
{
    value.set_rarity(1);
    return ;
}

FT_TEST(test_game_item_get_max_stack_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_get_max_stack));
    return (1);
}

FT_TEST(test_game_item_set_max_stack_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_set_max_stack));
    return (1);
}

FT_TEST(test_game_item_get_stack_size_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_get_stack_size));
    return (1);
}

FT_TEST(test_game_item_set_stack_size_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_set_stack_size));
    return (1);
}

FT_TEST(test_game_item_add_to_stack_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_add_to_stack));
    return (1);
}

FT_TEST(test_game_item_sub_from_stack_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_sub_from_stack));
    return (1);
}

FT_TEST(test_game_item_get_item_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_get_item_id));
    return (1);
}

FT_TEST(test_game_item_set_item_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_set_item_id));
    return (1);
}

FT_TEST(test_game_item_get_width_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0,
                 expect_game_lifecycle_sigabrt<game_item>(game_item_get_width));
    return (1);
}

FT_TEST(test_game_item_set_width_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0,
                 expect_game_lifecycle_sigabrt<game_item>(game_item_set_width));
    return (1);
}

FT_TEST(test_game_item_get_height_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_get_height));
    return (1);
}

FT_TEST(test_game_item_set_height_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_set_height));
    return (1);
}

FT_TEST(test_game_item_get_rarity_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_get_rarity));
    return (1);
}

FT_TEST(test_game_item_set_rarity_uninitialised_is_safe)
{
    FT_ASSERT_EQ(
        0, expect_game_lifecycle_sigabrt<game_item>(game_item_set_rarity));
    return (1);
}
