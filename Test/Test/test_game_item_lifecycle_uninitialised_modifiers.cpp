#include "../../Modules/Game/game_item.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_item_get_modifier1_id(game_item &value)
{
    (void)value.get_modifier1_id();
    return ;
}

static void game_item_set_modifier1_id(game_item &value)
{
    value.set_modifier1_id(1);
    return ;
}

static void game_item_get_modifier1_value(game_item &value)
{
    (void)value.get_modifier1_value();
    return ;
}

static void game_item_set_modifier1_value(game_item &value)
{
    value.set_modifier1_value(1);
    return ;
}

static void game_item_get_modifier2_id(game_item &value)
{
    (void)value.get_modifier2_id();
    return ;
}

static void game_item_set_modifier2_id(game_item &value)
{
    value.set_modifier2_id(1);
    return ;
}

static void game_item_get_modifier2_value(game_item &value)
{
    (void)value.get_modifier2_value();
    return ;
}

static void game_item_set_modifier2_value(game_item &value)
{
    value.set_modifier2_value(1);
    return ;
}

static void game_item_get_modifier3_id(game_item &value)
{
    (void)value.get_modifier3_id();
    return ;
}

static void game_item_set_modifier3_id(game_item &value)
{
    value.set_modifier3_id(1);
    return ;
}

static void game_item_get_modifier3_value(game_item &value)
{
    (void)value.get_modifier3_value();
    return ;
}

static void game_item_set_modifier3_value(game_item &value)
{
    value.set_modifier3_value(1);
    return ;
}

static void game_item_get_modifier4_id(game_item &value)
{
    (void)value.get_modifier4_id();
    return ;
}

static void game_item_set_modifier4_id(game_item &value)
{
    value.set_modifier4_id(1);
    return ;
}

static void game_item_get_modifier4_value(game_item &value)
{
    (void)value.get_modifier4_value();
    return ;
}

static void game_item_set_modifier4_value(game_item &value)
{
    value.set_modifier4_value(1);
    return ;
}

FT_TEST(test_game_item_get_modifier1_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier1_id));
    return (1);
}

FT_TEST(test_game_item_set_modifier1_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier1_id));
    return (1);
}

FT_TEST(test_game_item_get_modifier1_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier1_value));
    return (1);
}

FT_TEST(test_game_item_set_modifier1_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier1_value));
    return (1);
}

FT_TEST(test_game_item_get_modifier2_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier2_id));
    return (1);
}

FT_TEST(test_game_item_set_modifier2_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier2_id));
    return (1);
}

FT_TEST(test_game_item_get_modifier2_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier2_value));
    return (1);
}

FT_TEST(test_game_item_set_modifier2_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier2_value));
    return (1);
}

FT_TEST(test_game_item_get_modifier3_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier3_id));
    return (1);
}

FT_TEST(test_game_item_set_modifier3_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier3_id));
    return (1);
}

FT_TEST(test_game_item_get_modifier3_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier3_value));
    return (1);
}

FT_TEST(test_game_item_set_modifier3_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier3_value));
    return (1);
}

FT_TEST(test_game_item_get_modifier4_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier4_id));
    return (1);
}

FT_TEST(test_game_item_set_modifier4_id_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier4_id));
    return (1);
}

FT_TEST(test_game_item_get_modifier4_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_get_modifier4_value));
    return (1);
}

FT_TEST(test_game_item_set_modifier4_value_uninitialised_is_safe)
{
    FT_ASSERT_EQ(0, expect_game_lifecycle_sigabrt<game_item>(
                        game_item_set_modifier4_value));
    return (1);
}
