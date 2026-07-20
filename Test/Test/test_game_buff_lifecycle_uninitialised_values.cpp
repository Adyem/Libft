#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void game_buff_get_id(game_buff &value)
{
    (void)value.get_id();
    return ;
}

static void game_buff_set_id(game_buff &value)
{
    value.set_id(1);
    return ;
}

static void game_buff_get_duration(game_buff &value)
{
    (void)value.get_duration();
    return ;
}

static void game_buff_set_duration(game_buff &value)
{
    value.set_duration(1);
    return ;
}

static void game_buff_add_duration(game_buff &value)
{
    value.add_duration(1);
    return ;
}

static void game_buff_sub_duration(game_buff &value)
{
    value.sub_duration(1);
    return ;
}

static void game_buff_get_modifier1(game_buff &value)
{
    (void)value.get_modifier1();
    return ;
}

static void game_buff_set_modifier1(game_buff &value)
{
    value.set_modifier1(1);
    return ;
}

static void game_buff_add_modifier1(game_buff &value)
{
    value.add_modifier1(1);
    return ;
}

static void game_buff_sub_modifier1(game_buff &value)
{
    value.sub_modifier1(1);
    return ;
}

static void game_buff_get_modifier2(game_buff &value)
{
    (void)value.get_modifier2();
    return ;
}

static void game_buff_set_modifier2(game_buff &value)
{
    value.set_modifier2(1);
    return ;
}

static void game_buff_add_modifier2(game_buff &value)
{
    value.add_modifier2(1);
    return ;
}

static void game_buff_sub_modifier2(game_buff &value)
{
    value.sub_modifier2(1);
    return ;
}

static void game_buff_get_modifier3(game_buff &value)
{
    (void)value.get_modifier3();
    return ;
}

static void game_buff_set_modifier3(game_buff &value)
{
    value.set_modifier3(1);
    return ;
}

static void game_buff_add_modifier3(game_buff &value)
{
    value.add_modifier3(1);
    return ;
}

static void game_buff_sub_modifier3(game_buff &value)
{
    value.sub_modifier3(1);
    return ;
}

static void game_buff_get_modifier4(game_buff &value)
{
    (void)value.get_modifier4();
    return ;
}

static void game_buff_set_modifier4(game_buff &value)
{
    value.set_modifier4(1);
    return ;
}

static void game_buff_add_modifier4(game_buff &value)
{
    value.add_modifier4(1);
    return ;
}

static void game_buff_sub_modifier4(game_buff &value)
{
    value.sub_modifier4(1);
    return ;
}

FT_TEST(test_game_buff_get_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_id));
    return (1);
}

FT_TEST(test_game_buff_set_id_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_id));
    return (1);
}

FT_TEST(test_game_buff_get_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_duration));
    return (1);
}

FT_TEST(test_game_buff_set_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_duration));
    return (1);
}

FT_TEST(test_game_buff_add_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_duration));
    return (1);
}

FT_TEST(test_game_buff_sub_duration_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_duration));
    return (1);
}

FT_TEST(test_game_buff_get_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier1));
    return (1);
}

FT_TEST(test_game_buff_set_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier1));
    return (1);
}

FT_TEST(test_game_buff_add_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier1));
    return (1);
}

FT_TEST(test_game_buff_sub_modifier1_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier1));
    return (1);
}

FT_TEST(test_game_buff_get_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier2));
    return (1);
}

FT_TEST(test_game_buff_set_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier2));
    return (1);
}

FT_TEST(test_game_buff_add_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier2));
    return (1);
}

FT_TEST(test_game_buff_sub_modifier2_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier2));
    return (1);
}

FT_TEST(test_game_buff_get_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier3));
    return (1);
}

FT_TEST(test_game_buff_set_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier3));
    return (1);
}

FT_TEST(test_game_buff_add_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier3));
    return (1);
}

FT_TEST(test_game_buff_sub_modifier3_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier3));
    return (1);
}

FT_TEST(test_game_buff_get_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_get_modifier4));
    return (1);
}

FT_TEST(test_game_buff_set_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_set_modifier4));
    return (1);
}

FT_TEST(test_game_buff_add_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_add_modifier4));
    return (1);
}

FT_TEST(test_game_buff_sub_modifier4_uninitialised_aborts)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_buff>(game_buff_sub_modifier4));
    return (1);
}
